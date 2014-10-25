package com.example.yura.optimen;

import android.app.ListActivity;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Toast;
import android.app.AlertDialog;


public class Optimen extends ListActivity {

    //==============================================================================================
    optimen_list optimen_lst;
    //==============================================================================================

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.optimen);
        // get testing data
        optimen_lst = get_test_data();
        // click by item
        OnItemClickListener itemClickListener = new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                processItemClick(adapterView, view, i, l);
            }
        };
        // create adapter for listview
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this,
                android.R.layout.simple_list_item_1,
                optimen_lst.to_string()
        );
        // set adapter for listview
        setListAdapter(adapter);

        getListView().setOnItemClickListener(itemClickListener);
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
    public optimen_list get_test_data(){
        optimen_list lst = new optimen_list();

        lst.add_element('D',"books");
        lst.add_element('D',"films");
        lst.add_element('D',"study");
        lst.add_element('F',"main.cpp");
        lst.add_element('F',"hello.cpp");
        lst.add_element('F',"goodbye.cpp");
        lst.add_element('F',"test.cpp");
        lst.add_element('F',"log.cpp");
        lst.add_element('F',"process.cpp");
        lst.add_element('F',"entity.cpp");
        lst.add_element('F',"config.cpp");
        lst.add_element('F',"hello.h");
        lst.add_element('F',"goodbye.h");
        lst.add_element('F',"test.h");
        lst.add_element('F',"log.h");
        lst.add_element('F',"process.h");
        lst.add_element('F',"entity.h");
        lst.add_element('F',"config.h");

        return lst;
    }

    private void processItemClick(AdapterView<?> adapterView, View view, int i, long l){
        final dir_data tmp = optimen_lst.get_element(i);
        // if element it is directory
        if (tmp.getType() == dir_data.DIR){
            Toast.makeText(getApplicationContext(),
                    "Папка - " + tmp.getName(),
                    Toast.LENGTH_SHORT).show();
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
                    start_loading(tmp.getName());
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

    private void start_loading(String filename){
        Toast.makeText(getApplicationContext(),
                "Приступаю к скачиванию файла " + filename,
                Toast.LENGTH_SHORT).show();
    }
}
