#ifndef hw1_QNODE_HPP_
#define hw1_QNODE_HPP_

#ifndef Q_MOC_RUN
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <turtlesim/srv/set_pen.hpp>
#include <turtlesim/msg/pose.hpp>
#include <turtlesim/srv/teleport_absolute.hpp>
#include <std_srvs/srv/empty.hpp>
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

    // 과제 2 - 경로 기록/재생: 저장된 pose로 순간이동시켜 경로를 재현하고,
    // 화면에 그려진 선을 모두 지운다
    void teleportAbsolute(double x, double y, double theta);
    void clearScreen();

protected:
    void run();

private:
    std::shared_ptr<rclcpp::Node> node;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::Client<turtlesim::srv::SetPen>::SharedPtr set_pen_client_;
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr pose_sub_;

    // 과제 2 - 경로 기록/재생용 서비스 클라이언트
    rclcpp::Client<turtlesim::srv::TeleportAbsolute>::SharedPtr teleport_client_;
    rclcpp::Client<std_srvs::srv::Empty>::SharedPtr clear_client_;

Q_SIGNALS:
    void rosShutDown();
    // 터틀의 실제 위치/방향이 갱신될 때마다 발생
    void poseUpdated(double x, double y, double theta);
};

#endif /* hw1_QNODE_HPP_ */