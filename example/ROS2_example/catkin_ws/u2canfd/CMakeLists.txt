cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER gcc-13)
set(CMAKE_CXX_COMPILER g++-13)

project(dm_main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "LIBUSB_INCLUDE_DIRS = ${LIBUSB_INCLUDE_DIRS}")
message(STATUS "LIBUSB_LIBRARIES = ${LIBUSB_LIBRARIES}")
message(STATUS "LIBUSB_LIBRARY_DIRS = ${LIBUSB_LIBRARY_DIRS}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LIBUSB_INCLUDE_DIRS}
)

link_directories(${LIBUSB_LIBRARY_DIRS})

set(DM_DEVICE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libdm_device.so)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/protocol/damiao.cpp
    src/protocol/usb_class.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${DM_DEVICE_LIB}
    usb-1.0
)

add_executable(dev_sn
    src/dev_sn.cpp
)

target_link_libraries(dev_sn PRIVATE
    usb-1.0
)
