#ifndef hw1_QNODE_HPP_
#define hw1_QNODE_HPP_

#ifndef Q_MOC_RUN
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <turtlesim/srv/set_pen.hpp>
#include <turtlesim/msg/pose.hpp>
#endif
#include <QThread>
#include <cstdint>

class QNode : public QThread
{
    Q_OBJECT
public:
    QNode();
    ~QNode();

    void publishVelocity(double linear, double angular);
    void stopMoving();
    void setPen(uint8_t r, uint8_t g, uint8_t b, uint8_t width, bool off = false);

protected:
    void run();

private:
    std::shared_ptr<rclcpp::Node> node;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr set_pen_client_;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;

Q_SIGNALS:
    void rosShutDown();
    // 터틀의 실제 위치/방향이 갱신될 때마다 발생
    void poseUpdated(double x, double y, double theta);
};

#endif /* hw1_QNODE_HPP_ */