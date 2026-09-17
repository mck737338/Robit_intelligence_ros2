import launch
import launch_ros.actions
import launch_ros.event_handlers
import launch_ros.events.lifecycle
import lifecycle_msgs.msg


def generate_launch_description():
    fake_imu_node = launch_ros.actions.LifecycleNode(
        package='hw1',
        executable='fake_imu',
        name='fake_imu',
        namespace='',
        parameters=[{'rate_hz': 2.0}],
        output='screen',
    )

    watchdog_node = launch_ros.actions.LifecycleNode(
        package='hw1',
        executable='watchdog',
        name='imu_watchdog',
        namespace='',
        parameters=[{'timeout_sec': 2.0, 'check_rate_hz': 2.0}],
        output='screen',
    )

    def make_auto_activate(node_action):
        """unconfigured -> configure 요청 -> inactive 도달 시 activate 요청 체인"""
        configure_event = launch.actions.EmitEvent(
            event=launch_ros.events.lifecycle.ChangeState(
                lifecycle_node_matcher=launch.events.matches_action(node_action),
                transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
            )
        )

        activate_on_inactive = launch.actions.RegisterEventHandler(
            launch_ros.event_handlers.OnStateTransition(
                target_lifecycle_node=node_action,
                start_state='configuring',
                goal_state='inactive',
                entities=[
                    launch.actions.EmitEvent(
                        event=launch_ros.events.lifecycle.ChangeState(
                            lifecycle_node_matcher=launch.events.matches_action(node_action),
                            transition_id=lifecycle_msgs.msg.Transition.TRANSITION_ACTIVATE,
                        )
                    ),
                ],
            )
        )

        return [activate_on_inactive, configure_event]

    return launch.LaunchDescription([
        fake_imu_node,
        watchdog_node,
        *make_auto_activate(fake_imu_node),
        *make_auto_activate(watchdog_node),
    ])