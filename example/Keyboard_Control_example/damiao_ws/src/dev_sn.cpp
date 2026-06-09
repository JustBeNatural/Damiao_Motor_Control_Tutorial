#include <libusb-1.0/libusb.h>
#include <iostream>

int main() {
    // 初始化 libusb
    libusb_context* context = nullptr;
    int result = libusb_init(&context);
    if (result < 0) {
        std::cerr << "Failed to initialize libusb: " << libusb_error_name(result) << std::endl;
        return 1;
    }

    // 获取设备列表
    libusb_device** devices;
    ssize_t count = libusb_get_device_list(context, &devices);
    if (count < 0) {
        std::cerr << "Failed to obtain device list: " << libusb_error_name(count) << std::endl;
        libusb_exit(context);
        return 1;
    }

    // 遍历所有设备
    for (int i = 0; devices[i]; i++) {
        libusb_device* device = devices[i];
        
        // 获取设备描述符
        libusb_device_descriptor desc;
        result = libusb_get_device_descriptor(device, &desc);
        if (result < 0) {
            std::cerr << "Failed to obtain device descriptor: " << libusb_error_name(result) << std::endl;
            continue;
        }
        
        // 只处理有供应商ID和产品ID的设备
        if (desc.idVendor != 0x34B7 || desc.idProduct != 0x6877) {
            continue;
        }
        
        // 打开设备
        libusb_device_handle* handle = nullptr;
        result = libusb_open(device, &handle);
        if (result != LIBUSB_SUCCESS) {
            std::cerr << "Failed to open device: " << libusb_error_name(result) << std::endl;
            continue;
        }
        
        // 获取序列号
        char serial_number[256] = {0};
        if (desc.iSerialNumber > 0) {
            result = libusb_get_string_descriptor_ascii(
                handle, 
                desc.iSerialNumber,
                reinterpret_cast<unsigned char*>(serial_number),
                sizeof(serial_number)
            );
            
            if (result < 0) {
                std::cerr << "Failed to obtain serial number: " << libusb_error_name(result) << std::endl;
                serial_number[0] = '\0'; // 确保字符串为空
            }
        }
        
        // 打印设备信息
        std::cout << "U2CANFD_DEV " << i << ":" << std::endl;
        std::cout << "  VID: 0x" << std::hex << desc.idVendor << std::endl;
        std::cout << "  PID: 0x" << std::hex << desc.idProduct << std::endl;
        std::cout << "  SN: " << (serial_number[0] ? serial_number : "[No serial number]") << std::endl;
        std::cout << std::endl;
        
        // 关闭设备
        libusb_close(handle);
    }
    
    // 清理资源
    libusb_free_device_list(devices, 1);
    libusb_exit(context);
    
    return 0;
}