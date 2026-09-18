#ifndef HW1__WATCHDOG_HPP_
#define HW1__WATCHDOG_HPP_

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "lifecycle_msgs/srv/get_state.hpp"

namespace hw1
{

using LifecycleCallbackReturn =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
using GetState = lifecycle_msgs::srv::GetState;

enum class SensorHealth
{
  NO_DATA,
  FRESH,
  STALE
};

class Watchdog : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit Watchdog(const std::string & node_name);

  LifecycleCallbackReturn on_configure(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_activate(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_cleanup(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_shutdown(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_error(const rclcpp_lifecycle::State & state) override;

private:
  void on_imu_data(const sensor_msgs::msg::Imu::SharedPtr msg);
  void check_health();
  void check_fake_imu_state();

  // ---- 기존: IMU 데이터 신선도 감시 ----
  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_;
  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>> pub_checked_;
  rclcpp::TimerBase::SharedPtr check_timer_;

  double timeout_sec_;
  double check_rate_hz_;

  bool has_received_msg_;
  rclcpp::Time last_stamp_;
  SensorHealth last_health_;

  // ---- 신규: fake_imu lifecycle 상태 확인 ----
  rclcpp::Client<GetState>::SharedPtr get_state_client_;
  rclcpp::TimerBase::SharedPtr state_timer_;
  rclcpp::CallbackGroup::SharedPtr client_cb_group_;  // 선택 과제(4)에서 사용
};

}  // namespace hw1

#endif  // HW1__WATCHDOG_HPP_