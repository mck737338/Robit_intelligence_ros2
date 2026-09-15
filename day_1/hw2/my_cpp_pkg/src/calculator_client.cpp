#include <chrono>
#include <memory>
#include <cstdlib>
#include "rclcpp/rclcpp.hpp"
#include "my_interfaces/srv/calculator.hpp"

using Calculator = my_interfaces::srv::Calculator;
using namespace std::chrono_literals;

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  // 커맨드라인 인자로 a, b, operation을 받음 (없으면 기본값 사용)
  double a = (argc > 1) ? std::atof(argv[1]) : 10.0;
  double b = (argc > 2) ? std::atof(argv[2]) : 3.0;
  std::string op = (argc > 3) ? argv[3] : "add";

  auto node = rclcpp::Node::make_shared("calculator_client");
  auto client = node->create_client<Calculator>("calculator");

  // 서비스가 준비될 때까지 대기
  while (!client->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(node->get_logger(), "Interrupted while waiting for service.");
      return 1;
    }
    RCLCPP_INFO(node->get_logger(), "Waiting for calculator service...");
  }

  auto request = std::make_shared<Calculator::Request>();
  request->a = a;
  request->b = b;
  request->operation = op;

  auto future = client->async_send_request(request);

  if (rclcpp::spin_until_future_complete(node, future) ==
      rclcpp::FutureReturnCode::SUCCESS)
  {
    auto response = future.get();
    if (response->success) {
      RCLCPP_INFO(node->get_logger(), "Result: %.2f", response->result);
    } else {
      RCLCPP_ERROR(node->get_logger(), "Failed: %s", response->message.c_str());
    }
  } else {
    RCLCPP_ERROR(node->get_logger(), "Failed to call service.");
  }

  rclcpp::shutdown();
  return 0;
}
