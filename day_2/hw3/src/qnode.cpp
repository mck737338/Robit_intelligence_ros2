#include "../include/hw3/qnode.hpp"

namespace
{
constexpr const char* kStringTopic = "/dashboard/string_topic";
constexpr const char* kIntTopic = "/dashboard/int_topic";
constexpr const char* kFloatTopic = "/dashboard/float_topic";
constexpr const char* kRestartService = "/publisher_node/restart";
}  // namespace

QNode::QNode()
{
    int argc = 0;
    char** argv = nullptr;
    rclcpp::init(argc, argv);
    node_ = rclcpp::Node::make_shared("hw3_dashboard");

    // 토픽 1 - string
    string_sub_ = node_->create_subscription<std_msgs::msg::String>(
        kStringTopic, 10,
        [this](const std_msgs::msg::String::SharedPtr msg) {
            Q_EMIT stringUpdated(QString::fromStdString(msg->data));
        });

    // 토픽 2 - int
    int_sub_ = node_->create_subscription<std_msgs::msg::Int32>(
        kIntTopic, 10,
        [this](const std_msgs::msg::Int32::SharedPtr msg) {
            Q_EMIT intUpdated(msg->data);
        });

    // 토픽 3 - float
    float_sub_ = node_->create_subscription<std_msgs::msg::Float32>(
        kFloatTopic, 10,
        [this](const std_msgs::msg::Float32::SharedPtr msg) {
            Q_EMIT floatUpdated(static_cast<double>(msg->data));
        });

    restart_client_ = node_->create_client<std_srvs::srv::Trigger>(kRestartService);

    this->start();
}

QNode::~QNode()
{
    if (rclcpp::ok())
    {
        rclcpp::shutdown();
    }
    wait();
}

void QNode::requestRestart()
{
    if (!restart_client_->wait_for_service(std::chrono::milliseconds(200)))
    {
        Q_EMIT restartResult(false, "재시작 서비스(/publisher_node/restart)를 찾을 수 없습니다. "
                                     "restart_service_server가 실행 중인지 확인하세요.");
        return;
    }

    auto request = std::make_shared<std_srvs::srv::Trigger::Request>();
    restart_client_->async_send_request(
        request,
        [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture future) {
            auto response = future.get();
            Q_EMIT restartResult(response->success, QString::fromStdString(response->message));
        });
}

void QNode::run()
{
    rclcpp::WallRate loop_rate(20);
    while (rclcpp::ok())
    {
        rclcpp::spin_some(node_);
        loop_rate.sleep();
    }
    rclcpp::shutdown();
    Q_EMIT rosShutDown();
}
