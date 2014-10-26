package com.example.yura.optimen;

import android.os.StrictMode;
import android.util.Log;

import java.io.BufferedReader;
import java.io.DataInput;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;

/**
 * Created by yura on 26.10.14.
 */
public class command_processor
{
    private static Character END_LS = '.';

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
