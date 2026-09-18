#include "hw1/fake_imu.hpp"

namespace hw1
{

FakeImu::FakeImu(const std::string & node_name)
: rclcpp_lifecycle::LifecycleNode(node_name),
  count_(0),
  rate_hz_(0.0),
  stamp_offset_sec_(0.0)
{
  // 생성자: 파라미터 선언만. 실제 값 조회와 검증은 on_configure에서 한다.
  this->declare_parameter<double>("rate_hz", 2.0);
  this->declare_parameter<double>("stamp_offset_sec", 0.0);
}

LifecycleCallbackReturn FakeImu::on_configure(const rclcpp_lifecycle::State & /*state*/)
{
  rate_hz_ = this->get_parameter("rate_hz").as_double();
  stamp_offset_sec_ = this->get_parameter("stamp_offset_sec").as_double();

  if (rate_hz_ <= 0.0) {
    RCLCPP_ERROR(this->get_logger(), "rate_hz must be > 0 (got %.2f)", rate_hz_);
    return LifecycleCallbackReturn::FAILURE;
  }

  pub_ = this->create_publisher<sensor_msgs::msg::Imu>("imu", 10);

  RCLCPP_INFO(
    this->get_logger(),
    "on_configure: rate_hz=%.2f, stamp_offset_sec=%.3f",
    rate_hz_, stamp_offset_sec_);

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn FakeImu::on_activate(const rclcpp_lifecycle::State & state)
{
  // 부모 구현이 LifecyclePublisher(pub_)를 자동으로 활성화한다.
  LifecycleNode::on_activate(state);

  // 타이머는 lifecycle이 자동 관리하지 않으므로 여기서 직접 생성한다.
  // 1.0 / rate_hz_ 는 반드시 double 나눗셈이어야 한다 (정수 나눗셈이면 0이 됨).
  timer_ = this->create_timer(
    std::chrono::duration<double>(1.0 / rate_hz_),
    std::bind(&FakeImu::publish_data, this));

  RCLCPP_INFO(this->get_logger(), "on_activate");

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn FakeImu::on_deactivate(const rclcpp_lifecycle::State & state)
{
  LifecycleNode::on_deactivate(state);

  timer_.reset();  // 타이머는 직접 파괴 — 재활성화 시 on_activate에서 다시 생성

  RCLCPP_INFO(this->get_logger(), "on_deactivate");

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn FakeImu::on_cleanup(const rclcpp_lifecycle::State & /*state*/)
{
  timer_.reset();
  pub_.reset();

  RCLCPP_INFO(this->get_logger(), "on_cleanup");

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn FakeImu::on_shutdown(const rclcpp_lifecycle::State & state)
{
  timer_.reset();
  pub_.reset();

  RCLCPP_INFO(this->get_logger(), "on_shutdown from [%s]", state.label().c_str());

  return LifecycleCallbackReturn::SUCCESS;
}

LifecycleCallbackReturn FakeImu::on_error(const rclcpp_lifecycle::State & state)
{
  RCLCPP_ERROR(this->get_logger(), "on_error from [%s]", state.label().c_str());

  timer_.reset();
  pub_.reset();

  // SUCCESS 반환 시 Unconfigured로 복구, FAILURE/ERROR면 Finalized로 종료
  return LifecycleCallbackReturn::SUCCESS;
}

void FakeImu::publish_data()
{
  auto msg = sensor_msgs::msg::Imu();

  // 항상 노드 clock을 통해 시간을 얻는다 (system_clock::now() 직접 사용 금지).
  const rclcpp::Time now = this->now();
  const rclcpp::Time stamped_time = now - rclcpp::Duration::from_seconds(stamp_offset_sec_);

  msg.header.stamp = stamped_time;
  msg.header.frame_id = "imu_link";

  msg.orientation.w = 1.0;
  msg.angular_velocity.z = 0.01 * static_cast<double>(count_);
  msg.linear_acceleration.x = 0.05 * static_cast<double>(count_ % 10);

  pub_->publish(msg);  // 비활성 상태라면 발행되지 않음 (LifecyclePublisher가 내부 차단)

  RCLCPP_INFO(
    this->get_logger(), "Publishing IMU data, count: %zu, stamp_offset=%.3f",
    count_, stamp_offset_sec_);

  count_++;
}

}  // namespace hw1

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<hw1::FakeImu>("fake_imu");

  // LifecycleNode는 rclcpp::Node를 상속하지 않으므로 base interface로 spin한다.
  rclcpp::spin(node->get_node_base_interface());

  rclcpp::shutdown();
  return 0;
}