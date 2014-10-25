package com.example.yura.optimen;

/**
 * Created by yura on 25.10.14.
 */
public class dir_data
{
    public static final Character DIR = 'D';
    public static final Character FLD = 'F';

    public dir_data(){
        type = DIR;
        name="";
    }

    public dir_data(Character t, String nm){
        type = t;
        name = nm;
    }

    public void setType(Character t){
        type = t;
    }

    public Character getType(){
        return  type;
    }

    public void setName(String nm){
        name = nm;
    }

    public  String getName(){
        return  name;
    }

    private Character type;
    private String name;
}
