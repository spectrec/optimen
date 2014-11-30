package com.example.yura.optimen;

import android.os.StrictMode;
import android.util.Log;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetAddress;
import java.net.Socket;

/**
 * Created by yura on 26.10.14.
 */
public class command_processor
{
    private static Character END_LS = '.';
    private static Integer READ_SIZE = 4096;

    public static Socket process_command_file_open(Integer port, String ip, String file_name)
    {
        try {
            Socket socket = get_socket(port, ip);
            BufferedReader in = get_sock_in(socket);
            DataOutputStream out = get_sock_out(socket);

            String request = "file_open /" + file_name + "\n";
            out.writeBytes(request);
            out.flush();

            String resp = in. readLine();
            if (resp.contains("OK"))
                return socket;

            Log.w("Optimen: ", "can't process `file_open': " + resp);
        }
        catch (IOException e) {
            e.printStackTrace();
        }

        return null;
    }

    public static Boolean process_command_file_close(Socket socket)
    {
        try {
            BufferedReader in = get_sock_in(socket);
            DataOutputStream out = get_sock_out(socket);

            String request = "file_close\n";
            out.writeBytes(request);
            out.flush();

            String resp = in.readLine();
            socket.close();

            if (resp.contains("OK"))
                return true;

            Log.w("Optimen: ", "can't process `file_close': " + resp);
        }
        catch (IOException e) {
            e.printStackTrace();
        }

        return false;
    }

    public static Integer process_command_file_read(Socket socket, File file, Integer already_read)
    {
        try {
            DataOutputStream out = get_sock_out(socket);

            // send request
            Integer offset = already_read;
            String request = "file_read " + offset.toString() + " " + READ_SIZE.toString() + "\n";
            out.writeBytes(request);
            out.flush();

            // get response
            DataInputStream in = new DataInputStream(socket.getInputStream());

            // get header
            String header = new String();
            while (true) {
                byte[] bytes = new byte[1];
                in.read(bytes, 0, 1);
                if ((char)bytes[0] == 'E')
                    return -1;
                if ((char)bytes[0] == '\n')
                    break;
                header += (char)bytes[0];
            }

            String[] header_parts = header.split(" ");
            if (header_parts.length < 2)
                return -1;

            Integer size = Integer.decode(header_parts[1].trim());
            if (size == 0)
                return 0;

            already_read += size;
            FileOutputStream f_stream = new FileOutputStream(file, true);

            while (size > 0) {
                byte[] buf = new byte[size];
                Integer readed = in.read(buf, 0, size);
                size -= readed;
                byte[] tmp = new byte[readed];
                System.arraycopy(buf, 0, tmp, 0, readed);
                f_stream.write(tmp);
            }

            f_stream.close();

            return already_read;
        }
        catch (IOException e) {
            e.printStackTrace();
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return -1;
    }

    public static optimen_list process_command_ls(Integer port, String ip, String dir)
    {
        optimen_list result_list = new optimen_list();

        try {
            Socket socket = get_socket(port, ip);

            BufferedReader in = get_sock_in(socket);
            DataOutputStream out = get_sock_out(socket);

            String question = "ls " + dir + "\n";
            out.writeBytes(question);
            out.flush();

            String answer = in.readLine();
            if (answer.contains("OK")) {
                Boolean inActive = true;
                while (inActive) {
                    answer = in.readLine();
                    // all data received
                    if (answer.charAt(0) == END_LS) {
                        inActive = false;
                        Log.i("Optimen: ", "all data received");
                    }
                    else {
                        dir_data tmp = new dir_data();
                        tmp.setType(answer.charAt(0));
                        tmp.setName(answer.substring(2));
                        result_list.add_element(tmp);
                    }
                }
            }
            else
                Log.i("Optimen: ", "answer from server not contains OK identifier");

            socket.close();
        }
        catch (IOException e){
            e.printStackTrace();
        }

        return result_list;
    }

    public static Socket get_socket(Integer port, String ip){
        Socket socket = null;

        try {
            StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
            InetAddress ipAddress = InetAddress.getByName(ip);
            socket = new Socket(ipAddress, port);
        }
        catch (IOException e){
            e.printStackTrace();
        }
        catch (Exception e){
            e.printStackTrace();
        }

        return socket;
    }

    private static DataOutputStream get_sock_out(Socket socket){
        DataOutputStream stream = null;

        try {
            stream = new DataOutputStream(socket.getOutputStream());
        }
        catch (IOException e){
            e.printStackTrace();
        }

        return stream;
    }

    private static BufferedReader get_sock_in(Socket socket){
        BufferedReader br = null;

        try{
            br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        }
        catch (IOException e){
            e.printStackTrace();
        }

        return br;
    }
}
