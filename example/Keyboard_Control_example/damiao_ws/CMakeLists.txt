cmake_minimum_required(VERSION 3.16)

project(keyboard_arm_control)

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

set(DM_DEVICE_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libdm_device.so)

add_executable(joint_calibrator
    src/joint_calibrator.cpp
    src/protocol/damiao.cpp
    src/protocol/usb_class.cpp
)

target_link_directories(joint_calibrator PRIVATE ${LIBUSB_LIBRARY_DIRS})
target_link_libraries(joint_calibrator PRIVATE
    ${DM_DEVICE_LIB}
    ${LIBUSB_LIBRARIES}
)

add_executable(dev_sn
    src/dev_sn.cpp
)

target_link_directories(dev_sn PRIVATE ${LIBUSB_LIBRARY_DIRS})
target_link_libraries(dev_sn PRIVATE
    ${LIBUSB_LIBRARIES}
)
