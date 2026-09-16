cmake_minimum_required(VERSION 3.5)
project(hw3)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(Qt5 COMPONENTS Widgets Core Gui Network REQUIRED)

file(GLOB PRO_UI_DIR RELATIVE ${CMAKE_SOURCE_DIR} FOLLOW_SYMLINKS  ui/*.ui)
qt5_wrap_ui(QT_UI_HPP ${PRO_UI_DIR})
qt5_wrap_cpp(QT_MOC_HPP
  include/${PROJECT_NAME}/qnode.hpp
  include/${PROJECT_NAME}/main_window.hpp
)

include_directories(
        include/
        ${CMAKE_CURRENT_BINARY_DIR}
)

# ------------------------------------------------------------------
# 1) 대시보드 GUI (QThread 기반, string/int/float 3개 토픽 subscriber)
# ------------------------------------------------------------------
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/qnode.cpp
  src/main_window.cpp
  ${QT_MOC_HPP}
  ${QT_UI_HPP}
)
ament_target_dependencies(${PROJECT_NAME} rclcpp std_msgs std_srvs)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Network)

# ------------------------------------------------------------------
# 2) 하나의 publisher가 string/int/float 3개 토픽에 발행하는 노드
# ------------------------------------------------------------------
add_executable(publisher_node src/publisher_node.cpp)
ament_target_dependencies(publisher_node rclcpp std_msgs)

# ------------------------------------------------------------------
# 3) 재시작 서비스 서버: 대시보드 버튼이 호출하면 publisher_node를 재기동
# ------------------------------------------------------------------
add_executable(restart_service_server src/restart_service_server.cpp)
ament_target_dependencies(restart_service_server rclcpp std_srvs)

install(TARGETS ${PROJECT_NAME} publisher_node restart_service_server
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
