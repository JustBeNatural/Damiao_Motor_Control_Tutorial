#ifndef DAMIAO_H
#define DAMIAO_H

#include <cmath>
#include <utility>
#include <vector>
#include <unordered_map>
#include <array>
#include <variant>
#include <cstdint>
#include <cmath>
#include <thread>
#include <atomic>
#include <signal.h>
#include <iostream>
#include <boost/bind/bind.hpp>
#include "protocol/usb_class.h"



namespace damiao
{

#pragma pack(1)

    /*!
     * @brief Motor Type 电机类型
     */
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

    /*
     * @brief 电机控制模式
     */
    enum Control_Mode
    {
        MIT_MODE=0x000,
        POS_VEL_MODE=0x100,
        VEL_MODE=0x200,
        POS_FORCE_MODE=0x300,
    };

    enum Control_Mode_Code
    {
        MIT=1,
        POS_VEL=2,
        VEL=3,
        POS_FORCE=4,
    };

    /*
     * @brief 寄存器列表 具体参考达妙手册
     */
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

#pragma pack()



typedef struct
{
    float Q_MAX;
    float DQ_MAX;
    float TAU_MAX;
}Limit_param;

//电机PMAX DQMAX TAUMAX参数
extern Limit_param limit_param[Num_Of_Motor];

struct DmActData
{
    DM_Motor_Type motorType;//是哪款电机
    Control_Mode mode;//电机处于那种控制模式
    uint16_t can_id;
    uint16_t mst_id;
    uint16_t channel;
};

class Motor
{
private:
    /* data */
    uint8_t channel;
    uint16_t Can_id;
    uint16_t Master_id;
    float state_q=0.0;
    float state_dq=0.0;
    float state_tau=0.0;
    Limit_param limit_param{};
    DM_Motor_Type Motor_Type;
    Control_Mode mode;

    union ValueUnion {
        float floatValue;
        uint32_t uint32Value;
    };

    struct ValueType {
        ValueUnion value;
        bool isFloat;
    };

    std::unordered_map<uint32_t , ValueType> param_map;
    std::chrono::steady_clock::time_point last_time_;
    double delta_time_;
public:

    std::chrono::system_clock::time_point stamp;
    double frequency;

    Motor(DM_Motor_Type motor_type, Control_Mode ctrl_mode,
            uint16_t can_id, uint16_t master_id,uint8_t ch);
    
    void updateTimeInterval(); 
    double getTimeInterval();

    void receive_data(float q, float dq, float tau);
    
    DM_Motor_Type GetMotorType() const { return this->Motor_Type; }
    Control_Mode  GetMotorMode() const { return this->mode; }
    Limit_param get_limit_param() { return limit_param; }//获取电机限制参数
    uint16_t GetMasterId() const { return this->Master_id; }//获取反馈ID
    uint16_t GetCanId() const { return this->Can_id; }//获取电机CAN ID
    uint16_t GetChannel() const { return this->channel; }//获取电机所在的通道，can0还是can1

    float Get_Position() const { return this->state_q; }
    float Get_Velocity() const { return this->state_dq; }
    float Get_tau() const { return this->state_tau; }
    void set_mode(Control_Mode value){ this->mode = value; }
    void set_param(int key, float value);
    void set_param(int key, uint32_t value);
    float get_param_as_float(int key) const;
    uint32_t get_param_as_uint32(int key) const ;

    bool is_have_param(int key) const; 
};


class Motor_Control
{
using clock = std::chrono::steady_clock;
using duration = std::chrono::duration<double>;

 public:
    Motor_Control(device_def_t device_type,uint32_t nom_baud,uint32_t dat_baud,std::string sn,
                    std::vector<DmActData> *data_ptr);
    ~Motor_Control();
    
     

