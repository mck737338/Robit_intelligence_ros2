cmake_minimum_required(VERSION 3.8)
project(hw1)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

include_directories(include)

add_executable(fake_imu src/fake_imu.cpp)
ament_target_dependencies(fake_imu
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
  std_msgs
  sensor_msgs
)

add_executable(watchdog src/watchdog.cpp)
ament_target_dependencies(watchdog
  rclcpp
  rclcpp_lifecycle
  lifecycle_msgs
  std_msgs
  sensor_msgs
)

install(TARGETS
  fake_imu
  watchdog
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch/
  DESTINATION share/${PROJECT_NAME}/launch
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()