// 과제 3 - 다중 토픽 모니터링 대시보드
// 하나의 퍼블리셔 노드가 서로 다른 자료형(string / int / float)의 데이터를
// 각각 별도의 토픽 3개에 발행한다.
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/float32.hpp>
#include <cmath>

using namespace std::chrono_literals;

class PublisherNode : public rclcpp::Node
{
public:
    PublisherNode() : Node("publisher_node"), count_(0)
    {
        string_pub_ = create_publisher<std_msgs::msg::String>("/dashboard/string_topic", 10);
        int_pub_ = create_publisher<std_msgs::msg::Int32>("/dashboard/int_topic", 10);
        float_pub_ = create_publisher<std_msgs::msg::Float32>("/dashboard/float_topic", 10);

        timer_ = create_wall_timer(500ms, std::bind(&PublisherNode::publishAll, this));

        RCLCPP_INFO(get_logger(), "publisher_node 시작: string/int/float 3개 토픽 발행 (0.5초 주기)");
    }

private:
    void publishAll()
    {
        // 토픽 1 - string
        std_msgs::msg::String string_msg;
        string_msg.data = "hello #" + std::to_string(count_);
        string_pub_->publish(string_msg);

        // 토픽 2 - int
        std_msgs::msg::Int32 int_msg;
        int_msg.data = static_cast<int32_t>(count_);
        int_pub_->publish(int_msg);

        // 토픽 3 - float (사인파 형태로 변화하는 값)
        std_msgs::msg::Float32 float_msg;
        float_msg.data = static_cast<float>(std::sin(count_ * 0.1) * 10.0);
        float_pub_->publish(float_msg);

        ++count_;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr string_pub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr int_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr float_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    uint64_t count_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PublisherNode>());
    rclcpp::shutdown();
    return 0;
}
