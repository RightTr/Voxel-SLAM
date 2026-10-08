"""Launch Voxel-SLAM with one of the existing sensor configurations."""

import os

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


CONFIGS = ('avia', 'avia_fly', 'hesai', 'mid360', 'ouster', 'velodyne')
DOUBLE_PARAMETERS = {
    'General/blind',
    'Odometry/cov_gyr', 'Odometry/cov_acc', 'Odometry/rdw_gyr',
    'Odometry/rdw_acc', 'Odometry/down_size', 'Odometry/dept_err',
    'Odometry/beam_err', 'Odometry/voxel_size', 'Odometry/min_eigen_value',
    'LocalBA/cov_gyr', 'LocalBA/cov_acc', 'LocalBA/rdw_gyr',
    'LocalBA/rdw_acc', 'LocalBA/imu_coef',
    'Loop/jud_default', 'Loop/icp_eigval', 'Loop/ratio_drift',
    'GBA/voxel_size', 'GBA/min_eigen_value',
}


def flatten_parameters(values, prefix=''):
    result = {}
    for key, value in values.items():
        name = f'{prefix}/{key}' if prefix else key
        if isinstance(value, dict):
            result.update(flatten_parameters(value, name))
        elif isinstance(value, list) and all(
                isinstance(item, (int, float)) and not isinstance(item, bool)
                for item in value):
            result[name] = [float(item) for item in value]
        elif name in DOUBLE_PARAMETERS and isinstance(value, (int, float)):
            result[name] = float(value)
        else:
            result[name] = value
    return result


def launch_nodes(context):
    sensor = LaunchConfiguration('sensor').perform(context)
    if sensor not in CONFIGS:
        raise ValueError(f'Unknown sensor {sensor!r}; choose one of {CONFIGS}')

    share = get_package_share_directory('voxel_slam')
    with open(os.path.join(share, 'config', sensor + '.yaml'), encoding='utf-8') as stream:
        parameters = flatten_parameters(yaml.safe_load(stream))
    parameters['finish'] = False
    save_path = LaunchConfiguration('save_path').perform(context)
    os.makedirs(save_path, exist_ok=True)
    parameters['General/save_path'] = os.path.join(save_path, '')

    actions = [Node(package='voxel_slam', executable='voxelslam', name='voxelslam',
                    output='screen', parameters=[parameters])]
    if LaunchConfiguration('rviz').perform(context).lower() in ('true', '1', 'yes'):
        actions.append(Node(package='rviz2', executable='rviz2', name='rviz2',
                            output='screen', arguments=['-d', os.path.join(
                                share, 'rviz_cfg', 'back_ros2.rviz')]))
    return actions


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('sensor', default_value='mid360',
                              description='Sensor configuration: ' + ', '.join(CONFIGS)),
        DeclareLaunchArgument('rviz', default_value='true',
                              description='Start RViz2 with standard PointCloud2 displays'),
        DeclareLaunchArgument('save_path', default_value=os.path.expanduser('~/voxelslam'),
                              description='Directory for maps and state output'),
        OpaqueFunction(function=launch_nodes),
    ])
