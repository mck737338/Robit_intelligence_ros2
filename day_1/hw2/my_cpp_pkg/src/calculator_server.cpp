#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "my_interfaces/srv/calculator.hpp"

using Calculator = my_interfaces::srv::Calculator;

// 사칙연산 서비스 요청을 처리하는 콜백 함수
void calculate(
  const std::shared_ptr<Calculator::Request> request,
  std::shared_ptr<Calculator::Response> response)
{
  const std::string & op = request->operation;

  if (op == "add") {
    response->result = request->a + request->b;
    response->success = true;
    response->message = "OK";
  } else if (op == "sub") {
    response->result = request->a - request->b;
    response->success = true;
    response->message = "OK";
  } else if (op == "mul") {
    response->result = request->a * request->b;
    response->success = true;
    response->message = "OK";
  } else if (op == "div") {
    if (request->b == 0.0) {
      // 0으로 나누는 경우 에러 처리
      response->result = 0.0;
      response->success = false;
      response->message = "Error: division by zero";
    } else {
      response->result = request->a / request->b;
      response->success = true;
      response->message = "OK";
    }
  } else {
    // 지원하지 않는 연산자 처리
    response->result = 0.0;
    response->success = false;
    response->message = "Error: unknown operation '" + op + "'";
  }

  RCLCPP_INFO(rclcpp::get_logger("calculator_server"),
              "a=%.2f, b=%.2f, op=%s -> result=%.2f (success=%d)",
              request->a, request->b, op.c_str(), response->result, response->success);
}

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  auto node = rclcpp::Node::make_shared("calculator_server");
  rclcpp::Service<Calculator>::SharedPtr service =
    node->create_service<Calculator>("calculator", &calculate);

  RCLCPP_INFO(node->get_logger(), "Calculator service ready.");
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
