#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static speed_t baud_to_termios(int baud) {
	switch (baud) {
		case 1200: return B1200;
		case 2400: return B2400;
		case 4800: return B4800;
		case 9600: return B9600;
		case 19200: return B19200;
		case 38400: return B38400;
		case 57600: return B57600;
		case 115200: return B115200;
		default: return 0;
	}
}

static int hex_char_to_val(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

int hex_to_bytes(const char *hex, uint8_t *out, size_t out_cap, size_t *out_len) {
	size_t hex_len;
	size_t i;
	size_t j = 0;

	if (!hex || !out || !out_len) return -1;
	hex_len = strlen(hex);
	if (hex_len == 0 || (hex_len % 2) != 0) return -2;
	if (hex_len / 2 > out_cap) return -3;

	for (i = 0; i < hex_len; i += 2) {
		int hi = hex_char_to_val(hex[i]);
		int lo = hex_char_to_val(hex[i + 1]);
		if (hi < 0 || lo < 0) return -4;
		out[j++] = (uint8_t)((hi << 4) | lo);
	}

	*out_len = j;
	return 0;
}

uint16_t crc16_modbus(const uint8_t *data, size_t len) {
	uint16_t crc = 0xFFFF;
	size_t i;
	int j;
	for (i = 0; i < len; i++) {
		crc ^= data[i];
		for (j = 0; j < 8; j++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ 0xA001;
			} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}

void print_hex_bytes(const uint8_t *data, size_t len) {
	size_t i;
	for (i = 0; i < len; ++i) {
		printf("%02X", data[i]);
	}
	printf("\n");
}

int serial_open_8n1(const char *device, int baud) {
	int fd;
	struct termios tty;
	speed_t speed = baud_to_termios(baud);

	if (!device || speed == 0) return -1;

	fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd < 0) return -2;

	if (tcgetattr(fd, &tty) != 0) {
		close(fd);
		return -3;
	}

	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_oflag &= ~OPOST;
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	cfsetispeed(&tty, speed);
	cfsetospeed(&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_cflag |= CLOCAL | CREAD;
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
	tty.c_cflag &= ~CRTSCTS;
#endif

	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 0;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		close(fd);
		return -4;
	}

	tcflush(fd, TCIOFLUSH);
	return fd;
}

int modbus_send_and_recv_hex(const char *device,
							 int baud,
							 const char *request_hex,
							 uint8_t *response_buf,
							 size_t response_cap,
							 size_t *response_len,
							 int timeout_ms) {
	uint8_t request[512];
	size_t request_len = 0;
	size_t total_read = 0;
	int fd;
	int rc;

	if (!response_buf || !response_len || response_cap == 0) return -1;

	rc = hex_to_bytes(request_hex, request, sizeof(request), &request_len);
	if (rc != 0) return -10;

	if (request_len + 2 <= sizeof(request)) {
		uint16_t crc = crc16_modbus(request, request_len);
		request[request_len] = crc & 0xFF;         // 地位在前 CHL
		request[request_len + 1] = (crc >> 8) & 0xFF; // 高位在后 CLH
		request_len += 2;
	} else {
		return -11;
	}

	fd = serial_open_8n1(device, baud);
	if (fd < 0) return -20;

	printf("Auto CRC16 appended. Actual TX Data: ");
	print_hex_bytes(request, request_len);

	{
		ssize_t written = write(fd, request, request_len);
		if (written < 0 || (size_t)written != request_len) {
			close(fd);
			return -30;
		}
	}

	tcdrain(fd);

	while (total_read < response_cap) {
		fd_set readfds;
		struct timeval tv;
		int sel;
		ssize_t n;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;

		sel = select(fd + 1, &readfds, NULL, NULL, &tv);
		if (sel < 0) {
			close(fd);
			return -40;
		}
		if (sel == 0) {
			break;
		}

		n = read(fd, response_buf + total_read, response_cap - total_read);
		if (n < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			close(fd);
			return -50;
		}
		if (n == 0) {
			break;
		}

		total_read += (size_t)n;
	}

	close(fd);
	*response_len = total_read;
	return 0;
}

int modbus_build_and_send(const char *device, int baud,
                          uint8_t addr, uint8_t func,
                          uint16_t reg, uint16_t data,
                          uint8_t *response_buf, size_t response_cap,
                          size_t *response_len, int timeout_ms) {
    char hex_cmd[16];
    // 按照 设备地址[%02X] 读写模式[%02X] 寄存器地址[%04X] 寄存器长度[%04X] 来自动格式化生成操作HEX字符串
    snprintf(hex_cmd, sizeof(hex_cmd), "%02X%02X%04X%04X", addr, func, reg, data);
    return modbus_send_and_recv_hex(device, baud, hex_cmd, response_buf, response_cap, response_len, timeout_ms);
}

