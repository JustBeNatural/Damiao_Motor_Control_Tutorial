#!/usr/bin/env python3

import os
from typing import Dict, List

import rclpy
import yaml
from ament_index_python.packages import get_package_share_directory
from rcl_interfaces.msg import ParameterDescriptor, ParameterType
from rclpy.node import Node
from sensor_msgs.msg import JointState
from std_msgs.msg import Float64MultiArray


class ElasticHoldNode(Node):
    def __init__(self) -> None:
        super().__init__("elastic_hold")

        config_dir = os.path.join(get_package_share_directory("damiao_description"), "config")
        motors_path = os.path.join(config_dir, "motors.yaml")
        joint_names = self._load_joint_names(motors_path)

        if not joint_names:
            raise RuntimeError(f"No joints found in {motors_path}")

        self.declare_parameter("update_rate", 50.0)
        self.declare_parameter("kp", 2.0)
        self.declare_parameter("kd", 0.1)
        self.declare_parameter("feedforward", 0.0)
        self.declare_parameter("velocity_des", 0.0)
        self.declare_parameter("capture_initial_positions", True)
        self.declare_parameter(
            "targets",
            [0.0] * len(joint_names),
            ParameterDescriptor(type=ParameterType.PARAMETER_DOUBLE_ARRAY),
        )

        self.joint_names = joint_names
        self.kp = float(self.get_parameter("kp").value)
        self.kd = float(self.get_parameter("kd").value)
        self.feedforward = float(self.get_parameter("feedforward").value)
        self.velocity_des = float(self.get_parameter("velocity_des").value)
        self.capture_initial_positions = bool(self.get_parameter("capture_initial_positions").value)
        self.targets = self._get_targets_parameter(len(joint_names))

        self.command_publishers = {
            joint_name: self.create_publisher(
                Float64MultiArray, f"/mit_cmd_controller_{joint_name}/commands", 10
            )
            for joint_name in joint_names
        }
        self.latest_positions: Dict[str, float] = {}
        self.targets_captured = not self.capture_initial_positions

        self.create_subscription(JointState, "/joint_states", self._joint_state_callback, 20)

        period = 1.0 / max(float(self.get_parameter("update_rate").value), 1.0)
        self.timer = self.create_timer(period, self._publish_commands)

        self.get_logger().info(
            f"Elastic hold ready for joints {self.joint_names}, kp={self.kp}, kd={self.kd}, "
            f"capture_initial_positions={self.capture_initial_positions}"
        )

    def _load_joint_names(self, motors_path: str) -> List[str]:
        with open(motors_path, "r", encoding="utf-8") as file:
            motors_config = yaml.safe_load(file) or {}
        return list(motors_config.keys())

    def _parse_targets(self, targets_param: List[float], expected_length: int) -> List[float]:
        targets = [float(value) for value in targets_param]
        if len(targets) != expected_length:
            self.get_logger().warn(
                f"Parameter 'targets' length {len(targets)} does not match joint count {expected_length}. "
                "Using zeros until positions are captured or parameters are updated."
            )
            return [0.0] * expected_length
        return targets

    def _get_targets_parameter(self, expected_length: int) -> List[float]:
        targets_param = self.get_parameter_or("targets", [0.0] * expected_length)
        value = targets_param.value if hasattr(targets_param, "value") else targets_param

        if value is None:
            return [0.0] * expected_length

        return self._parse_targets(value, expected_length)

    def _joint_state_callback(self, msg: JointState) -> None:
        for joint_name, position in zip(msg.name, msg.position):
            if joint_name in self.command_publishers:
                self.latest_positions[joint_name] = float(position)

        if self.targets_captured:
            return

        if all(joint_name in self.latest_positions for joint_name in self.joint_names):
            self.targets = [self.latest_positions[joint_name] for joint_name in self.joint_names]
            self.targets_captured = True
            self.get_logger().info(f"Captured initial elastic targets: {self.targets}")

    def _publish_commands(self) -> None:
        if not self.targets_captured:
            return

        for joint_name, target in zip(self.joint_names, self.targets):
            msg = Float64MultiArray()
            msg.data = [target, self.velocity_des, self.kp, self.kd, self.feedforward]
            self.command_publishers[joint_name].publish(msg)


def main() -> None:
    rclpy.init()
    node = ElasticHoldNode()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
