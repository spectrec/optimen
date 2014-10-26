package com.example.yura.optimen;


import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;

/**
 * Created by yura on 26.10.14.
 */
public class config_reader {
    private Integer default_port = 12345;
    private String default_ip = "192.168.0.100";

    public config_reader(){
        port = default_port;
        ip = default_ip;
    }

    public void read_config(String config_name){
        try{
            FileReader f = new FileReader(config_name);
            BufferedReader br = new BufferedReader(f);

            String line;
            while ((line = br.readLine()) != null){
                line = line.replaceAll(" ", "");
                String[] parts = line.split("=");
                if (parts.length <= 0 || parts.length > 2)
                    return;
                if (parts[0].compareTo("tcp_port") == 0)
                    port = Integer.parseInt(parts[1]);
                else if (parts[0].compareTo("server_ip") == 0)
                    ip = parts[1];
            }
        }
        catch (IOException e){
            e.printStackTrace();
        }
    }

    public void write_config_to_file(String filename){
        try {
            File f = new File(filename);
            PrintWriter out = new PrintWriter(f);
            out.write("tcp_port=" + Integer.toString(port) + "\n");
            out.write("server_ip=" + ip);
            out.close();
        }
        catch (IOException e){
            e.printStackTrace();
        }
    }

    public void print_config_data(){
        Log.i("Optimen: ", "port = " + Integer.toString(port));
        Log.i("Optimen: ", "ip = " + ip);
    }

    public Integer get_port(){
        return  port;
    }

    public String get_ip(){
        return ip;
    }

    private Integer port;
    private String ip;
}
