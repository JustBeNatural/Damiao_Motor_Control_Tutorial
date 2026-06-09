#include "protocol/damiao.h"
#include <csignal>

// 原子标志，用于安全地跨线程修改
std::atomic<bool> running(true);

// Ctrl+C 触发的信号处理函数
void signalHandler(int signum) {

    running = false;
    //_exit(EXIT_FAILURE);
    std::cerr << "\nInterrupt signal (" << signum << ") received.\n";
}


std::shared_ptr<damiao::Motor_Control> control;
std::shared_ptr<damiao::Motor_Control> control2;

void process_data(std::shared_ptr<damiao::Motor_Control> con, usb_rx_frame_t* frame)
{ 
  static auto uint_to_float = [](uint16_t x, float xmin, float xmax, uint8_t bits) -> float {
        float span = xmax - xmin;
        float data_norm = float(x) / ((1 << bits) - 1);
        float data = data_norm * span + xmin;
        
        return data;
    };
  //frame->head.can_id;这个是mst_id不是can_id
  uint32_t canID =frame->head.can_id;
  uint8_t ch =frame->head.channel;
    
  if( con->getRWSFlag()==true&&   con->getMotorsByChannel(ch)->find(canID) !=  con->getMotorsByChannel(ch)->end())
  {//这是发送保存参数或者写参数或者读参数返回的数据
      if(frame->payload[2]==0x33 || frame->payload[2]==0x55 || frame->payload[2]==0xAA)
      {//发的是读参数或写参数命令，返回对应寄存器参数
          if(frame->payload[2]==0x33 || frame->payload[2]==0x55)
          {//写参数或者读参数返回
                con->receive_param(&frame->payload[0],ch);  
                con->getRWSFlag()=false;
          }
          con->getRWSFlag()=false;            
      }      
  }
  else
  {//这是正常返回的位置速度力矩数据
      uint16_t q_uint = (uint16_t(frame->payload[1]) << 8) | frame->payload[2];
      uint16_t dq_uint = (uint16_t(frame->payload[3]) << 4) | (frame->payload[4] >> 4);
      uint16_t tau_uint = (uint16_t(frame->payload[4] & 0xf) << 8) | frame->payload[5];

      if(  con->getMotorsByChannel(ch)->find(canID) ==  con->getMotorsByChannel(ch)->end())
      {
          return;
      }
      
      auto m =   con->getMotorsByChannel(ch)->find(canID);
      auto limit_param_receive = m->second->get_limit_param();
      float receive_q = uint_to_float(q_uint, -limit_param_receive.Q_MAX, limit_param_receive.Q_MAX, 16);

      float receive_dq = uint_to_float(dq_uint, -limit_param_receive.DQ_MAX, limit_param_receive.DQ_MAX, 12);
      float receive_tau = uint_to_float(tau_uint, -limit_param_receive.TAU_MAX, limit_param_receive.TAU_MAX, 12);
      m->second->receive_data(receive_q, receive_dq, receive_tau); 

      m->second->updateTimeInterval();
  } 
 
}

std::mutex m_mutex;
void canframeCallback(usb_rx_frame_t* frame)
{ 
  std::lock_guard<std::mutex> lock(m_mutex);
  process_data(control,frame);  
}


void canframeCallback2(usb_rx_frame_t* frame)
{ 
  std::lock_guard<std::mutex> lock(m_mutex);
  process_data(control2,frame);  
}

