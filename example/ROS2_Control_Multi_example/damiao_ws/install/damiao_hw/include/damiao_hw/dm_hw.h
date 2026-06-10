#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "damiao_hw/damiao.h"

namespace damiao
{

class DmHW : public hardware_interface::SystemInterface
{
public:
  hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo & info) override;
  hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State & previous_state) override;
  hardware_interface::CallbackReturn on_cleanup(const rclcpp_lifecycle::State & previous_state) override;
  hardware_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State & previous_state) override;
  hardware_interface::CallbackReturn on_deactivate(const rclcpp_lifecycle::State & previous_state) override;
  hardware_interface::CallbackReturn on_error(const rclcpp_lifecycle::State & previous_state) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) override;
  hardware_interface::return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:
  struct JointBinding
  {
    std::string device_serial;
    uint16_t can_id {0};
    uint8_t channel {CHANNEL0};
    damiao::Motor_Control * driver {nullptr};
    std::shared_ptr<damiao::Motor> motor;
  };

  static DM_Motor_Type stringToMotorType(const std::string & type_str);
  static uint8_t parseChannel(const std::string & channel_str);
  static Control_Mode_Code toControlModeCode(Control_Mode mode);

  std::vector<damiao::DmActData> hw_actuator_data_;
  std::vector<JointBinding> joint_bindings_;
  std::unordered_map<std::string, std::vector<damiao::DmActData>> device_to_motor_configs_;
  std::unordered_map<std::string, std::unique_ptr<damiao::Motor_Control>> motor_controls_;
};

RCLCPP_SHARED_PTR_DEFINITIONS(DmHW)

}  // namespace damiao
