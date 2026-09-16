// 과제 3 - 다중 토픽 모니터링 대시보드
// 대시보드의 "재시작" 버튼이 호출하는 서비스 서버.
// publisher_node를 자식 프로세스로 직접 관리하며, 서비스가 호출되면
// 기존 프로세스를 종료(SIGTERM)한 뒤 새 프로세스로 재기동시킨다.

#include <rclcpp/rclcpp.hpp>
#include <std_srvs/srv/trigger.hpp>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <memory>
#include <string>

class RestartServiceServer : public rclcpp::Node
{
public:
    RestartServiceServer() : Node("restart_service_server"), child_pid_(-1)
    {
        service_ = create_service<std_srvs::srv::Trigger>(
            "/publisher_node/restart",
            std::bind(&RestartServiceServer::handleRestart, this,
                      std::placeholders::_1, std::placeholders::_2));

        launchPublisher();
        RCLCPP_INFO(get_logger(),
                    "restart_service_server 시작: /publisher_node/restart 서비스 대기 중");
    }

    ~RestartServiceServer() override
    {
        killPublisher();
    }

private:
    void launchPublisher()
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            // 자식 프로세스: publisher_node 실행
            execlp("ros2", "ros2", "run", "hw3", "publisher_node", static_cast<char*>(nullptr));
            // execlp가 실패한 경우에만 도달
            _exit(1);
        }
        else if (pid > 0)
        {
            child_pid_ = pid;
            RCLCPP_INFO(get_logger(), "publisher_node 프로세스 시작 (pid=%d)", child_pid_);
        }
        else
        {
            RCLCPP_ERROR(get_logger(), "publisher_node 프로세스를 fork하지 못했습니다.");
        }
    }

    void killPublisher()
    {
        if (child_pid_ > 0)
        {
            kill(child_pid_, SIGTERM);
            int status = 0;
            waitpid(child_pid_, &status, 0);
            child_pid_ = -1;
        }
    }

    void handleRestart(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> /*request*/,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response)
    {
        RCLCPP_INFO(get_logger(), "재시작 요청 수신: publisher_node를 재기동합니다.");
        killPublisher();
        launchPublisher();

        if (child_pid_ > 0)
        {
            response->success = true;
            response->message = "publisher_node를 재시작했습니다 (pid=" +
                                 std::to_string(child_pid_) + ").";
        }
        else
        {
            response->success = false;
            response->message = "publisher_node 재시작에 실패했습니다.";
        }
    }

    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr service_;
    pid_t child_pid_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<RestartServiceServer>());
    rclcpp::shutdown();
    return 0;
}
