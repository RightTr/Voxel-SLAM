#pragma once

#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <Eigen/Geometry>

#if defined(USE_ROS1)
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/PointCloud2.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>
#include <livox_ros_driver2/CustomMsg.h>
#include <tf/transform_broadcaster.h>

using RosNode = ros::NodeHandle;
using ImuMsg = sensor_msgs::Imu;
using PointCloud2Msg = sensor_msgs::PointCloud2;
using PathMsg = nav_msgs::Path;
using PoseStampedMsg = geometry_msgs::PoseStamped;
using LivoxMsg = livox_ros_driver2::CustomMsg;
using ImuMsgPtr = ImuMsg::Ptr;
using ImuMsgConstPtr = ImuMsg::ConstPtr;
using PointCloud2MsgConstPtr = PointCloud2Msg::ConstPtr;
using LivoxMsgConstPtr = LivoxMsg::ConstPtr;
using PointCloud2Publisher = ros::Publisher;
using PathPublisher = ros::Publisher;
using ImuSubscriber = ros::Subscriber;
using PointCloud2Subscriber = ros::Subscriber;
using LivoxSubscriber = ros::Subscriber;
using RosStamp = ros::Time;

inline RosNode *&ros_node_storage() { static RosNode *node = nullptr; return node; }
inline void init_ros_node(RosNode &node) { ros_node_storage() = &node; }
inline void ros_init(int argc, char **argv) { ros::init(argc, argv, "cmn_voxel"); }
inline void ros_shutdown() { ros::shutdown(); }
inline bool ros_ok() { return ros::ok(); }
inline void spin_once() { ros::spinOnce(); }
inline RosStamp ros_now() { return ros::Time::now(); }
inline double ros_time_sec(const RosStamp &stamp) { return stamp.toSec(); }
inline double ros_now_sec() { return ros_time_sec(ros_now()); }
inline RosStamp ros_stamp_from_sec(double seconds) { return ros::Time().fromSec(seconds); }
inline void ros_warn(const char *message) { ROS_WARN("%s", message); }

template<typename T>
inline void rosparam_get(const std::string &name, T &value, const T &fallback) {
  ros_node_storage()->param<T>(name, value, fallback);
}
inline void rosparam_get(const std::string &name, std::string &value, const char *fallback) {
  rosparam_get(name, value, std::string(fallback));
}

template<typename T>
inline PointCloud2Publisher create_publisher(const std::string &topic, size_t depth) {
  return ros_node_storage()->advertise<T>(topic, depth);
}

template<typename T, typename Callback>
inline ros::Subscriber create_sensor_subscriber(const std::string &topic, size_t depth, Callback callback) {
  return ros_node_storage()->subscribe<T>(topic, depth, callback);
}

inline void ros_publish(PointCloud2Publisher &publisher, const PointCloud2Msg &message) {
  publisher.publish(message);
}
inline void ros_publish(PathPublisher &publisher, const PathMsg &message) {
  publisher.publish(message);
}

inline void publish_transform(const Eigen::Vector3d &position, const Eigen::Quaterniond &orientation,
                              const std::string &parent, const std::string &child) {
  static tf::TransformBroadcaster broadcaster;
  tf::Transform transform;
  transform.setOrigin(tf::Vector3(position.x(), position.y(), position.z()));
  transform.setRotation(tf::Quaternion(orientation.x(), orientation.y(), orientation.z(), orientation.w()));
  broadcaster.sendTransform(tf::StampedTransform(transform, ros_now(), parent, child));
}

#elif defined(USE_ROS2)
#include <rclcpp/rclcpp.hpp>
#include <builtin_interfaces/msg/time.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <nav_msgs/msg/path.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <livox_ros_driver2/msg/custom_msg.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2_ros/transform_broadcaster.h>

