#include "protocol/damiao.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace
{

std::atomic<bool> running(true);
std::shared_ptr<damiao::Motor_Control> control;
std::mutex callback_mutex;

constexpr uint16_t kCanId1 = 0x01;
constexpr uint16_t kMstId1 = 0x11;
constexpr uint16_t kCanId2 = 0x02;
constexpr uint16_t kMstId2 = 0x12;
constexpr uint16_t kCanId3 = 0x03;
constexpr uint16_t kMstId3 = 0x13;
constexpr uint8_t kChannel = CHANNEL0;

constexpr uint32_t kNomBaud = 1000000;
constexpr uint32_t kDatBaud = 5000000;
constexpr const char* kDeviceSn = "FB8FBE34E6C8743A8B6BB0AEEB97085F";

constexpr double kJogStep = 0.03;
constexpr double kJogVelocity = 0.08;

void signalHandler(int)
{
    running = false;
}

class RawTerminal
{
public:
    RawTerminal()
    {
        if (tcgetattr(STDIN_FILENO, &old_attr_) == 0)
        {
            valid_ = true;
            termios raw = old_attr_;
            raw.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO));
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        }
    }

    ~RawTerminal()
    {
        if (valid_)
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &old_attr_);
        }
    }

private:
    termios old_attr_{};
    bool valid_ = false;
};

int readKey()
{
    unsigned char c = 0;
    const ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n == 1)
    {
        return c;
    }
    return -1;
}

void processData(std::shared_ptr<damiao::Motor_Control> con, usb_rx_frame_t* frame)
{
    static auto uint_to_float = [](uint16_t x, float xmin, float xmax, uint8_t bits) -> float {
        const float span = xmax - xmin;
        const float data_norm = float(x) / ((1 << bits) - 1);
        return data_norm * span + xmin;
    };

    if (con == nullptr || frame == nullptr)
    {
        return;
    }

    const uint32_t can_id = frame->head.can_id;
    const uint8_t ch = frame->head.channel;
    auto motors = con->getMotorsByChannel(ch);
    if (motors == nullptr)
    {
        return;
    }

    if (con->getRWSFlag() == true && motors->find(can_id) != motors->end())
    {
        if (frame->payload[2] == 0x33 || frame->payload[2] == 0x55 || frame->payload[2] == 0xAA)
        {
            if (frame->payload[2] == 0x33 || frame->payload[2] == 0x55)
            {
                con->receive_param(&frame->payload[0], ch);
            }
            con->getRWSFlag() = false;
        }
        return;
    }

    if (motors->find(can_id) == motors->end())
    {
        return;
    }

    const uint16_t q_uint = (uint16_t(frame->payload[1]) << 8) | frame->payload[2];
    const uint16_t dq_uint = (uint16_t(frame->payload[3]) << 4) | (frame->payload[4] >> 4);
    const uint16_t tau_uint = (uint16_t(frame->payload[4] & 0xf) << 8) | frame->payload[5];

    auto motor = motors->find(can_id);
    const auto limit = motor->second->get_limit_param();
    const float q = uint_to_float(q_uint, -limit.Q_MAX, limit.Q_MAX, 16);
    const float dq = uint_to_float(dq_uint, -limit.DQ_MAX, limit.DQ_MAX, 12);
    const float tau = uint_to_float(tau_uint, -limit.TAU_MAX, limit.TAU_MAX, 12);

    motor->second->receive_data(q, dq, tau);
    motor->second->updateTimeInterval();
}

void canframeCallback(usb_rx_frame_t* frame)
{
    std::lock_guard<std::mutex> lock(callback_mutex);
    processData(control, frame);
}

void printMotor(const char* name, const std::shared_ptr<damiao::Motor>& motor,
                double target, bool selected)
{
    std::cout << (selected ? "> " : "  ")
              << name
              << "  pos=" << std::setw(8) << motor->Get_Position()
              << "  vel=" << std::setw(8) << motor->Get_Velocity()
              << "  tau=" << std::setw(8) << motor->Get_tau()
              << "  target=" << std::setw(8) << target
              << '\n';
}

} // namespace

