package com.example.yura.optimen;

import android.os.StrictMode;
import android.provider.ContactsContract;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
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
            DataInputStream din = new DataInputStream(socket.getInputStream());
            String str = "";
            while (true) {
                char c = din.readChar();
                str += c;
                if (c == '\n' || c == '\r') {
                    Log.w("Optimen: ", "READ line: " + str);
                    return -1;
                }
                /*
                din.read(buf);
                String str = new String(buf, "UTF-8");
                Log.e("Optimen: ", str);
                break;
*/
                //char c = din.readChar();
                //if (c == '\n') break;

            }

            /*
            String resp = in.readLine();
            Log.w("Optimen: ", "debug: received line " + resp);

            if (!resp.contains("OK")) {
                Log.w("Optimen: ", "can't process `file_read': " + resp);
                return -1;
            }

            // get read size
            String return_size_str = resp.substring(resp.indexOf(' ') + 1);
            Integer return_size = Integer.decode(return_size_str);

            // this means `eof'
            if (return_size == 0)
                return 0;

            Integer readed = 0;
            char data[] = new char[return_size];
            while (readed < return_size) {
                Integer left = return_size - readed;
                Log.w("Optimen: ", "left: " + left.toString());
                Integer ret = in.read(data, readed, left);
                Log.w("Optimen: ", "readed: " + ret.toString());

                if (ret == -1)
                    throw new Exception("read file error");

                readed += ret;
            }

            FileWriter fw = new FileWriter(file, true);
            fw.write(data);
            fw.flush();
            fw.close();
            */

            //return -1;
        }
        catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
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
