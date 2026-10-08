cmake_minimum_required(VERSION 3.16)
project(voxel_slam)

if(NOT ROS_EDITION)
  # Detect the ROS installation in the same way as FAST-LIO-SAM. The
  # ROS_VERSION environment variable is not always set when configuring with
  # CMake directly.
  find_package(catkin QUIET)
  if(catkin_FOUND)
    message(STATUS "ROS1 detected")
    set(ROS_EDITION ROS1)
  else()
    find_package(ament_cmake QUIET)
    if(ament_cmake_FOUND)
      message(STATUS "ROS2 detected")
      set(ROS_EDITION ROS2)
    endif()
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Eigen3 REQUIRED)
find_package(PCL REQUIRED)
find_package(GTSAM REQUIRED)
find_package(Threads REQUIRED)

if(ROS_EDITION STREQUAL "ROS1")
  add_compile_definitions(USE_ROS1)
  find_package(catkin REQUIRED COMPONENTS
    roscpp sensor_msgs geometry_msgs nav_msgs std_msgs pcl_conversions tf livox_ros_driver2)
  catkin_package(CATKIN_DEPENDS
    roscpp sensor_msgs geometry_msgs nav_msgs std_msgs pcl_conversions tf livox_ros_driver2
    DEPENDS Eigen3 PCL GTSAM)
elseif(ROS_EDITION STREQUAL "ROS2")
  add_compile_definitions(USE_ROS2)
  find_package(ament_cmake REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(nav_msgs REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(pcl_conversions REQUIRED)
  find_package(tf2_ros REQUIRED)
  find_package(livox_ros_driver2 REQUIRED)
else()
  message(FATAL_ERROR "ROS_EDITION must be ROS1 or ROS2")
endif()

add_executable(voxelslam src/voxelslam.cpp src/BTC.cpp)
target_include_directories(voxelslam PRIVATE src ${EIGEN3_INCLUDE_DIRS} ${PCL_INCLUDE_DIRS})
target_link_libraries(voxelslam ${PCL_LIBRARIES} gtsam Threads::Threads)

if(ROS_EDITION STREQUAL "ROS1")
  target_include_directories(voxelslam PRIVATE ${catkin_INCLUDE_DIRS})
  target_link_libraries(voxelslam ${catkin_LIBRARIES})
  install(TARGETS voxelslam
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
  install(DIRECTORY launch_ROS1/
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch)
  install(DIRECTORY config rviz_cfg
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
else()
  ament_target_dependencies(voxelslam
    rclcpp sensor_msgs geometry_msgs nav_msgs pcl_conversions tf2_ros livox_ros_driver2)
  install(TARGETS voxelslam DESTINATION lib/${PROJECT_NAME})
  install(DIRECTORY launch_ROS2/
    DESTINATION share/${PROJECT_NAME}/launch)
  install(DIRECTORY config rviz_cfg
    DESTINATION share/${PROJECT_NAME})
  ament_package()
endif()
