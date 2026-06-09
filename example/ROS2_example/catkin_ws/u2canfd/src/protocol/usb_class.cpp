#include "protocol/usb_class.h"

//#define DM_DEV_MAX  16       //DM-FD-CAN的设备数,这个可以调整，不一定非要16个

device_handle *dm_device[DM_DEV_MAX];

std::vector<std::string> dm_sn;

usb_class::usb_class(device_def_t device_type,uint32_t nom_baud,uint32_t dat_baud,std::string sn) 
        : device_type_(device_type),nom_baud_(nom_baud),dat_baud_(dat_baud),sn_(sn) 
{   
    
    dm_sn.resize(DM_DEV_MAX);

    handle=damiao_handle_create(device_type_);
    //打印sdk版本信息
    damiao_print_version(handle);

    std::vector<std::string> res = usb_get_dm_device(&num_devices);
    if (res.empty()) 
    {
        std::cerr << "[Error] DM-FDCAN/FDCAN_DUAL device not found, program terminated" << std::endl;
        std::exit(EXIT_FAILURE);  // 或 return -1;
    }

    int out=-1;
    out=usb_open(sn_);
    if(out<0)
    {
        std::cerr << "[Error] Failed to open usb device you specified: "<<sn_ << std::endl;
        std::exit(EXIT_FAILURE);  // 或 return -1;
    }

    //device_hook_to_sent(usb_dev,sent_callback);
    //接收钩子函数注册
    //device_hook_to_rec(usb_dev,rec_callback);

    if(device_type_==DEV_USB2CANFD)
    {
        device_channel_set_baud_with_sp(usb_dev,0,true,nom_baud_,dat_baud_,0.75f,0.75f); 
        device_baud_t baud={0};
        //获取通道波特率
        if (device_channel_get_baudrate(usb_dev,0,&baud))
        {
            printf("===can channel0 baudrate info===\n");
            printf("can_baud:%d  \n",baud.can_baudrate);
            printf("canfd_baud:%d  \n",baud.canfd_baudrate);
            printf("can_sp:%f  \n",baud.can_sp);
            printf("canfd_sp:%f  \n",baud.canfd_sp);
        }
        //开启can通道
        device_open_channel(usb_dev,0);
    }
    else if(device_type_==DEV_USB2CANFD_DUAL)
    {
        device_channel_set_baud_with_sp(usb_dev,0,true,nom_baud_,dat_baud_,0.75f,0.75f); 
        device_channel_set_baud_with_sp(usb_dev,1,true,nom_baud_,dat_baud_,0.75f,0.75f);
        device_baud_t baud0={0};
        //获取通道波特率
        if (device_channel_get_baudrate(usb_dev,0,&baud0))
        {
            printf("===can channel0 baudrate info===\n");
            printf("can_baud:%d  \n",baud0.can_baudrate);
            printf("canfd_baud:%d  \n",baud0.canfd_baudrate);
            printf("can_sp:%f  \n",baud0.can_sp);
            printf("canfd_sp:%f  \n",baud0.canfd_sp);
        }
        device_baud_t baud1={0};
        if (device_channel_get_baudrate(usb_dev,1,&baud1))
        {
            printf("===can channel1 baudrate info===\n");
            printf("can_baud:%d  \n",baud1.can_baudrate);
            printf("canfd_baud:%d  \n",baud1.canfd_baudrate);
            printf("can_sp:%f  \n",baud1.can_sp);
            printf("canfd_sp:%f  \n",baud1.canfd_sp);
        }
        device_open_channel(usb_dev,0);
        device_open_channel(usb_dev,1);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

usb_class::~usb_class()  
{   
    std::cerr<<"Enter ~usb_class"<<std::endl;

    usb_clear();
}

void usb_class::usb_clear()
{
    if(device_type_==DEV_USB2CANFD)
    {
        device_close_channel(usb_dev,0);
    }
    else if(device_type_==DEV_USB2CANFD_DUAL)
    {
        device_close_channel(usb_dev,0);
        device_close_channel(usb_dev,1);     
    }
    _exit(EXIT_FAILURE);
    //关闭设备
    device_close(usb_dev);
    
    //销毁模块句柄
    damiao_handle_destroy(handle);
    num_devices = 0;
    dm_cnt = 0;
    for (int var = 0; var < DM_DEV_MAX; ++var) {
        dm_device[var] = NULL;
    }
   //_exit(EXIT_FAILURE);
    std::cerr<<"**********USB Cleaned up**********"<<std::endl<<std::endl;
}

std::vector<std::string> usb_class::usb_get_dm_device(int* num)
{   
    int device_cnt=damiao_handle_find_devices(handle);
    std::cerr << "device_cnt " << device_cnt<<std::endl;
    std::vector<std::string> result;
    int handle_cnt=0;
    device_handle* dev_list[16];
    //获取设备信息
    damiao_handle_get_devices(handle,dev_list,&handle_cnt);
    std::cerr << "handle_cnt " << handle_cnt<<std::endl;
    for (int i = 0; i < handle_cnt; i++)
    {
        int id_product = 0;
        int id_vendor = 0;    
        device_get_pid_vid(dev_list[0], &id_product, &id_vendor);

        if (id_vendor == DM_USB2CANFD_VID && id_product == DM_USB2CANFD_PID)
        {           
            dm_device[dm_cnt] = dev_list[i];
                
            char strBuf[255]={0};       
            device_get_serial_number(dev_list[i],strBuf,sizeof(strBuf));
            std::cerr << "Found DM-FDCAN, SN: " <<strBuf<< std::endl;
            
            dm_sn[dm_cnt]=std::string(strBuf);
           
            dm_cnt++;
        } 
        else if (id_vendor == DM_USB2CANFD_DUAL_VID && id_product == DM_USB2CANFD_DUAL_PID)
        {           
            dm_device[dm_cnt] = dev_list[i];
                
            char strBuf[255]={0};       
            device_get_serial_number(dev_list[i],strBuf,sizeof(strBuf));
            std::cerr << "Found DM-FDCAN_DUAL, SN: " <<strBuf<< std::endl;
            
            dm_sn[dm_cnt]=std::string(strBuf);
           
            dm_cnt++;
        }     
    }
   
    if (dm_cnt == 0)
    {   
        if(device_type_==DEV_USB2CANFD)
        {
            std::cerr << "[Error] DM-FDCAN device not found" << std::endl;
        } 
        else if(device_type_==DEV_USB2CANFD_DUAL)
        {
            std::cerr << "[Error] DM-FDCAN_DUAL device not found" << std::endl;
        }
        *num = 0;
        return result;
    }
    else
    {
        //std::cerr << "found DM-FDCAN, cnt = " << dm_cnt << std::endl;
        *num = dm_cnt;

        result = dm_sn;
        return result;
    }
}


int usb_class::usb_open(std::string str)
{
    for (int var = 0; var < DM_DEV_MAX; ++var) 
    {   
        if(str == dm_sn[var])
        {
            usb_dev = dm_device[var];
            std::cerr << "Choose usb device is " << dm_sn[var] << std::endl;
            break;
        }
    }

    if( usb_dev == NULL )//如果没有发现设备的话
    {
        std::cerr<<"[Error] Not found device you specified: "<<str<<std::endl;
        return -1;
    }
    if (device_open(usb_dev))
    {
        printf("device opened !\n");
    }
    else
    {
        printf("open device failed !\n");
        damiao_handle_destroy(handle);
        return -1;
    }

    return 1;
}

//  device_channel_send_fast(
//     dev_list[0],   // 设备句柄
//     0,             // 通道号
//     0x123,         // CAN ID
//     1,             // 发送次数（-1 表示持续发送）
//     false,         // 是否扩展帧
//     true,          // 是否 CAN FD
//     true,          // 是否启用 BRS（波特率切换）
//     8,             // 数据长度
//     payload        // 数据内容
// );
void usb_class::fdcanFrameSend(std::vector< uint8_t>& data,uint32_t canId, uint8_t ch)
{   
    uint8_t* payload = data.data();
    device_channel_send_fast(usb_dev,ch,canId,1,false,true,true,8,payload);
}



// void usb_class::rec_callback(usb_rx_frame_t* frame)
// {
//    //printf("rec callback , packet id:%x\n",frame->head.can_id);
// }

// void usb_class::sent_callback(usb_rx_frame_t* frame)
// { 
//     //printf("sent callback , packet id:%x\n",frame->head.can_id);
// }