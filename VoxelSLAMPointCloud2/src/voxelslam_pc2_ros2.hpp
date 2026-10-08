#pragma once

#include <memory>

#include <rviz_common/message_filter_display.hpp>
#include <rviz_default_plugins/displays/pointcloud/point_cloud_common.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

namespace voxelslam_pointcloud2
{

class PointCloud2Display : public rviz_common::MessageFilterDisplay<sensor_msgs::msg::PointCloud2>
{
public:
  PointCloud2Display();
  ~PointCloud2Display() override;

  void reset() override;
  void update(float wall_dt, float ros_dt) override;

protected:
  void onInitialize() override;
  void onDisable() override;
  void processMessage(sensor_msgs::msg::PointCloud2::ConstSharedPtr cloud) override;

private:
  std::unique_ptr<rviz_default_plugins::PointCloudCommon> point_cloud_common_;
};

}  // namespace voxelslam_pointcloud2
