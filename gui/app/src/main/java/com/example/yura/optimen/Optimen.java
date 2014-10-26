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


public class Optimen extends Activity {

    //==============================================================================================
    optimen_list optimen_lst;
    config_reader config;
    String current_path;
    ListView listView;
    //==============================================================================================

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.optimen);

        config = new config_reader();

        // create Optimen directory
        if (os_helper.create_directory("/sdcard/Optimen") == 1)
            config.write_config_to_file("/sdcard/Optimen/optimen_gui.conf");

        // read configuration
        config.read_config("/sdcard/Optimen/optimen_gui.conf");
        config.print_config_data();

        current_path = new String("/");

        // get testing data in '/'
        optimen_lst = command_processor.process_command_ls(config.get_port(), config.get_ip(), current_path);

        // click by item
        OnItemClickListener itemClickListener = new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                processItemClick(adapterView, view, i, l);
            }
        };

        listView = (ListView)findViewById(R.id.listview);
        listView.setOnItemClickListener(itemClickListener);
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
        if (tmp.getType() == dir_data.DIR){
            process_dir(tmp);
        }
        // if element it is file
        else if (tmp.getType() == dir_data.FLD){
            AlertDialog.Builder builder = new AlertDialog.Builder(Optimen.this);
            builder.setTitle("Информация");
            builder.setMessage("Скачать файл " + tmp.getName());
            builder.setIcon(R.drawable.question_icon);
            builder.setCancelable(false);
            builder.setPositiveButton("Да", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialogInterface, int i) {
                    process_file(tmp.getName());
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
        if (data.getName().compareTo(".") == 0)
            return;
        // back
        if (data.getName().compareTo("..") == 0){
            int pos = -1;
            for (int i = current_path.length()-2; i >= 0 && pos == -1; --i){
                if (current_path.charAt(i) == '/')
                    pos = i;
            }
            if (pos != -1)
                current_path = current_path.substring(0, pos+1);
        }
        // forward
        else {
            current_path += data.getName() + "/";
        }

        Log.i("Optimen", current_path);

        optimen_lst = command_processor.process_command_ls(config.get_port(), config.get_ip(), current_path);
        update_list_view(optimen_lst);
    }

    private void process_file(String filename){
        Toast.makeText(getApplicationContext(),
                "Файл - " + filename,
                Toast.LENGTH_SHORT).show();
    }

    private void update_list_view(optimen_list lst){
        SimpleAdapter adapter = getAdapterForView(lst);
        listView.setAdapter(adapter);
    }
}
