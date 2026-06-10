#ifndef DAMIAO_HW_DAMIAO_H
#define DAMIAO_HW_DAMIAO_H

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "damiao_hw/usb_class.h"

namespace damiao
{

constexpr uint8_t CHANNEL0 = 0x00;
constexpr uint8_t CHANNEL1 = 0x01;

enum DM_Motor_Type
{
  DM3507,
  DM4310,
  DM4310_48V,
  DM4340,
  DM4340_48V,
  DM6006,
  DM8006,
  DM8009,
  DM10010L,
  DM10010,
  DMH3510,
  DMH6215,
  DMG6220,
  Num_Of_Motor
};

enum Control_Mode
{
  MIT_MODE = 0x000,
  POS_VEL_MODE = 0x100,
  VEL_MODE = 0x200,
  POS_FORCE_MODE = 0x300,
};

enum Control_Mode_Code
{
  MIT = 1,
  POS_VEL = 2,
  VEL = 3,
  POS_FORCE = 4,
};

enum DM_REG
{
  UV_Value = 0,
  KT_Value = 1,
  OT_Value = 2,
  OC_Value = 3,
  ACC = 4,
  DEC = 5,
  MAX_SPD = 6,
  MST_ID = 7,
  ESC_ID = 8,
  TIMEOUT = 9,
  CTRL_MODE = 10,
  Damp = 11,
  Inertia = 12,
  hw_ver = 13,
  sw_ver = 14,
  SN = 15,
  NPP = 16,
  Rs = 17,
  LS = 18,
  Flux = 19,
  Gr = 20,
  PMAX = 21,
  VMAX = 22,
  TMAX = 23,
  I_BW = 24,
  KP_ASR = 25,
  KI_ASR = 26,
  KP_APR = 27,
  KI_APR = 28,
  OV_Value = 29,
  GREF = 30,
  Deta = 31,
  V_BW = 32,
  IQ_c1 = 33,
  VL_c1 = 34,
  can_br = 35,
  sub_ver = 36,
  u_off = 50,
  v_off = 51,
  k1 = 52,
  k2 = 53,
  m_off = 54,
  dir = 55,
  p_m = 80,
  xout = 81,
};

struct Limit_param
{
  float Q_MAX;
  float DQ_MAX;
  float TAU_MAX;
};

extern Limit_param limit_param[Num_Of_Motor];

struct DmActData
{
  std::string name;
  DM_Motor_Type motorType {DM4310};
  Control_Mode mode {MIT_MODE};
  uint16_t can_id {0};
  uint16_t mst_id {0};
  uint8_t channel {CHANNEL0};

  double pos {0.0};
  double vel {0.0};
  double effort {0.0};

  double cmd_pos {0.0};
  double cmd_vel {0.0};
  double cmd_effort {0.0};
  double kp {0.0};
  double kd {0.0};
};

class Motor
{
public:
  Motor(DM_Motor_Type motor_type, Control_Mode ctrl_mode, uint16_t can_id, uint16_t master_id, uint8_t ch);

  void receive_data(float q, float dq, float tau);

  DM_Motor_Type GetMotorType() const { return motor_type_; }
  Control_Mode GetMotorMode() const;
  Limit_param get_limit_param() const;
  uint16_t GetMasterId() const { return master_id_; }
  uint16_t GetCanId() const { return can_id_; }
  uint8_t GetChannel() const { return channel_; }

  float Get_Position() const;
  float Get_Velocity() const;
  float Get_tau() const;

  void set_mode(Control_Mode value);
  void set_param(int key, float value);
  void set_param(int key, uint32_t value);
  float get_param_as_float(int key) const;
  uint32_t get_param_as_uint32(int key) const;
  bool is_have_param(int key) const;

  bool has_feedback() const;
  double get_feedback_period() const;

private:
  uint8_t channel_;
  uint16_t can_id_;
  uint16_t master_id_;
  DM_Motor_Type motor_type_;

  mutable std::mutex mutex_;
  float state_q_ {0.0f};
  float state_dq_ {0.0f};
  float state_tau_ {0.0f};
  Limit_param limit_param_ {};
  Control_Mode mode_;
  bool feedback_received_ {false};
  std::chrono::steady_clock::time_point last_feedback_time_;
  double feedback_period_ {0.0};

