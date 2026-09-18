import os
from launch import LaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    robot_description_content = Command([
        PathJoinSubstitution([FindExecutable(name="xacro")]), " ",
        PathJoinSubstitution([FindPackageShare("robot_description"), "urdf", "eclipse.xacro"]),
        " with_base:=true",
        " with_arm:=true",
        " with_camera_tower:=true",
        " use_mock_base:=true",
        " use_mock_arm:=true",
        " use_mock_ct:=true",
        " mock_sensor_commands:=true",
    ])
    robot_description = {"robot_description": robot_description_content}

    controllers_yaml = PathJoinSubstitution(
        [FindPackageShare("eclipse_bringup_hong"), "config", "eclipse_controllers.yaml"]
    )
    rviz_config = PathJoinSubstitution(
        [FindPackageShare("eclipse_bringup_hong"), "rviz", "eclipse.rviz"]
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[robot_description],
    )

    controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_description, controllers_yaml],
        output="screen",
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
    )

    diff_drive_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_controller", "--controller-manager", "/controller_manager"],
    )

    flipper_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["flipper_controller", "--controller-manager", "/controller_manager"],
    )

    arm_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["arm_controller", "--controller-manager", "/controller_manager"],
    )

    gripper_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["gripper_controller", "--controller-manager", "/controller_manager"],
    )

    camera_tower_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["camera_tower_controller", "--controller-manager", "/controller_manager"],
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", rviz_config],
        output="screen",
    )

    # joint_state_broadcaster가 active 된 후 나머지 컨트롤러들을 순서대로 스폰
    delay_after_jsb = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=[diff_drive_spawner, flipper_spawner, arm_spawner,
                     gripper_spawner, camera_tower_spawner, rviz_node],
        )
    )

    return LaunchDescription([
        robot_state_publisher,
        controller_manager,
        joint_state_broadcaster_spawner,
        delay_after_jsb,
    ])