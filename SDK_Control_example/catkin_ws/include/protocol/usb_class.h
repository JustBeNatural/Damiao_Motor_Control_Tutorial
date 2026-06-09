#ifndef USB_CLASS_H
#define USB_CLASS_H

#include <stdint.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <libusb-1.0/libusb.h>
#include <vector>
#include <string>
#include <atomic>
#include <cstring> 
#include <iomanip>
#include <functional>
#include <mutex>

#include <unistd.h>
#include <stdio.h>
#include "protocol/pub_user.h"

#define DM_DEV_MAX  16 //DM-FD-CAN的设备数,这个可以调整，不一定非要16个

#define CHANNEL0           0x00
#define CHANNEL1           0x01

//DM-FD-CAN的PID和VID
#define DM_USB2CANFD_VID           0x34B7
#define DM_USB2CANFD_PID           0x6877  

#define DM_USB2CANFD_DUAL_VID           0x34B7
#define DM_USB2CANFD_DUAL_PID           0x6632 

class usb_class 
{  
using clock = std::chrono::steady_clock;
using duration = std::chrono::duration<double>;
// using FrameCallback = std::function<void(usb_rx_frame_t*)>;

public:
    usb_class(device_def_t device_type,uint32_t nom_baud,uint32_t dat_baud,std::string sn);
    ~usb_class();

    void usb_clear();
    std::vector<std::string> usb_get_dm_device(int* num);
    int usb_open(std::string str);
    
    void fdcanFrameSend(std::vector< uint8_t>& data,uint32_t canId, uint8_t ch);
    
    // static void rec_callback(usb_rx_frame_t* frame);
    // static void sent_callback(usb_rx_frame_t* frame);
    
    device_handle* getDeviceHandle() const {
        return usb_dev;
    }

private:
    damiao_handle* handle;

   
    device_handle* usb_dev= NULL;

    int num_devices = 0;
    int dm_cnt = 0;

    mutable std::mutex mutex_;
   
    device_def_t device_type_;
    uint32_t nom_baud_;
    uint32_t dat_baud_;//数据域波特率
    std::string sn_;//设备的serial_number
};
#endif // USB_CLASS_H