  union ValueUnion
  {
    float floatValue;
    uint32_t uint32Value;
  };

  struct ValueType
  {
    ValueUnion value;
    bool isFloat;
  };

  std::unordered_map<uint32_t, ValueType> param_map_;
};

class Motor_Control
{
public:
  Motor_Control(device_def_t device_type, uint32_t nom_baud, uint32_t dat_baud, const std::string & sn,
                std::vector<DmActData> * data_ptr);
  ~Motor_Control();

  void addMotor(const std::shared_ptr<Motor> & motor);

  void enable_all();
  void disable_all();
  float read_motor_param(Motor & motor, uint8_t rid);
  void save_motor_param(Motor & motor);
  void refresh_motor_status(Motor & motor);
  void control_cmd(uint16_t id, uint8_t cmd, uint8_t ch);
  void write_motor_param(Motor & motor, uint8_t rid, const uint8_t data[4]);
  void set_zero_position(Motor & motor);

  void control_mit(Motor & motor, float kp, float kd, float q, float dq, float tau);
  void control_pos_vel(Motor & motor, float pos, float vel);
  void control_vel(Motor & motor, float vel);
  void receive_param(uint8_t * data, uint8_t ch);

  bool switchControlMode(Motor & motor, Control_Mode_Code mode);
  bool change_motor_param(Motor & motor, uint8_t rid, float data);
  void changeMotorLimit(Motor & motor, float p_max, float q_max, float t_max);

  std::shared_ptr<Motor> getMotor(uint8_t ch, uint16_t id) const;
  std::shared_ptr<usb_class> getUSBHw() const { return usb_hw_; }

private:
  static constexpr std::size_t kMaxCallbackSlots = 4;
  static constexpr uint8_t kMitEnableCmd = 0xFC;
  static constexpr uint8_t kMitDisableCmd = 0xFD;
  static constexpr int kMaxRetries = 20;
  static constexpr useconds_t kRetryIntervalUs = 2000;

  static void callback0(usb_rx_frame_t * frame);
  static void callback1(usb_rx_frame_t * frame);
  static void callback2(usb_rx_frame_t * frame);
  static void callback3(usb_rx_frame_t * frame);
  static void dispatch_callback(std::size_t slot, usb_rx_frame_t * frame);

  static bool is_in_ranges(int number)
  {
    return (7 <= number && number <= 10) || (13 <= number && number <= 16) || (35 <= number && number <= 36);
  }

  static uint32_t float_to_uint32(float value) { return static_cast<uint32_t>(value); }
  static float uint32_to_float(uint32_t value) { return static_cast<float>(value); }
  static float uint8_to_float(const uint8_t data[4])
  {
    uint32_t combined = (static_cast<uint32_t>(data[3]) << 24) | (static_cast<uint32_t>(data[2]) << 16) |
                        (static_cast<uint32_t>(data[1]) << 8) | static_cast<uint32_t>(data[0]);
    float result = 0.0f;
    std::memcpy(&result, &combined, sizeof(result));
    return result;
  }

  static uint16_t float_to_uint(float x, float xmin, float xmax, uint8_t bits);
  static float uint_to_float(uint16_t x, float xmin, float xmax, uint8_t bits);

  void handle_frame(usb_rx_frame_t * frame);
  int acquire_callback_slot();
  void release_callback_slot();

  device_def_t device_type_;
  std::vector<DmActData> * data_ptr_;
  std::shared_ptr<usb_class> usb_hw_;

  mutable std::mutex mutex_;
  std::atomic<bool> read_write_save_ {false};

  std::vector<std::shared_ptr<Motor>> motor_list_;
  std::unordered_map<uint8_t, std::unordered_map<uint16_t, std::shared_ptr<Motor>>> motor_lookup_;
  int callback_slot_ {-1};

  static std::mutex callback_slots_mutex_;
  static std::array<Motor_Control *, kMaxCallbackSlots> callback_slots_;
};

}  // namespace damiao

#endif  // DAMIAO_HW_DAMIAO_H
