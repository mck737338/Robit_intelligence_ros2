#ifndef HW1__FAKE_IMU_HPP_
#define HW1__FAKE_IMU_HPP_

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

class FakeImu : public rclcpp_lifecycle::LifecycleNode
{
public:
  explicit FakeImu(const std::string & node_name);

  LifecycleCallbackReturn on_configure(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_activate(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_cleanup(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_shutdown(const rclcpp_lifecycle::State & state) override;
  LifecycleCallbackReturn on_error(const rclcpp_lifecycle::State & state) override;

private:
  void publish_data();

  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<sensor_msgs::msg::Imu>> pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  size_t count_;

  double rate_hz_;
  double stamp_offset_sec_;
};

}  // namespace hw1

#endif  // HW1__FAKE_IMU_HPP_