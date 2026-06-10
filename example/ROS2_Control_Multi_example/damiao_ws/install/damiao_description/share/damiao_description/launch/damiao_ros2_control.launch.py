import os
import tempfile

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    description_pkg = get_package_share_directory("damiao_description")
    urdf_file = os.path.join(description_pkg, "urdf", "damiao_test.urdf.xacro")
    controllers_file = os.path.join(description_pkg, "config", "controllers.yaml")
    motors_file = os.path.join(description_pkg, "config", "motors.yaml")

    with open(motors_file, "r", encoding="utf-8") as file:
        motors_config = yaml.safe_load(file) or {}

    with open(controllers_file, "r", encoding="utf-8") as file:
        controller_params = yaml.safe_load(file) or {}

    joint_names = list(motors_config.keys())
    controller_manager_params = controller_params.setdefault("controller_manager", {}).setdefault(
        "ros__parameters", {}
    )

    for joint_name in joint_names:
        controller_name = f"mit_cmd_controller_{joint_name}"
        controller_manager_params[controller_name] = {
            "type": "forward_command_controller/MultiInterfaceForwardCommandController"
        }
        controller_params[controller_name] = {
            "ros__parameters": {
                "joint": joint_name,
                "interface_names": [
                    "position_des",
                    "velocity_des",
                    "kp",
                    "kd",
                    "feedforward",
                ],
            }
        }

    with tempfile.NamedTemporaryFile(mode="w", suffix=".yaml", delete=False) as file:
        yaml.safe_dump(controller_params, file, sort_keys=False)
        generated_controllers_file = file.name

    robot_description_content = ParameterValue(
        Command(["xacro ", urdf_file]),
        value_type=str,
    )

    robot_description = {"robot_description": robot_description_content}

    control_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[robot_description, generated_controllers_file],
        output="screen",
    )

    robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[robot_description],
        output="screen",
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
        output="screen",
    )

    mit_cmd_controller_spawners = [
        Node(
            package="controller_manager",
            executable="spawner",
            arguments=[f"mit_cmd_controller_{joint_name}", "--controller-manager", "/controller_manager"],
            output="screen",
        )
        for joint_name in joint_names
    ]

    delay_mit_controller = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=joint_state_broadcaster_spawner,
            on_exit=mit_cmd_controller_spawners,
        )
    )

    return LaunchDescription(
        [
            control_node,
            robot_state_publisher,
            joint_state_broadcaster_spawner,
            delay_mit_controller,
        ]
    )
