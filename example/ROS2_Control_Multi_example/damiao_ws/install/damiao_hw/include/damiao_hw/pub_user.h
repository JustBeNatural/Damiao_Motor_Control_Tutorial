#ifndef DM_DEVICE_PUB_USER_H
#define DM_DEVICE_PUB_USER_H

#ifdef _WIN32
  #ifdef DEVICE_EXPORTS
    #define DEVICE_API __declspec(dllexport)
  #else
    #define DEVICE_API __declspec(dllimport)
  #endif
#else
  #define DEVICE_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus

#include <stdint.h>

extern "C"
{
  typedef struct damiao_handle hDamiaoUsb;
  typedef struct device_handle hDevice;

#pragma pack(push, 1)
  typedef struct
  {
    uint32_t can_id : 29;
    uint32_t esi : 1;
    uint32_t ext : 1;
    uint32_t rtr : 1;
    uint8_t canfd : 1;
    uint8_t brs : 1;
    uint8_t id_inc : 1;
    uint8_t data_inc : 1;
    uint8_t dlc : 4;
    uint8_t channel;
    uint16_t reserved;
    uint16_t step_id;
    uint32_t stop_id;
    uint32_t interval;
    int32_t send_times;
  } usb_tx_frame_head_t;

  typedef struct
  {
    usb_tx_frame_head_t head;
    uint8_t payload[64];
  } usb_tx_frame_t;

  typedef struct
  {
    uint32_t can_id : 29;
    uint32_t esi : 1;
    uint32_t ext : 1;
    uint32_t rtr : 1;
    uint64_t time_stamp;
    uint8_t channel;
    uint8_t canfd : 1;
    uint8_t dir : 1;
    uint8_t brs : 1;
    uint8_t ack : 1;
    uint8_t dlc : 4;
    uint16_t reserved;
  } usb_rx_frame_head_t;

  typedef struct
  {
    usb_rx_frame_head_t head;
    uint8_t payload[64];
  } usb_rx_frame_t;

  typedef struct
  {
    int can_baudrate;
    int canfd_baudrate;
    float can_sp;
    float canfd_sp;
  } device_baud_t;

  typedef enum
  {
    DEV_None = -1,
    DEV_USB2CANFD = 0,
    DEV_USB2CANFD_DUAL,
    DEV_ECAT2CANFD
  } device_def_t;

  typedef struct
  {
    uint8_t channel;
    uint8_t can_fd;
    uint8_t can_seg1;
    uint8_t can_seg2;
    uint8_t can_sjw;
    uint8_t can_prescaler;
    uint8_t canfd_seg1;
    uint8_t canfd_seg2;
    uint8_t canfd_sjw;
    uint8_t canfd_prescaler;
  } dmcan_ch_can_config_t;
#pragma pack(pop)

  typedef void (*dev_rec_callback)(usb_rx_frame_t * rec_frame);
  typedef void (*dev_sent_callback)(usb_rx_frame_t * sent_frame);
  typedef void (*dev_err_callback)(usb_rx_frame_t * err_frame);

  DEVICE_API damiao_handle * damiao_handle_create(device_def_t type);
  DEVICE_API void damiao_handle_destroy(damiao_handle * handle);
  DEVICE_API void damiao_print_version(damiao_handle * handle);
  DEVICE_API void damiao_get_sdk_version(damiao_handle * handle, char * version_buf, size_t buf_size);
  DEVICE_API int damiao_handle_find_devices(damiao_handle * handle);
  DEVICE_API void damiao_handle_get_devices(damiao_handle * handle, device_handle ** dev_list, int * device_count);
  DEVICE_API void device_get_version(device_handle * dev, char * version_buf, size_t buf_size);
  DEVICE_API void device_get_pid_vid(device_handle * dev, int * pid, int * vid);
  DEVICE_API void device_get_serial_number(device_handle * dev, char * serial_buf, size_t buf_size);
  DEVICE_API void device_get_type(device_handle * dev, device_def_t * type);
  DEVICE_API bool device_open(device_handle * dev);
  DEVICE_API bool device_close(device_handle * dev);
  DEVICE_API bool device_save_config(device_handle * dev);
  DEVICE_API bool device_open_channel(device_handle * dev, uint8_t channel);
  DEVICE_API bool device_close_channel(device_handle * dev, uint8_t channel);
  DEVICE_API bool device_channel_get_baudrate(device_handle * dev, uint8_t channel, device_baud_t * baud);
  DEVICE_API bool device_channel_set_baud(
    device_handle * dev, uint8_t channel, bool canfd, int bitrate, int dbitrate);
  DEVICE_API bool device_channel_set_baud_with_sp(
    device_handle * dev, uint8_t channel, bool canfd, int bitrate, int dbitrate, float can_sp, float canfd_sp);
  DEVICE_API void device_hook_to_rec(device_handle * dev, dev_rec_callback callback);
  DEVICE_API void device_hook_to_sent(device_handle * dev, dev_sent_callback callback);
  DEVICE_API void device_hook_to_err(device_handle * dev, dev_err_callback callback);
  DEVICE_API void device_channel_send(device_handle * dev, usb_tx_frame_t frame);
  DEVICE_API void device_channel_send_fast(
    device_handle * dev, uint8_t ch, uint32_t can_id, int32_t cnt, bool ext, bool canfd, bool brs, uint8_t len,
    uint8_t * payload);
  DEVICE_API void device_channel_send_advanced(
    device_handle * dev, uint8_t ch, uint32_t can_id, uint16_t step_id, uint32_t stop_id, int32_t cnt, bool id_inc,
    bool data_inc, bool ext, bool canfd, bool brs, uint8_t len, uint8_t * payload);
}

#endif

#endif
