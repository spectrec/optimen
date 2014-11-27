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


def recv_message(fd):
	all_data = fd.recv(1024)
	lines = all_data.split(b'\r\n')
	if len(lines) < 1 or b'ERROR' in lines[0]:
		return None

	size = 0
	headers = lines[0].decode('utf-8').split()
	if len(headers) >= 2:
		size= int(headers[1])

	offset = size
	part = all_data[len(lines[0])+2:]
	message = part
	size = size - len(part)
	message = message + fd.recv(size)

	return (message, offset)


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
	
	result = recv_message(fd)
	if not result:
		close_fd(fd, output, "bad answer from server")

	offset = 0
	size = 4096
	
	while True:
		message_to = "file_read {0} {1}\r\n".format(offset, size)
		send_message(fd, message_to)
		
		result = recv_message(fd)
		if not result:
			close_fd(fd, output, "bad answer from server")

		output.write(result[0])
		
		bytes_count = result[1]
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
