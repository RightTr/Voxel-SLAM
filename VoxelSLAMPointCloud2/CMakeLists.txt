cmake_minimum_required(VERSION 3.16)
project(voxelslam_pointcloud2)

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
set(CMAKE_AUTOMOC ON)

if(ROS_EDITION STREQUAL "ROS1")
  find_package(catkin REQUIRED COMPONENTS rviz)
  find_package(Qt5 REQUIRED COMPONENTS Widgets)
  catkin_package()

  add_library(${PROJECT_NAME} SHARED
    src/voxelslam_pc2_ros1.cpp src/voxelslam_pc2_ros1.hpp)
  target_include_directories(${PROJECT_NAME} PRIVATE ${catkin_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} Qt5::Widgets)

  install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})
  install(FILES plugin_description.xml
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
  install(DIRECTORY icons/
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/icons
    FILES_MATCHING PATTERN "*.png")
elseif(ROS_EDITION STREQUAL "ROS2")
  find_package(ament_cmake REQUIRED)
  find_package(pluginlib REQUIRED)
  find_package(rviz_common REQUIRED)
  find_package(rviz_default_plugins REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(Qt5 REQUIRED COMPONENTS Widgets)

  add_library(${PROJECT_NAME} SHARED
    src/voxelslam_pc2_ros2.cpp src/voxelslam_pc2_ros2.hpp)
  ament_target_dependencies(${PROJECT_NAME}
    pluginlib rviz_common rviz_default_plugins sensor_msgs)
  target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

  pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)
  install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib)
  install(DIRECTORY icons/ DESTINATION share/${PROJECT_NAME}/icons
    FILES_MATCHING PATTERN "*.png")
  ament_package()
else()
  message(FATAL_ERROR "ROS_EDITION must be ROS1 or ROS2")
endif()