int main()
{
    std::signal(SIGINT, signalHandler);
    RawTerminal terminal;

    try
    {
        std::vector<damiao::DmActData> init_data;
        init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
                                              .mode = damiao::POS_VEL_MODE,
                                              .can_id = kCanId1,
                                              .mst_id = kMstId1,
                                              .channel = kChannel});
        init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
                                              .mode = damiao::POS_VEL_MODE,
                                              .can_id = kCanId2,
                                              .mst_id = kMstId2,
                                              .channel = kChannel});
        init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
                                              .mode = damiao::POS_VEL_MODE,
                                              .can_id = kCanId3,
                                              .mst_id = kMstId3,
                                              .channel = kChannel});

        control = std::make_shared<damiao::Motor_Control>(
            DEV_USB2CANFD, kNomBaud, kDatBaud, kDeviceSn, &init_data);
        device_hook_to_rec(control->getUSBHw()->getDeviceHandle(), canframeCallback);
        control->enable_all();

        auto motor1 = control->getMotor(kChannel, kCanId1);
        auto motor2 = control->getMotor(kChannel, kCanId2);
        auto motor3 = control->getMotor(kChannel, kCanId3);

        int selected = 0;
        bool target_captured = false;
        double target[3] = {0.0, 0.0, 0.0};
        int print_div = 0;

        while (running)
        {
            control->refresh_motor_status(*motor1);
            control->refresh_motor_status(*motor2);
            control->refresh_motor_status(*motor3);

            const int key = readKey();
            if (key == 'q' || key == 'Q')
            {
                running = false;
            }
            else if (key == '1')
            {
                selected = 0;
            }
            else if (key == '2')
            {
                selected = 1;
            }
            else if (key == '3')
            {
                selected = 2;
            }
            else if (key == 'c' || key == 'C')
            {
                target[0] = motor1->Get_Position();
                target[1] = motor2->Get_Position();
                target[2] = motor3->Get_Position();
                target_captured = true;
            }
            else if ((key == '+' || key == '=') && target_captured)
            {
                target[selected] += kJogStep;
            }
            else if ((key == '-' || key == '_') && target_captured)
            {
                target[selected] -= kJogStep;
            }

            if (target_captured)
            {
                if (selected == 0)
                {
                    control->control_pos_vel(*motor1, target[0], kJogVelocity);
                }
                else if (selected == 1)
                {
                    control->control_pos_vel(*motor2, target[1], kJogVelocity);
                }
                else
                {
                    control->control_pos_vel(*motor3, target[2], kJogVelocity);
                }
            }

            if (++print_div >= 20)
            {
                print_div = 0;
                std::cout << "\033[2J\033[H";
                std::cout << std::fixed << std::setprecision(4);
                std::cout << "Joint calibrator | q退出 | 1/2/3选择电机 | c捕获当前位置 | +/- 点动 "
                          << kJogStep << " rad\n";
                std::cout << "状态："
                          << (target_captured ? "已捕获目标，可点动" : "只显示数据，按 c 后才允许点动")
                          << "\n\n";
                printMotor("motor1", motor1, target[0], selected == 0);
                printMotor("motor2", motor2, target[1], selected == 1);
                printMotor("motor3", motor3, target[2], selected == 2);
                std::cout << "\n当前姿态可复制为 kMotorHomeReading = {"
                          << motor1->Get_Position() << ", "
                          << motor2->Get_Position() << ", "
                          << motor3->Get_Position() << "};\n";
                std::cout << "判定方法：按 + 后，观察该关节是否沿 URDF/RViz 里的 +q 方向转。相同则 direction=+1，反向则 direction=-1。\n";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        control->disable_all();
        std::cout << "\ncalibrator exited safely.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nError: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
