# 键盘控制三关节达妙电机

这个工程只保留了用键盘点动控制 3 个 DM4310 电机的最小代码。

## 依赖

- Linux
- gcc/g++ 13 或系统默认支持 C++17 的编译器
- CMake 3.16+
- libusb-1.0
- 达妙 USB 转 CANFD 设备

安装 libusb：

```shell
sudo apt update
sudo apt install libusb-1.0-0-dev
```

## 编译

```shell
mkdir -p build
cd build
cmake ..
make
```

## 查询设备序列号

```shell
./dev_sn
```

把输出的 Serial Number 填到 `src/joint_calibrator.cpp` 里的 `kDeviceSn`。

## 运行键盘控制

```shell
./joint_calibrator
```

按键：

- `1` / `2` / `3`：选择电机
- `c`：捕获当前位置，之后才允许点动
- `+`：当前选中电机正向转动
- `-`：当前选中电机反向转动
- `q`：退出并失能电机

默认电机配置在 `src/joint_calibrator.cpp` 顶部：

- 电机 1：CAN ID `0x01`，MST ID `0x11`
- 电机 2：CAN ID `0x02`，MST ID `0x12`
- 电机 3：CAN ID `0x03`，MST ID `0x13`
- 通道：`CHANNEL0`

![1280X1280 (1)](三轴机械臂键盘控制程序.assets\1280X1280 (1).PNG)
