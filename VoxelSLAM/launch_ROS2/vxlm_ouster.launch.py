"""Start Voxel-SLAM with the ouster configuration."""

import os

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    common_launch = os.path.join(os.path.dirname(__file__), 'vxlm.launch.py')
    return LaunchDescription([
        DeclareLaunchArgument('rviz', default_value='true'),
        DeclareLaunchArgument('save_path', default_value=os.path.expanduser('~/voxelslam')),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(common_launch),
            launch_arguments={
                'sensor': 'ouster',
                'rviz': LaunchConfiguration('rviz'),
                'save_path': LaunchConfiguration('save_path'),
            }.items(),
        ),
    ])
