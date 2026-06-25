#!/usr/bin/env python3
"""步进电机直接测试 - 发送 CiA402 启动序列到 /dev/ttyS9"""
import serial
import time
import struct

SERIAL_PORT = "/dev/ttyS9"
BAUD = 9600
ADDR = 0x02  # SW1-4 = 0011

def crc16(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return struct.pack("<H", crc)  # little-endian

def build_frame(addr, func, data=b""):
    frame = bytes([addr, func]) + data
    return frame + crc16(frame)

def send(ser, frame, label):
    hex_str = frame.hex(" ").upper()
    print(f">>> {label}: {hex_str}")
    ser.write(frame)
    time.sleep(0.1)
    resp = ser.read(ser.in_waiting or 1)
    if resp:
        print(f"<<< 响应: {resp.hex(' ').upper()}")
    else:
        print("<<< 无响应")

ser = serial.Serial(SERIAL_PORT, BAUD, timeout=0.3)

try:
    # Step 1: Shutdown (6040h = 0x0006)
    send(ser, build_frame(ADDR, 0x06, b"\x60\x40\x00\x06"), "1.关机(6040h=0006)")

    # Step 2: Ready (6040h = 0x0007)
    send(ser, build_frame(ADDR, 0x06, b"\x60\x40\x00\x07"), "2.就绪(6040h=0007)")

    # Step 3: Enable (6040h = 0x000F)
    send(ser, build_frame(ADDR, 0x06, b"\x60\x40\x00\x0F"), "3.使能(6040h=000F)")

    # Step 4: Speed mode (6060h = 0x0003)
    send(ser, build_frame(ADDR, 0x06, b"\x60\x60\x00\x03"), "4.速度模式(6060h=0003)")

    # Step 5: Acceleration 10 rps/s (6083h)
    send(ser, build_frame(ADDR, 0x10, b"\x60\x83\x00\x02\x04\x00\x00\x00\x0A"), "5.加速度10rps/s")

    # Step 6: Deceleration 10 rps/s (6084h)
    send(ser, build_frame(ADDR, 0x10, b"\x60\x84\x00\x02\x04\x00\x00\x00\x0A"), "6.减速度10rps/s")

    # Step 7: Target speed 200 rpm (60FFh = 0x00C8)
    send(ser, build_frame(ADDR, 0x10, b"\x60\xFF\x00\x02\x04\x00\x00\x00\xC8"), "7.转速200rpm")

    print("\n电机应开始旋转。按 Enter 停止...")
    input()

    # Stop: Shutdown
    send(ser, build_frame(ADDR, 0x06, b"\x60\x40\x00\x06"), "停止(6040h=0006)")

finally:
    ser.close()
    print("串口已关闭")