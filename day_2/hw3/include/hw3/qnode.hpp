#ifndef hw3_QNODE_HPP_
#define hw3_QNODE_HPP_

#ifndef Q_MOC_RUN
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <std_msgs/msg/int32.hpp>
#include <std_msgs/msg/float32.hpp>
#include <std_srvs/srv/trigger.hpp>
#endif
#include <QThread>
#include <QString>

// QThread 기반 ROS 2 노드.
// 서로 다른 자료형(string/int/float)을 발행하는 토픽 3개를 각각
// 구독하는 3개의 subscriber를 가지며, 값이 갱신될 때마다 Qt 시그널로
// GUI 스레드에 알린다. 또한 "재시작" 서비스를 호출하는 클라이언트를 가진다.
class QNode : public QThread
{
    Q_OBJECT
public:
    QNode();
    ~QNode();

    // 대시보드의 재시작 버튼에서 호출: publisher_node를 재시작해달라는
    // 서비스 요청을 비동기로 보낸다. 결과는 restartResult 시그널로 전달된다.
    void requestRestart();

protected:
    void run() override;

private:
    std::shared_ptr<rclcpp::Node> node_;

    // 서로 다른 토픽 3개를 각각 구독하는 3개의 subscriber
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr string_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr int_sub_;
    rclcpp::Subscription<std_msgs::msg::Float32>::SharedPtr float_sub_;

    // publisher_node 재시작 서비스의 클라이언트
    rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr restart_client_;

Q_SIGNALS:
    void rosShutDown();
    void stringUpdated(const QString& value);
    void intUpdated(int value);
    void floatUpdated(double value);
    void restartResult(bool success, const QString& message);
};

#endif  // hw3_QNODE_HPP_
