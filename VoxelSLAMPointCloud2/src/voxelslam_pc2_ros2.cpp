#include "voxelslam_pc2_ros2.hpp"

#include <cstring>
#include <memory>
#include <sstream>

#include <pluginlib/class_list_macros.hpp>
#include <rviz_common/properties/status_property.hpp>
#include <rviz_common/validate_floats.hpp>
#include <rviz_default_plugins/displays/pointcloud/point_cloud_helpers.hpp>

namespace voxelslam_pointcloud2
{

PointCloud2Display::PointCloud2Display()
: point_cloud_common_(std::make_unique<rviz_default_plugins::PointCloudCommon>(this))
{}

PointCloud2Display::~PointCloud2Display() = default;

void PointCloud2Display::onInitialize()
{
  MFDClass::onInitialize();
  point_cloud_common_->initialize(context_, scene_node_);
}

void PointCloud2Display::onDisable()
{
  MFDClass::onDisable();
  point_cloud_common_->onDisable();
}

void PointCloud2Display::processMessage(sensor_msgs::msg::PointCloud2::ConstSharedPtr cloud)
{
  if (!cloud) return;

  const int32_t xi = rviz_default_plugins::findChannelIndex(cloud, "x");
  const int32_t yi = rviz_default_plugins::findChannelIndex(cloud, "y");
  const int32_t zi = rviz_default_plugins::findChannelIndex(cloud, "z");
  if (xi < 0 || yi < 0 || zi < 0) return;

  const size_t count = static_cast<size_t>(cloud->width) * cloud->height;
  const size_t step = cloud->point_step;
  if (count * step != cloud->data.size()) {
    std::ostringstream error;
    error << "PointCloud2 data size does not match width, height and point_step";
    setStatusStd(rviz_common::properties::StatusProperty::Error, "Message", error.str());
    return;
  }

  if (count == 0) {
    point_cloud_common_->reset();
    return;
  }

  const size_t xoff = cloud->fields[xi].offset;
  const size_t yoff = cloud->fields[yi].offset;
  const size_t zoff = cloud->fields[zi].offset;
  if (step == 0 || xoff + sizeof(float) > step ||
      yoff + sizeof(float) > step || zoff + sizeof(float) > step) {
    setStatusStd(rviz_common::properties::StatusProperty::Error,
                 "Message", "PointCloud2 XYZ field offset is invalid");
    return;
  }

  auto filtered = std::make_shared<sensor_msgs::msg::PointCloud2>();
  filtered->header = cloud->header;
  filtered->fields = cloud->fields;
  filtered->is_bigendian = cloud->is_bigendian;
  filtered->is_dense = true;
  filtered->point_step = cloud->point_step;
  filtered->height = 1;
  filtered->data.resize(cloud->data.size());

  size_t valid = 0;
  for (size_t i = 0; i < count; ++i) {
    const uint8_t *point = cloud->data.data() + i * step;
    float x, y, z;
    std::memcpy(&x, point + xoff, sizeof(float));
    std::memcpy(&y, point + yoff, sizeof(float));
    std::memcpy(&z, point + zoff, sizeof(float));
    if (rviz_common::validateFloats(x) && rviz_common::validateFloats(y) &&
        rviz_common::validateFloats(z)) {
      std::memcpy(filtered->data.data() + valid * step, point, step);
      ++valid;
    }
  }

  if (valid == 0) {
    point_cloud_common_->reset();
    return;
  }

  filtered->data.resize(valid * step);
  filtered->width = valid;
  filtered->row_step = valid * step;
  point_cloud_common_->addMessage(filtered);
}

void PointCloud2Display::update(float wall_dt, float ros_dt)
{
  point_cloud_common_->update(wall_dt, ros_dt);
}

void PointCloud2Display::reset()
{
  MFDClass::reset();
  point_cloud_common_->reset();
}

}  // namespace voxelslam_pointcloud2

PLUGINLIB_EXPORT_CLASS(voxelslam_pointcloud2::PointCloud2Display, rviz_common::Display)