int main(int argc, char** argv)
{
  using clock = std::chrono::steady_clock;
  using duration = std::chrono::duration<double>;

  std::signal(SIGINT, signalHandler);

  try 
  {   
      uint16_t canid1 = 0x01;
      uint16_t mstid1 = 0x11;
      uint16_t canid2 = 0x02;
      uint16_t mstid2 = 0x12;
      uint16_t canid3 = 0x03;
      uint16_t mstid3 = 0x13;
      uint16_t canid4 = 0x04;
      uint16_t mstid4 = 0x14;
      uint16_t canid5 = 0x05;
      uint16_t mstid5 = 0x15;
      uint16_t canid6 = 0x06;
      uint16_t mstid6 = 0x16;
      uint16_t canid7 = 0x07;
      uint16_t mstid7 = 0x17;
      uint16_t canid8 = 0x08;
      uint16_t mstid8 = 0x18;
      uint16_t canid9 = 0x09;
      uint16_t mstid9 = 0x19;
      
      uint32_t nom_baud =1000000;
      uint32_t dat_baud =5000000;

      std::vector<damiao::DmActData> init_data;
      std::vector<damiao::DmActData> init_data2;

      init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
                                            .mode = damiao::MIT_MODE,
                                            .can_id=canid2,
                                            .mst_id=mstid2,
                                            .channel=CHANNEL0 });
                                            
      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid2,
      //   .mst_id=mstid2,
      //   .channel=CHANNEL0});

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid3,
      //   .mst_id=mstid3,
      //   .channel=CHANNEL0 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid4,
      //   .mst_id=mstid4,
      //   .channel=CHANNEL0 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid5,
      //   .mst_id=mstid5,
      //   .channel=CHANNEL0 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid6,
      //   .mst_id=mstid6,
      //   .channel=CHANNEL0 });
      

      //  init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid7,
      //   .mst_id=mstid7,
      //   .channel=CHANNEL0 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid8,
      //   .mst_id=mstid8,
      //   .channel=CHANNEL0 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid9,
      //   .mst_id=mstid9,
      //   .channel=CHANNEL0 });

      
      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //                                       .mode = damiao::MIT_MODE,
      //                                       .can_id=canid1,
      //                                       .mst_id=mstid1,
      //                                       .channel=CHANNEL1 });
                                            
      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid2,
      //   .mst_id=mstid2,
      //   .channel=CHANNEL1});

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid3,
      //   .mst_id=mstid3,
      //   .channel=CHANNEL1 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid4,
      //   .mst_id=mstid4,
      //   .channel=CHANNEL1 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid5,
      //   .mst_id=mstid5,
      //   .channel=CHANNEL1 });

      // init_data.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid6,
      //   .mst_id=mstid6,
      //   .channel=CHANNEL1 });
      // init_data2.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //                                       .mode = damiao::MIT_MODE,
      //                                       .can_id=canid1,
      //                                       .mst_id=mstid1,
      //                                       .channel=CHANNEL0 });
                                            
      // init_data2.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid2,
      //   .mst_id=mstid2,
      //   .channel=CHANNEL0});

      // init_data2.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid3,
      //   .mst_id=mstid3,
      //   .channel=CHANNEL0 });

      // init_data2.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid4,
      //   .mst_id=mstid4,
      //   .channel=CHANNEL0 });

      // init_data2.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid5,
      //   .mst_id=mstid5,
      //   .channel=CHANNEL0 });

      // init_data2.push_back(damiao::DmActData{.motorType = damiao::DM4310,
      //   .mode = damiao::MIT_MODE,
      //   .can_id=canid6,
      //   .mst_id=mstid6,
      //   .channel=CHANNEL0 });
      
        control = std::make_shared<damiao::Motor_Control>(
        DEV_USB2CANFD,nom_baud,dat_baud,"AA96DF2EC013B46B1BE4613798544085",&init_data);
        //接收回调函数注册
        device_hook_to_rec(control->getUSBHw()->getDeviceHandle(),canframeCallback);

        // control2 = std::make_shared<damiao::Motor_Control>( 
        // DEV_USB2CANFD,nom_baud,dat_baud,"14AA044B241402B10DDBDAFE448040BB",&init_data2);
        // device_hook_to_rec(control2->getUSBHw()->getDeviceHandle(),canframeCallback2);

        control->enable_all();//使能该接口下的所有电机
        //control2->enable_all();//使能该接口下的所有电机
      while (running)
      {
        const duration desired_duration(0.001); // 计算期望周期
        auto current_time = clock::now();
       
        control->control_mit(*control->getMotor(CHANNEL0,canid2), 0.0, 0.0, 0.0, 0.0, 0.12);
        // control->control_mit(*control->getMotor(CHANNEL0,canid2), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid3), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid4), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid5), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid6), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid7), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid8), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control->control_mit(*control->getMotor(CHANNEL0,canid9), 0.0, 0.0, 0.0, 0.0, 0.0);

        // control2->control_mit(*control2->getMotor(CHANNEL0,canid1), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control2->control_mit(*control2->getMotor(CHANNEL0,canid2), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control2->control_mit(*control2->getMotor(CHANNEL0,canid3), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control2->control_mit(*control2->getMotor(CHANNEL0,canid4), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control2->control_mit(*control2->getMotor(CHANNEL0,canid5), 0.0, 0.0, 0.0, 0.0, 0.0);
        // control2->control_mit(*control2->getMotor(CHANNEL0,canid6), 0.0, 0.0, 0.0, 0.0, 0.0);

        // for(uint16_t id = 1;id<=1;id++)
        // {
        //   float pos=control2->getMotor(CHANNEL0,id)->Get_Position();
        //   float vel=control2->getMotor(CHANNEL0,id)->Get_Velocity();
        //   float tau=control2->getMotor(CHANNEL0,id)->Get_tau();
        //   double time=control2->getMotor(CHANNEL0,id)->getTimeInterval();
        //   std::cerr<<"id is: "<<id<<" pos: "<<pos<<" vel: "<<vel<<" effort: "<<tau<<" time(s): "<<time<<std::endl;
        // }

        for(uint16_t id = 2;id<=2;id++)
        {
          float pos=control->getMotor(CHANNEL0,id)->Get_Position();
          float vel=control->getMotor(CHANNEL0,id)->Get_Velocity();
          float tau=control->getMotor(CHANNEL0,id)->Get_tau();
          double time=control->getMotor(CHANNEL0,id)->getTimeInterval();
          std::cerr<<"id is: "<<id<<" pos: "<<pos<<" vel: "<<vel<<" effort: "<<tau<<" time(s): "<<time<<std::endl;
        }
        
        // for(uint16_t id = 1;id<=1;id++)
        // {
        //   float pos=control->getMotor(CHANNEL1,id)->Get_Position();
        //   float vel=control->getMotor(CHANNEL1,id)->Get_Velocity();
        //   float tau=control->getMotor(CHANNEL1,id)->Get_tau();
        //   double time=control->getMotor(CHANNEL1,id)->getTimeInterval();
        //   std::cerr<<"id is: "<<id<<" pos: "<<pos<<" vel: "<<vel<<" effort: "<<tau<<" time(s): "<<time<<std::endl;
        // }

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        const auto sleep_till = current_time + std::chrono::duration_cast<clock::duration>(desired_duration);
        std::this_thread::sleep_until(sleep_till);    
      }

      std::cout << "The program exited safely." << std::endl;
  }
  catch (const std::exception& e) {
      std::cerr << "Error: hardware interface exception: " << e.what() << std::endl;
      return 1;
  }

  return 0;
}
