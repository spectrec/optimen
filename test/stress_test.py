import socket
import threading
import os
import time


_ip = "192.168.0.100"
_port = 12345
file_counter = 0
server_file_name = "/bg.png"
file_name_for_diff = "etc/bg.png"
client_file_name = "bg"
client_file_format = ".png"
thread_counter = 0
threads_count = 50
max_delay = 0.0
test_files = []


def recv_message(fd):
    all_data = fd.recv(1024)
    lines = all_data.split(b'\r\n')
    if len(lines) < 1 or b'ERROR' in lines[0]:
        return None

    size = 0
    headers = lines[0].decode('utf-8').split()
    if len(headers) >= 2:
        size = int(headers[1])

    offset = size
    part = all_data[len(lines[0])+2:]
    message = part
    size -= len(part)

    while size > 0:
        tmp = fd.recv(size)
        message += tmp
        size -= len(tmp)

    return (message, offset)


def do(output, fname):
    global thread_counter
    try:
        thread_counter += 1

        fd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        fd.connect((_ip, _port))

        message = "file_open {0}\r\n".format(server_file_name)
        fd.send(message.encode())
        result = recv_message(fd)
        if not result:
            fd.close()
            output.close()
            raise Exception("file_open error")

        offset = 0
        size = 4096

        while True:
            tm = time.time()
            message = "file_read {0} {1}\r\n".format(offset, size)
            fd.send(message.encode())
            result = recv_message(fd)
            tm = time.time() - tm

            global max_delay
            if tm > max_delay:
                max_delay = tm

            if not result:
                fd.close()
                output.close()
                raise Exception("file_read error")

            output.write(result[0])

            bytes_count = result[1]
            if bytes_count == 0:
                break

            offset = offset + bytes_count

        message = "file_close\r\n"
        fd.send(message.encode())

        fd.close()
        output.close()

        thread_counter -= 1

    except Exception as e:
        os.system("rm {0}".format(fname))
        test_files.remove(fname)
        thread_counter -= 1
        print(str(e))


def remove_test_files():
    for file in test_files:
        os.system("rm {0}".format(file))


if __name__ == "__main__":
    for i in range(threads_count):
        fname = client_file_name+str(file_counter)+client_file_format
        test_files.append(fname)
        output = open(fname, "wb+")
        file_counter += 1

        thread = threading.Thread(target=do, args=(output,fname))
        thread.start()

    while thread_counter != 0:
        pass

    test_result = ""
    for file in test_files:
        if os.system("diff {0} {1}".format(file_name_for_diff, file)) != 0:
            test_result = "DIFF:FALSE"
            break

    if not test_result:
        test_result = "DIFF:TRUE"
    print(test_result)
    print("Max. response time = {0}".format(str(max_delay)))

    remove_test_files()

