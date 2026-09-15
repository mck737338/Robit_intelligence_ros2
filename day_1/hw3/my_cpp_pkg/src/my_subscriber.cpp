#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "my_interfaces/msg/my_msg.hpp"

// MySub 클래스: "my_topic"을 구독하고 수신한 MyMsg를 로그로 출력
class MySub : public rclcpp::Node
{
public:
  MySub() : Node("my_sub")
  {
    // "my_topic"을 구독. 메시지 수신 시 람다 콜백 실행
    sub_ = create_subscription<my_interfaces::msg::MyMsg>(
      "my_topic", 10,
      [this](const my_interfaces::msg::MyMsg & msg) {
        RCLCPP_INFO(this->get_logger(), "I heard: id=%d, label='%s'",
                    msg.id, msg.label.c_str());
      });
  }

private:
  rclcpp::Subscription<my_interfaces::msg::MyMsg>::SharedPtr sub_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MySub>());
  rclcpp::shutdown();
  return 0;
}
