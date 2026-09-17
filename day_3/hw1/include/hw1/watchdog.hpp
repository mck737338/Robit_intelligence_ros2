#ifndef HW1__WATCHDOG_HPP_
#define HW1__WATCHDOG_HPP_

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_lifecycle/lifecycle_publisher.hpp"
#include "sensor_msgs/msg/imu.hpp"

namespace hw1
{

using LifecycleCallbackReturn =
  rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

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

  rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr sub_;
  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>> pub_checked_;
  rclcpp::TimerBase::SharedPtr check_timer_;

  double timeout_sec_;
  double check_rate_hz_;

  bool has_received_msg_;
  rclcpp::Time last_stamp_;
  SensorHealth last_health_;
};

}  // namespace hw1

#endif  // HW1__WATCHDOG_HPP_