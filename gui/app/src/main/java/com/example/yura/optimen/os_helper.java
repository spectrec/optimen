package com.example.yura.optimen;

import android.util.Log;
import java.io.File;

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
}