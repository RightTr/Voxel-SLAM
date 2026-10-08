#!/bin/bash

# Voxel-SLAM ROS1/ROS2 build helper, following the FAST-LIO-SAM workflow.
readonly VERSION_ROS1="ROS1"
readonly VERSION_ROS2="ROS2"
readonly VERSION_HUMBLE="humble"

pushd "$(pwd)" >/dev/null
cd "$(dirname "$0")"
echo "Working Path: $(pwd)"

ROS_VERSION=""
ROS_HUMBLE=""
if [ "${1:-}" = "$VERSION_ROS1" ]; then
  ROS_VERSION="$VERSION_ROS1"
elif [ "${1:-}" = "$VERSION_ROS2" ]; then
  ROS_VERSION="$VERSION_ROS2"
elif [ "${1:-}" = "$VERSION_HUMBLE" ]; then
  ROS_VERSION="$VERSION_ROS2"
  ROS_HUMBLE="$VERSION_HUMBLE"
else
  echo "Usage: $0 ROS1|ROS2|humble" >&2
  exit 2
fi
echo "ROS version is: $ROS_VERSION"

# ROS tooling uses the system Python installation (catkin_pkg/ament packages
# are installed there). Keep an active Conda environment from leaking into
# CMake even when the caller forgot to run `conda deactivate` first.
if [ -n "${CONDA_PREFIX:-}" ] && command -v conda >/dev/null 2>&1; then
  conda deactivate >/dev/null 2>&1 || true
fi
python3_executable="/usr/bin/python3"
if [ ! -x "$python3_executable" ]; then
  python3_executable="$(command -v python3)"
fi

repo_dir="$(pwd)"
workspace_dir="$(cd "$repo_dir/../.." && pwd)"
voxel_manifest="$repo_dir/VoxelSLAM/package.xml"
plugin_manifest="$repo_dir/VoxelSLAMPointCloud2/package.xml"
plugin_description="$repo_dir/VoxelSLAMPointCloud2/plugin_description.xml"

voxel_backup="$(mktemp)"
plugin_backup="$(mktemp)"
description_backup="$(mktemp)"
cp "$voxel_manifest" "$voxel_backup"
cp "$plugin_manifest" "$plugin_backup"
cp "$plugin_description" "$description_backup"
restore_files() {
  cp "$voxel_backup" "$voxel_manifest"
  cp "$plugin_backup" "$plugin_manifest"
  cp "$description_backup" "$plugin_description"
  rm -f "$voxel_backup" "$plugin_backup" "$description_backup"
}
trap restore_files EXIT

if [ "$ROS_VERSION" = "$VERSION_ROS1" ]; then
  cp "$repo_dir/VoxelSLAM/package_ROS1.xml" "$voxel_manifest"
  cp "$repo_dir/VoxelSLAMPointCloud2/package_ROS1.xml" "$plugin_manifest"
  cp "$repo_dir/VoxelSLAMPointCloud2/plugin_description_ROS1.xml" "$plugin_description"
  pushd "$workspace_dir" >/dev/null
  catkin_make -DROS_EDITION="$VERSION_ROS1"
  popd >/dev/null
else
  cp "$repo_dir/VoxelSLAM/package_ROS2.xml" "$voxel_manifest"
  cp "$repo_dir/VoxelSLAMPointCloud2/package_ROS2.xml" "$plugin_manifest"
  cp "$repo_dir/VoxelSLAMPointCloud2/plugin_description_ROS2.xml" "$plugin_description"
  pushd "$workspace_dir" >/dev/null
  # Match Livox and FAST-LIO-SAM: build the complete workspace with colcon's
  # default isolated layout. A full build refreshes the workspace-level
  # setup.bash so it includes Livox and both Voxel-SLAM packages.
  cmake_clean_cache=()
  if [ -f install/.colcon_install_layout ] && grep -qx "merged" install/.colcon_install_layout; then
    backup_dir="install_merged_backup_$(date +%Y%m%d_%H%M%S)"
    mv install "$backup_dir"
    cmake_clean_cache+=(--cmake-clean-cache)
    echo "Detected merged install; rebuilding with the default isolated layout"
    echo "Previous install saved to: $workspace_dir/$backup_dir"
  fi
  colcon_args=(
    --build-base build
    --install-base install
    --cmake-args
    -DROS_EDITION="$VERSION_ROS2"
    -DHUMBLE_ROS="$ROS_HUMBLE"
    "-DPython3_EXECUTABLE=$python3_executable"
  )
  colcon --log-base log build "${cmake_clean_cache[@]}" "${colcon_args[@]}"
  popd >/dev/null
fi

popd >/dev/null
