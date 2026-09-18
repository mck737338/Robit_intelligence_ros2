#include "hw1/watchdog.hpp"

namespace hw1
{

Watchdog::Watchdog(const std::string & node_name)
: rclcpp_lifecycle::LifecycleNode(node_name),
  timeout_sec_(0.0),
  check_rate_hz_(0.0),
  has_received_msg_(false),
  last_stamp_(0, 0, RCL_ROS_TIME),
  last_health_(SensorHealth::NO_DATA)
{
  // 생성자: 파라미터 선언만. 값 조회·검증은 on_configure에서.
  this->declare_parameter<double>("timeout_sec", 2.0);
  this->declare_parameter<double>("check_rate_hz", 2.0);
}

LifecycleCallbackReturn Watchdog::on_configure(const rclcpp_lifecycle::State & /*state*/)
{
  timeout_sec_ = this->get_parameter("timeout_sec").as_double();
  check_rate_hz_ = this->get_parameter("check_rate_hz").as_double();

  if (timeout_sec_ <= 0.0 || check_rate_hz_ <= 0.0) {
    RCLCPP_ERROR(
      this->get_logger(),
      "Invalid parameters: timeout_sec=%.2f, check_rate_hz=%.2f (both must be > 0)",
      timeout_sec_, check_rate_hz_);
    return LifecycleCallbackReturn::FAILURE;
  }

  pub_checked_ = this->create_publisher<sensor_msgs::msg::Imu>("imu_checked", 10);

  sub_ = this->create_subscription<sensor_msgs::msg::Imu>(
    "imu", 10,
    std::bind(&Watchdog::on_imu_data, this, std::placeholders::_1));

  // 신규: get_state 서비스 클라이언트 생성
  get_state_client_ = this->create_client<GetState>("/fake_imu/get_state");

  has_received_msg_ = false;
  last_health_ = SensorHealth::NO_DATA;

  RCLCPP_INFO(
    this->get_logger(),
    "on_configure: timeout_sec=%.2f, check_rate_hz=%.2f",
    timeout_sec_, check_rate_hz_);

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn Watchdog::on_activate(const rclcpp_lifecycle::State & state)
{
  LifecycleNode::on_activate(state);

  check_timer_ = this->create_timer(
    std::chrono::duration<double>(1.0 / check_rate_hz_),
    std::bind(&Watchdog::check_health, this));

  // 신규: 1초 주기 상태 확인 타이머
  state_timer_ = this->create_timer(
    std::chrono::duration<double>(1.0),
    std::bind(&Watchdog::check_fake_imu_state, this));

  RCLCPP_INFO(this->get_logger(), "on_activate");

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn Watchdog::on_deactivate(const rclcpp_lifecycle::State & state)
{
  LifecycleNode::on_deactivate(state);

  check_timer_.reset();
  state_timer_.reset();   // 신규

  RCLCPP_INFO(this->get_logger(), "on_deactivate");

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn Watchdog::on_cleanup(const rclcpp_lifecycle::State & /*state*/)
{
  check_timer_.reset();
  state_timer_.reset();
  sub_.reset();
  pub_checked_.reset();
  get_state_client_.reset();   // 신규: 서비스 클라이언트 해제

  has_received_msg_ = false;
  last_health_ = SensorHealth::NO_DATA;

  RCLCPP_INFO(this->get_logger(), "on_cleanup");

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn Watchdog::on_shutdown(const rclcpp_lifecycle::State & state)
{
  check_timer_.reset();
  sub_.reset();
  pub_checked_.reset();

  RCLCPP_INFO(this->get_logger(), "on_shutdown from [%s]", state.label().c_str());

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn Watchdog::on_error(const rclcpp_lifecycle::State & state)
{
  RCLCPP_ERROR(this->get_logger(), "on_error from [%s]", state.label().c_str());

  check_timer_.reset();
  sub_.reset();
  pub_checked_.reset();

  return LifecycleCallbackReturn::SUCCESS;
}

void Watchdog::on_imu_data(const sensor_msgs::msg::Imu::SharedPtr msg)
{
  has_received_msg_ = true;
  // 수신 시각이 아니라 메시지 안의 stamp를 기준으로 나이를 재야
  // fake_imu의 stamp_offset_sec 트릭(지연 데이터)을 감지할 수 있다.
  last_stamp_ = rclcpp::Time(msg->header.stamp, RCL_ROS_TIME);

  if (pub_checked_->is_activated()) {
    const auto age = this->now() - last_stamp_;
    if (age.seconds() <= timeout_sec_) {
      pub_checked_->publish(*msg);
    }
  }
}

void Watchdog::check_health()
{
  SensorHealth current_health;
  double age_sec = 0.0;

  if (!has_received_msg_) {
    current_health = SensorHealth::NO_DATA;
  } else {
    const rclcpp::Time now = this->now();
    age_sec = (now - last_stamp_).seconds();
    current_health = (age_sec <= timeout_sec_) ? SensorHealth::FRESH : SensorHealth::STALE;
  }

  switch (current_health) {
    case SensorHealth::NO_DATA:
      RCLCPP_WARN(this->get_logger(), "[NO DATA] No IMU message received yet.");
      break;

    case SensorHealth::STALE:
      RCLCPP_WARN(
        this->get_logger(), "[STALE] IMU data age = %.2f sec (timeout = %.2f sec)",
        age_sec, timeout_sec_);
      break;

    case SensorHealth::FRESH:
      if (last_health_ == SensorHealth::STALE) {
        RCLCPP_INFO(
          this->get_logger(), "[RECOVERED] IMU data resumed. age = %.2f sec", age_sec);
      }
      break;
  }

  last_health_ = current_health;
}

/*void Watchdog::check_fake_imu_state()   //일부러 추가한 함수, 
{
  if (!get_state_client_->service_is_ready()) {
    RCLCPP_WARN(this->get_logger(), "/fake_imu/get_state service not available");
    return;
  }

  auto request = std::make_shared<GetState::Request>();
  auto future = get_state_client_->async_send_request(request);

  // ---- 일부러 틀린 부분 ----
  // future.get()은 응답이 도착할 때까지 현재 스레드를 블로킹한다.
  // 그런데 이 콜백 자체가 유일한 executor 스레드 위에서 실행 중이므로,
  // 응답을 실제로 받아 처리해줄 스레드가 없다. → 영원히 반환하지 않음.
  auto response = future.get();

  RCLCPP_INFO(
    this->get_logger(), "fake_imu state: %s", response->current_state.label.c_str());
}*/

void Watchdog::check_fake_imu_state()   //수정 후
{
  if (!get_state_client_->service_is_ready()) {
    RCLCPP_WARN(this->get_logger(), "/fake_imu/get_state service not available");
    return;
  }

  auto request = std::make_shared<GetState::Request>();

  get_state_client_->async_send_request(
    request,
    [this](rclcpp::Client<GetState>::SharedFuture future) {
      auto response = future.get();
      RCLCPP_INFO(
        this->get_logger(), "fake_imu state: %s", response->current_state.label.c_str());
    });
}

}  // namespace hw1

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<hw1::Watchdog>("imu_watchdog");

  rclcpp::spin(node->get_node_base_interface());

  rclcpp::shutdown();
  return 0;
}