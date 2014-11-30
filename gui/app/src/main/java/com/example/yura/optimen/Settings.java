package com.example.yura.optimen;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.EditText;

/**
 * Created by yura on 30.11.14.
 */
public class Settings extends Activity{
    config_reader config;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings);

        String ip = getIntent().getExtras().getString("ip");
        Integer port = getIntent().getExtras().getInt("port");

        EditText ip_edit = (EditText)findViewById(R.id.editIp);
        ip_edit.setText(ip);

        EditText port_edit = (EditText)findViewById(R.id.editPort);
        port_edit.setText(port.toString());

        config = (config_reader)getIntent().getSerializableExtra("config");
    }

    public void onSettingsClick(View view) {
        EditText ip_edit = (EditText)findViewById(R.id.editIp);
        String new_ip = ip_edit.getText().toString();

        EditText port_edit = (EditText)findViewById(R.id.editPort);
        Integer new_port = Integer.parseInt(port_edit.getText().toString());

        config.set_ip(new_ip);
        config.set_port(new_port);
        config.write_config_to_file();

        finish();
    }
}
