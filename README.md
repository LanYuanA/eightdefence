# Modbus 串口读数测试（`/dev/ttyS9`）

## 功能
- 发送十六进制请求（默认：`300300010001D1EB`）
- 从串口读取返回数据
- 以十六进制打印返回（`RX HEX`）

## 文件
- `test.c`：API + 测试入口
- `test_modbus`：编译后的可执行程序

## API
`int modbus_send_and_recv_hex(const char *device, int baud, const char *request_hex, uint8_t *response_buf, size_t response_cap, size_t *response_len, int timeout_ms)`

返回值：
- `0`：成功
- `<0`：失败（例如打开串口失败）

## 编译
```bash
gcc -O2 -Wall -Wextra -std=c11 test.c -o test_modbus
```

## 运行
默认参数（设备`/dev/ttyS9`，波特率`9600`，超时`300ms`）：
```bash
./test_modbus
```

自定义参数：
```bash
./test_modbus /dev/ttyS9 300300010001D1EB 9600 300
```

参数顺序：
1. 串口设备
2. 请求hex字符串（无空格）
3. 波特率
4. 超时毫秒

## 输出示例
```text
TX HEX: 300300010001D1EB
RX HEX: 30030201F4....
RX LEN: 7 bytes
```