using RosNode = rclcpp::Node::SharedPtr;
using ImuMsg = sensor_msgs::msg::Imu;
using PointCloud2Msg = sensor_msgs::msg::PointCloud2;
using PathMsg = nav_msgs::msg::Path;
using PoseStampedMsg = geometry_msgs::msg::PoseStamped;
using LivoxMsg = livox_ros_driver2::msg::CustomMsg;
using ImuMsgPtr = ImuMsg::SharedPtr;
using ImuMsgConstPtr = ImuMsg::ConstSharedPtr;
using PointCloud2MsgConstPtr = PointCloud2Msg::ConstSharedPtr;
using LivoxMsgConstPtr = LivoxMsg::ConstSharedPtr;
using PointCloud2Publisher = rclcpp::Publisher<PointCloud2Msg>::SharedPtr;
using PathPublisher = rclcpp::Publisher<PathMsg>::SharedPtr;
using ImuSubscriber = rclcpp::Subscription<ImuMsg>::SharedPtr;
using PointCloud2Subscriber = rclcpp::Subscription<PointCloud2Msg>::SharedPtr;
using LivoxSubscriber = rclcpp::Subscription<LivoxMsg>::SharedPtr;
using RosStamp = builtin_interfaces::msg::Time;

inline RosNode &ros_node_storage() { static RosNode node; return node; }
inline void init_ros_node(RosNode &node) { ros_node_storage() = node; }
inline void ros_init(int argc, char **argv) { rclcpp::init(argc, argv); }
inline void ros_shutdown() { rclcpp::shutdown(); }
inline bool ros_ok() { return rclcpp::ok(); }
inline void spin_once() { rclcpp::spin_some(ros_node_storage()); }
inline RosStamp ros_now() { return ros_node_storage()->get_clock()->now(); }
inline double ros_time_sec(const RosStamp &stamp) {
  return static_cast<double>(stamp.sec) + static_cast<double>(stamp.nanosec) * 1e-9;
}
inline double ros_now_sec() { return ros_time_sec(ros_now()); }
inline RosStamp ros_stamp_from_sec(double seconds) {
  return rclcpp::Time(static_cast<int64_t>(std::llround(seconds * 1e9)), RCL_ROS_TIME);
}
inline void ros_warn(const char *message) { RCLCPP_WARN(ros_node_storage()->get_logger(), "%s", message); }

template<typename T>
inline void rosparam_get(const std::string &name, T &value, const T &fallback) {
  auto node = ros_node_storage();
  if (!node->has_parameter(name)) node->declare_parameter<T>(name, fallback);
  value = node->get_parameter(name).get_value<T>();
}
inline void rosparam_get(const std::string &name, std::string &value, const char *fallback) {
  rosparam_get(name, value, std::string(fallback));
}

template<typename T>
inline typename rclcpp::Publisher<T>::SharedPtr create_publisher(const std::string &topic, size_t depth) {
  return ros_node_storage()->create_publisher<T>(topic, rclcpp::QoS(rclcpp::KeepLast(depth)));
}

template<typename T, typename Callback>
inline typename rclcpp::Subscription<T>::SharedPtr create_sensor_subscriber(
    const std::string &topic, size_t depth, Callback callback) {
  return ros_node_storage()->create_subscription<T>(topic, rclcpp::SensorDataQoS().keep_last(depth), callback);
}

inline void ros_publish(const PointCloud2Publisher &publisher, const PointCloud2Msg &message) {
  publisher->publish(message);
}
inline void ros_publish(const PathPublisher &publisher, const PathMsg &message) {
  publisher->publish(message);
}

inline void publish_transform(const Eigen::Vector3d &position, const Eigen::Quaterniond &orientation,
                              const std::string &parent, const std::string &child) {
  static auto broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(ros_node_storage());
  geometry_msgs::msg::TransformStamped transform;
  transform.header.stamp = ros_now();
  transform.header.frame_id = parent;
  transform.child_frame_id = child;
  transform.transform.translation.x = position.x();
  transform.transform.translation.y = position.y();
  transform.transform.translation.z = position.z();
  transform.transform.rotation.x = orientation.x();
  transform.transform.rotation.y = orientation.y();
  transform.transform.rotation.z = orientation.z();
  transform.transform.rotation.w = orientation.w();
  broadcaster->sendTransform(transform);
}

#else
#error "Build with USE_ROS1 or USE_ROS2"
#endif
