package com.example.yura.optimen;

import android.util.Log;
import java.io.File;

/**
 * Created by yura on 26.10.14.
 */
public class os_helper {
    public static void create_directory(String dirname){
        try {
            File file = new File(dirname);
            if (file.exists() && file.isDirectory()) {
                Log.i("Optimen: ", "folder " + dirname + " already exist.");
            } else {
                if (!file.mkdir()) {
                    throw new Exception("Can't create " + dirname);
                }
                config_reader cfg = new config_reader();
                String cfg_file = dirname + "/optimen_gui.conf";
                cfg.write_config_to_file(cfg_file);
            }
        }
        catch (Exception e){
            Log.e("Optimen: ", e.getMessage());
        }
    }
}