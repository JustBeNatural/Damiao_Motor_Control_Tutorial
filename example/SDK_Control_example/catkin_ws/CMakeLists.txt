cmake_minimum_required(VERSION 3.16)
project(dm_main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

find_package(Threads REQUIRED)

message(STATUS ">>> libusb version: ${LIBUSB_VERSION}")
message(STATUS ">>> libusb libs: ${LIBUSB_LIBRARIES}")
message(STATUS ">>> libusb lib dirs: ${LIBUSB_LIBRARY_DIRS}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib
                    ${LIBUSB_INCLUDE_DIRS})

# 链接目录
link_directories(${LIBUSB_LIBRARY_DIRS})

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/protocol/damiao.cpp 
    src/protocol/usb_class.cpp
)

# 关键修改：显式指定 libusb 完整路径，确保链接 1.0.29
set(LIBUSB_FULL_PATH "/usr/local/lib/libusb-1.0.so")

target_link_libraries(${PROJECT_NAME} PRIVATE 
    -Wl,--no-as-needed
    -Wl,-rpath,/usr/local/lib
    ${LIBUSB_FULL_PATH}           # 显式路径，确保是 1.0.29
    Threads::Threads
    udev
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libdm_device.so
)

# RPATH 设置
set_target_properties(${PROJECT_NAME} PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "/usr/local/lib;${CMAKE_CURRENT_SOURCE_DIR}/lib/linux"
)

add_executable(dev_sn
    src/dev_sn.cpp
)

target_link_libraries(dev_sn PRIVATE
    ${LIBUSB_FULL_PATH}
    Threads::Threads
    udev
)