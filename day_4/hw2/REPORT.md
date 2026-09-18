**ROS2 4일차 과제2 보고서
예비단원 김민찬

---

## CP1 — Launch 한 번으로 bringup, 모든 컨트롤러 active

```bash
ros2 launch eclipse_bringup_hong eclipse_bringup.launch.py
```

robot_state_publisher, controller_manager, 6개 컨트롤러 spawner(joint_state_broadcaster, diff_drive_controller, flipper_controller, arm_controller, gripper_controller, camera_tower_controller)를 하나의 launch 파일에서 순차 실행한다.

---

## CP2 — 카메라 타워 관절 4개 명령

```bash
ros2 topic pub --once /camera_tower_controller/commands \
  std_msgs/msg/Float64MultiArray "{data: [0.5, -0.5, 0.3, -0.3]}"
```

camera_tower_controller(JointGroupPositionController)에 ct_joint1_1, ct_joint1_2, ct_joint2_1, ct_joint2_2 순서로 4개의 목표 position 값을 배열로 전달한다.

---

## CP3 — 팔 관절 6개, 컨트롤러 하나로 궤적 명령

```bash
ros2 topic pub --once /arm_controller/joint_trajectory \
  trajectory_msgs/msg/JointTrajectory \
"{joint_names: [arm_joint1, arm_joint2, arm_joint3, arm_joint4, arm_joint5, arm_joint6], points: [{positions: [0.3, 0.2, 1.0, 0.5, 0.5, 0.3], time_from_start: {sec: 3}}]}"
```

arm_controller(JointTrajectoryController) 하나에 arm_joint1~6의 이름과 목표 position, 도달 시간(3초)을 포함한 단일 trajectory point를 전달한다.

---

## CP4 — 베이스의 바퀴와 플리퍼 명령

### 4-1. 바퀴 (diff_drive_controller)

```bash
ros2 topic pub --once /diff_drive_controller/cmd_vel \
  geometry_msgs/msg/TwistStamped \
  "{header: {stamp: {sec: 0, nanosec: 0}, frame_id: ''}, twist: {linear: {x: 0.2, y: 0.0, z: 0.0}, angular: {x: 0.0, y: 0.0, z: 0.0}}}"
```

diff_drive_controller가 구독하는 TwistStamped 타입 토픽에 header와 선속도 x=0.2인 twist 값을 전달한다.

### 4-2. 플리퍼 (flipper_controller)

```bash
ros2 topic pub --once /flipper_controller/commands \
  std_msgs/msg/Float64MultiArray "{data: [1.0, 1.0, 1.0, 1.0]}"
```

flipper_controller(JointGroupPositionController)에 fl_joint, fr_joint, bl_joint, br_joint 순서로 4개의 목표 position 값 1.0을 배열로 전달한다.
