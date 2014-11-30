package com.example.yura.optimen;

import android.app.Activity;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Toast;
import android.app.AlertDialog;
import android.content.Intent;

import java.io.File;
import java.net.Socket;


public class Optimen extends Activity {
    //==============================================================================================
    String optimen_directory = "/sdcard/Optimen/";
    optimen_list optimen_lst;
    config_reader config;
    String current_path;
    ListView listView;
    //==============================================================================================

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.optimen);

        config = new config_reader(optimen_directory + "optimen_gui.conf");

        // create Optimen directory
        if (os_helper.create_directory(optimen_directory) == 1)
            config.write_config_to_file();

        // read configuration
        config.read_config();
        config.print_config_data();

        current_path = new String("/");

        // click by item
        OnItemClickListener itemClickListener = new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                processItemClick(adapterView, view, i, l);
            }
        };

        listView = (ListView) findViewById(R.id.listview);
        listView.setOnItemClickListener(itemClickListener);

        try {
            optimen_lst = command_processor.process_command_ls(config.get_port(), config.get_ip(), current_path);
        }
        catch (Exception e) {
            Toast.makeText(getApplicationContext(), "Ошибка при получении списка файлов (проверьте соединение)", Toast.LENGTH_SHORT).show();
            return;
        }

        update_list_view(optimen_lst);

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.optimen, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            Intent settings = new Intent(Optimen.this, Settings.class);
            settings.putExtra("ip", config.get_ip());
            settings.putExtra("port", config.get_port());
            settings.putExtra("config", config);
            startActivity(settings);
            return true;
        }
        else if (id == R.id.action_update) {
            try {
                config.read_config();
                optimen_lst = command_processor.process_command_ls(config.get_port(), config.get_ip(), current_path);
                update_list_view(optimen_lst);
            }
            catch (Exception e) {
                Toast.makeText(getApplicationContext(), "Ошибка при получении списка файлов (проверьте соединение)", Toast.LENGTH_SHORT).show();
                return false;
            }
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    //==============================================================================================
    public SimpleAdapter getAdapterForView(optimen_list list){
        String[] from = {"icon", "name"};
        int[] to = {R.id.icon, R.id.name};

        return  new SimpleAdapter(getBaseContext(),
                optimen_lst.get_data_for_view(),
                R.layout.listview_layout, from, to);
    }

    private void processItemClick(AdapterView<?> adapterView, View view, int i, long l){
        final dir_data tmp = optimen_lst.get_element(i);
        // if element it is directory
        if (tmp.getType() == dir_data.DIR)
        {
            process_dir(tmp);
        }
        // if element it is file
        else if (tmp.getType() == dir_data.FLD)
        {
            AlertDialog.Builder builder = new AlertDialog.Builder(Optimen.this);
            builder.setTitle("Информация");
            builder.setMessage("Скачать файл " + tmp.getName());
            builder.setIcon(R.drawable.question_icon);
            builder.setCancelable(false);
            builder.setPositiveButton("Да", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialogInterface, int i) {
                    try {
                        process_file(tmp.getName());
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
            builder.setNegativeButton("Нет", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialogInterface, int i) {
                }
            });
            builder.show();
        }
    }

    private void process_dir(dir_data data){
        String tmp_current_path = current_path;

        try {
            if (data.getName().compareTo(".") == 0)
                return;
            // back
            if (data.getName().compareTo("..") == 0) {
                int pos = -1;
                for (int i = tmp_current_path.length() - 2; i >= 0 && pos == -1; --i) {
                    if (tmp_current_path.charAt(i) == '/')
                        pos = i;
                }
                if (pos != -1)
                    tmp_current_path = tmp_current_path.substring(0, pos + 1);
            }
            // forward
            else {
                tmp_current_path += data.getName() + "/";
            }

            Log.i("Optimen", tmp_current_path);

            optimen_lst = command_processor.process_command_ls(config.get_port(), config.get_ip(), tmp_current_path);
            update_list_view(optimen_lst);
        }
        catch (Exception e) {
            Toast.makeText(getApplicationContext(), "Ошибка при переходе (проверьте соедниение)", Toast.LENGTH_SHORT).show();
            return;
        }

        current_path = tmp_current_path;
    }

    private void process_file(String filename) throws Exception {
        try {
            Socket socket = command_processor.process_command_file_open(config.get_port(),
                                                                        config.get_ip(), filename);
            if (socket == null)
                throw new Exception("Downloading file failed");

            // save file into optimen
            Integer basename_pos = filename.lastIndexOf('/');
            if (basename_pos == -1)
                basename_pos = 0;
            String local_file_name = optimen_directory + filename.substring(basename_pos);

            File file = os_helper.open_file(local_file_name);
            if (file == null)
                throw new Exception("Can't create file: " + local_file_name);

            Boolean eof = false;
            Integer already_read = 0;
            while (!eof)
            {
                already_read = command_processor.process_command_file_read(socket, file, already_read);
                eof = already_read == 0;

                if (already_read == -1)
                   throw new Exception("Can't recv file part (server error)");
            }

            if (!command_processor.process_command_file_close(socket))
                throw new Exception("Closing file on server side failed");

            Toast.makeText(getApplicationContext(), "File downloaded", Toast.LENGTH_SHORT).show();
        }
        catch (Exception e) {
            Toast.makeText(getApplicationContext(), "Ошибка при скачивании:\n" + e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

    private void update_list_view(optimen_list lst){
        SimpleAdapter adapter = getAdapterForView(lst);
        listView.setAdapter(adapter);
    }
}
