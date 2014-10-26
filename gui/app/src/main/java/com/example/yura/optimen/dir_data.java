package com.example.yura.optimen;

/**
 * Created by yura on 25.10.14.
 */
public class dir_data implements Comparable<dir_data>
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

    public int compareTo(dir_data cmp){
        if (type == DIR && cmp.getType() == FLD)
            return -1;
        if (type == FLD && cmp.getType() == DIR)
            return  1;
        return name.compareTo(cmp.getName());
    }

    private Character type;
    private String name;
}
