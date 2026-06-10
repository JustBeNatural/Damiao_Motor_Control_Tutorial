#ifndef DAMIAO_HW_USB_CLASS_H
#define DAMIAO_HW_USB_CLASS_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include <libusb-1.0/libusb.h>
#include <unistd.h>

#include "damiao_hw/pub_user.h"

#define DM_DEV_MAX 16
#define DM_USB2CANFD_VID 0x34B7
#define DM_USB2CANFD_PID 0x6877
#define DM_USB2CANFD_DUAL_VID 0x34B7
#define DM_USB2CANFD_DUAL_PID 0x6632

class usb_class
{
public:
  usb_class(device_def_t device_type, uint32_t nom_baud, uint32_t dat_baud, const std::string & sn);
  ~usb_class();

  void usb_clear();
  std::vector<std::string> usb_get_dm_device(int * num);
  int usb_open(const std::string & serial);
  void fdcanFrameSend(std::vector<uint8_t> & data, uint32_t can_id, uint8_t ch);

  device_handle * getDeviceHandle() const { return usb_dev_; }

private:
  damiao_handle * handle_ {nullptr};
  device_handle * usb_dev_ {nullptr};
  int num_devices_ {0};
  int dm_cnt_ {0};

  mutable std::mutex mutex_;

  device_def_t device_type_;
  uint32_t nom_baud_;
  uint32_t dat_baud_;
  std::string sn_;
  bool channel0_opened_ {false};
  bool channel1_opened_ {false};
  bool device_opened_ {false};
};

#endif  // DAMIAO_HW_USB_CLASS_H