    void addMotor(std::shared_ptr<Motor> DM_Motor);
    void enable_all();
    void disable_all();
    float read_motor_param(Motor &DM_Motor,uint8_t RID);//读取电机内部寄存器参数 RID: register id 寄存器ID
    void save_motor_param(Motor &DM_Motor);//保存电机的所有参数到flash里面 电机默认参数不会写到flash里面，需要进行写操作
    void refresh_motor_status(Motor& motor);//刷新电机状态
    
    void control_cmd(uint16_t id , uint8_t cmd,uint8_t ch);
    void write_motor_param(Motor &DM_Motor,uint8_t RID,const uint8_t data[4]);
    void set_zero_position(Motor &DM_Motor);
  
    void control_mit(Motor &DM_Motor ,float kp, float kd, float q, float dq, float tau);
    void control_pos_vel(Motor &DM_Motor, float pos,float vel);
    void control_vel(Motor &DM_Motor,float vel);
    void receive_param(uint8_t* data,uint8_t ch);

    bool switchControlMode(Motor &DM_Motor,Control_Mode_Code mode);//切换电机控制模式
    bool change_motor_param(Motor &DM_Motor,uint8_t RID,float data);//修改电机内部寄存器参数
    void changeMotorLimit(Motor &DM_Motor,float P_MAX,float Q_MAX,float T_MAX);//修改电机限制参数，这个修改的不是电机内部的寄存器参数，而是电机的限制参数
    
    //void canframeCallback(usb_rx_frame_t* frame);
    std::unordered_map<uint16_t, std::shared_ptr<Motor>>* getMotorsByChannel(uint16_t ch)
    {
        auto motorsch = motors.find(ch);

        if (motorsch == motors.end())
        {
            std::cerr << "[Error] In getMotorsByChannel,no motor with channel " << ch << " is registered." << std::endl;
            return nullptr;  // 终止程序，返回非 0 表示错误
        }
        else
        {
            return &(motorsch->second);
        }
    }


    std::shared_ptr<Motor> getMotor(uint8_t ch,uint16_t id) 
    {   
        auto motorsch = motors.find(ch);
        
        if (motorsch == motors.end())
        {
            std::cerr << "[Error] In getMotor,no motor with channel " << ch << " is registered." << std::endl;
            return nullptr;  // 终止程序，返回非 0 表示错误
        }
        if(motorsch->second.find(id) == motorsch->second.end())
        {
            std::cerr << "[Error] In getMotor,no motor with id " << id << " is registered." << std::endl;
            return nullptr;  // 终止程序，返回非 0 表示错误
        }
        else
        {
            return motorsch->second[id];
        }
        
    }

    std::shared_ptr<usb_class> getUSBHw() const {
        return usb_hw;
    }
    
    std::atomic<bool>& getRWSFlag() 
    {
        return read_write_save;
    }
    
private:

    static bool is_in_ranges(int number) {
        return (7 <= number && number <= 10) ||
                (13 <= number && number <= 16) ||
                (35 <= number && number <= 36);
    }

    static uint32_t float_to_uint32(float value) {
        return static_cast<uint32_t>(value);
    }

    static float uint32_to_float(uint32_t value) {
        return static_cast<float>(value);
    }
    static float uint8_to_float(const uint8_t data[4]) {
        uint32_t combined = (static_cast<uint32_t>(data[3]) << 24) |
                            (static_cast<uint32_t>(data[2]) << 16) |
                            (static_cast<uint32_t>(data[1]) << 8)  |
                            static_cast<uint32_t>(data[0]);
        float result;
        memcpy(&result, &combined, sizeof(result));
        return result;
    }
    device_def_t device_type_;
    //static std::string sn_;
    std::mutex m_mutex;

    std::unordered_map<uint8_t,std::unordered_map<uint16_t, std::shared_ptr<Motor>>> motors;

    std::vector<DmActData>* data_ptr_;
    std::shared_ptr<usb_class> usb_hw;

    std::atomic<bool> read_write_save{false};

};

};

#endif