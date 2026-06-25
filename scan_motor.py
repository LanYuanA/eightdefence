import serial, struct, time

def crc16(d):
    c = 0xFFFF
    for b in d:
        c ^= b
        for _ in range(8): c = (c >> 1) ^ 0xA001 if c & 1 else c >> 1
    return struct.pack("<H", c)

def test(addr, baud):
    try:
        s = serial.Serial("/dev/ttyS9", baud, timeout=0.2)
        payload = bytes([addr, 0x06, 0x60, 0x40, 0x00, 0x06])
        frame = payload + crc16(payload)
        s.write(frame)
        time.sleep(0.2)
        r = s.read(s.in_waiting or 1)
        s.close()
        if r:
            print(f"  >>> 响应! addr=0x{addr:02X} baud={baud} 响应={r.hex(' ').upper()}")
            return True
        else:
            print(f"      无响应  addr=0x{addr:02X} baud={baud}")
            return False
    except Exception as e:
        print(f"      错误 addr=0x{addr:02X} baud={baud}: {e}")
        return False

print("=== 步进电机通讯扫描 ===")
print("发送指令: 06 6040 0006 (关机)")
print()

baud_rates = [4800, 9600, 14400, 19200, 38400, 57600, 115200]
addrs = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]

found = False
for baud in baud_rates:
    print(f"[波特率 {baud}]")
    for addr in addrs:
        if test(addr, baud):
            found = True
    print()

print("=== 扫描结束 ===")
if not found:
    print("所有组合均无响应！请检查:")
    print("1. 电机电源(24V/48V)是否接通, 电机指示灯是否亮?")
    print("2. RS485 A/B 线是否接反? 试试对调")
    print("3. /dev/ttyS9 是否正确?")