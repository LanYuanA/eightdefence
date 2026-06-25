/**
 * @file dev_stepper_motor.hpp
 * @brief 一体式步进电机 (RS485 / CiA402 over Modbus)
 *
 * 适用型号: JKISS42-R04A、JKSS57-R03A 等
 * 协议: CANopen CiA 402 映射到 Modbus RTU 寄存器
 *   - 6040h 控制字 (0x06写单寄存器)
 *   - 6060h 运行模式 (3=速度模式)
 *   - 6083h 加速度 (0x10写双寄存器, 32-bit, rps/s)
 *   - 6084h 减速度 (0x10写双寄存器, 32-bit, rps/s)
 *   - 60FFh 目标速度 (0x10写双寄存器, 32-bit, -3000~3000 rpm)
 *
 * 启动序列:
 *   1. 写控制字 0x0006 (关机)
 *   2. 写控制字 0x0007 (上电就绪)
 *   3. 写控制字 0x000F (使能运行)
 *   4. 写模式 0x0003 (速度模式)
 *   5~7. 写加减速度+目标速度
 */

#ifndef DEV_STEPPER_MOTOR_HPP
#define DEV_STEPPER_MOTOR_HPP

#include "device_base.hpp"
#include <atomic>
#include <string>

class DevStepperMotor : public DeviceBase {
public:
    DevStepperMotor(const std::string& name, uint8_t addr)
        : name_(name), addr_(addr) {}

    void init() override;
    std::vector<DeviceTask> getTasks() override;
    std::string getName() const override { return name_; }

    // 控制接口 (供外部调用)
    bool start(ModbusService& svc, int speedRpm);
    bool stop(ModbusService& svc);

    bool isRunning() const { return running_.load(); }
    int  getSpeed() const { return speed_.load(); }
    bool isOnline() { return status_.isOnline(); }
    uint8_t getAddr() const { return addr_; }

private:
    std::string name_;
    uint8_t     addr_;
    DeviceStatusCpp status_;
    std::atomic<bool> running_{false};
    std::atomic<int>  speed_{0};
};

#endif
