import socket
import sys
import os


_port = 12345
_ip = "127.0.0.1"
output_name = "bg.png"


def create_socket():
	fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	fd.connect((_ip, _port))
	return fd


def send_message(fd, message):
	fd.send(message.encode())


def recv_message(fd, size):
	return fd.recv(size)


def close_fd(fd, output, message):
	print(message)
	fd.close()
	output.close()
	os.remove(output_name)
	sys.exit()


if __name__ == "__main__":
	fd = create_socket()
	output = open(output_name, 'wb+')
	
	message_to = "file_open test/etc/bg.png\r\n"
	send_message(fd, message_to)
	
	message_from = recv_message(fd, 1024)
	lines = message_from.split(b'\r\n')
	if len(lines) < 1 or b'ERROR' in lines[0]:
		close_fd(fd, output, "bad answer from server")

	offset = 0
	size = 4096
	
	while True:
		message_to = "file_read {0} {1}\r\n".format(offset, size)
		send_message(fd, message_to)
		
		message_from = recv_message(fd, 1024)
		
		lines = message_from.split(b'\r\n')
		if len(lines) < 1 or b'ERROR' in lines[0]:
			close_fd(fd, output, "bad answer from server")

		data = message_from[len(lines[0])+2:]
		output.write(data)
		
		bytes_count = int(lines[0].decode('utf-8').split()[1])
		if bytes_count == 0:
			break

		offset = offset + bytes_count

	message_to = "file_close\r\n"
	send_message(fd, message_to)

	fd.close()
	output.close()

	if os.system("diff " + output_name + " etc/bg.png") == 0:
		print("TEST PASSED:TRUE")
	else:
		print("TEST PASSED:FALSE")

	os.remove(output_name)
