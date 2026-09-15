#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "my_interfaces/msg/my_msg.hpp"

using namespace std::chrono_literals;

// MyPub 클래스: MyMsg(id, label)를 주기적으로 발행하는 퍼블리셔 노드
class MyPub : public rclcpp::Node
{
public:
  MyPub() : Node("my_pub")
  {
    // "my_topic" 이름으로 MyMsg를 발행하는 퍼블리셔 생성 (QoS depth = 10)
    pub_ = create_publisher<my_interfaces::msg::MyMsg>("my_topic", 10);

    // 0.5초마다 callback() 함수를 실행하는 타이머 생성
    timer_ = create_wall_timer(500ms, [this] { callback(); });
  }

private:
  // 주기적으로 호출되어 MyMsg 메시지를 발행하는 콜백 함수
  void callback()
  {
    auto msg = my_interfaces::msg::MyMsg();
    msg.id = count_++;
    msg.label = "hello";
    pub_->publish(msg);
    RCLCPP_INFO(this->get_logger(), "Publishing: id=%d, label='%s'",
                msg.id, msg.label.c_str());
  }

  rclcpp::Publisher<my_interfaces::msg::MyMsg>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
  int32_t count_ = 0;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MyPub>());
  rclcpp::shutdown();
  return 0;
}
