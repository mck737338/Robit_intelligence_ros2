#include "../include/hw2/qnode.hpp"

QNode::QNode()
{
    int argc = 0;
    char** argv = NULL;
    rclcpp::init(argc, argv);
    node = rclcpp::Node::make_shared("hw2");

    cmd_vel_pub_ = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel", 10);
    set_pen_client_ = node->create_client<turtlesim::srv::SetPen>("/turtle1/set_pen");

    // 과제 2 - 경로 기록/재생용 서비스 클라이언트
    teleport_client_ = node->create_client<turtlesim::srv::TeleportAbsolute>("/turtle1/teleport_absolute");
    clear_client_ = node->create_client<std_srvs::srv::Empty>("/clear");

    pose_sub_ = node->create_subscription<turtlesim::msg::Pose>(
        "/turtle1/pose", 10,
        [this](const turtlesim::msg::Pose::SharedPtr msg) {
            Q_EMIT poseUpdated(msg->x, msg->y, msg->theta);
        });

    this->start();
}

QNode::~QNode()
{
    stopMoving();
    if (rclcpp::ok())
    {
        rclcpp::shutdown();
    }
}

void QNode::publishVelocity(double linear, double angular)
{
    if (!rclcpp::ok())
    {
        return;
    }
    auto msg = geometry_msgs::msg::Twist();
    msg.linear.x = linear;
    msg.angular.z = angular;
    cmd_vel_pub_->publish(msg);
}

void QNode::stopMoving()
{
    publishVelocity(0.0, 0.0);
}

void QNode::setPen(uint8_t r, uint8_t g, uint8_t b, uint8_t width, bool off)
{
    if (!set_pen_client_->wait_for_service(std::chrono::milliseconds(100)))
    {
        return;
    }
    auto request = std::make_shared<turtlesim::srv::SetPen::Request>();
    request->r = r;
    request->g = g;
    request->b = b;
    request->width = width;
    request->off = off;
    set_pen_client_->async_send_request(request);
}

/*****************************************************************************
** 과제 2 - 경로 기록/재생 (Path Recorder/Replayer)
*****************************************************************************/

void QNode::teleportAbsolute(double x, double y, double theta)
{
    if (!teleport_client_->wait_for_service(std::chrono::milliseconds(100)))
    {
        return;
    }
    auto request = std::make_shared<turtlesim::srv::TeleportAbsolute::Request>();
    request->x = x;
    request->y = y;
    request->theta = theta;
    teleport_client_->async_send_request(request);
}

void QNode::clearScreen()
{
    if (!clear_client_->wait_for_service(std::chrono::milliseconds(100)))
    {
        return;
    }
    auto request = std::make_shared<std_srvs::srv::Empty::Request>();
    clear_client_->async_send_request(request);
}

void QNode::run()
{
    rclcpp::WallRate loop_rate(20);
    while (rclcpp::ok())
    {
        rclcpp::spin_some(node);
        loop_rate.sleep();
    }
    rclcpp::shutdown();
    Q_EMIT rosShutDown();
}