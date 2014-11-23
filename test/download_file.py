import socket
import sys
import os


_port = 12345
_ip = "127.0.0.1"
output_name = "file_read_test_result.data"


def create_socket():
	fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	fd.connect((_ip, _port))
	return fd


def send_message(fd, message):
	fd.send(message.encode())


def recv_message(fd, size):
	data = fd.recv(size)
	return data.decode('utf-8')


def close_fd(fd, output, message):
	print(message)
	fd.close()
	output.close()
	os.remove(output_name)
	sys.exit()


if __name__ == "__main__":
	fd = create_socket()
	output = open(output_name, 'w+')
	
	message_to = "file_open test/etc/file_read_test.data\r\n"
	send_message(fd, message_to)
	
	message_from = recv_message(fd, 1024)

	if "ERROR" in message_from:
		close_fd(fd, output, message_from)

	offset = 0
	size = 16
	
	while True:
		message_to = "file_read {0} {1}\r\n".format(offset, size)
		send_message(fd, message_to)
		
		message_from = recv_message(fd, 1024)
		if "ERROR" in message_from:
			close_fd(fd, output, message_from)
		
		lines = message_from.split('\r\n')
		if len(lines) != 2:
			close_fde(fd, output, "bad answer from server")

		output.write(lines[1])		
		
		bytes_count = int(lines[0].split()[1])
		if bytes_count == 0:
			break

		offset = offset + bytes_count

	message_to = "file_close\r\n"
	send_message(fd, message_to)

	fd.close()
	output.close()

	if os.system("diff " + output_name + " etc/file_read_test.data") == 0:
		print("TEST PASSED:TRUE")
	else:
		print("TEST PASSED:FALSE")

	os.remove(output_name)
