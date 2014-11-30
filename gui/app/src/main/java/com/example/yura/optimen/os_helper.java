package com.example.yura.optimen;

import android.util.Log;
import java.io.File;
import java.io.FileWriter;

/**
 * Created by yura on 26.10.14.
 */
public class os_helper {
    public static Integer create_directory(String dirname){
        try {
            File file = new File(dirname);

            if (file.exists() && file.isDirectory()) {
                Log.i("Optimen: ", "folder " + dirname + " already exist.");

                return 0;
            } else {
                if (!file.mkdir()) {
                    throw new Exception("Can't create " + dirname);
                }
            }
        }
        catch (Exception e){
            Log.e("Optimen: ", e.getMessage());
        }

        return 1;
    }

    public static File open_file(String file_name) {
        try {
            File file = new File(file_name);

            if (file.exists()) {
                // truncate file
                FileWriter fw = new FileWriter(file);
                fw.close();
            }
            else if (!file.createNewFile()) {
                Log.w("Optimen: ", "can't create file: " + file_name);

                return null;
            }

            return file;
        }
        catch (Exception e) {
            Log.e("Optimen: ", e.getMessage());
        }

        return null;
    }
}