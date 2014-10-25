package com.example.yura.optimen;

/**
 * Created by yura on 25.10.14.
 */
import java.util.Iterator;
import java.util.Vector;

public class optimen_list
{
    public optimen_list(){
        lst = new Vector<dir_data>();
        lst.clear();
    }

    public optimen_list(Vector<dir_data> from){
        lst = from;
    }

    public void add_element(dir_data element){
        lst.add(element);
    }

    public  void add_element(Character t, String nm){
        dir_data tmp = new dir_data(t, nm);
        lst.add(tmp);
    }

    public dir_data get_element(Integer position){
        if (position < 0 || position >= lst.size())
            return new dir_data('-',"");

        return lst.elementAt(position);
    }

    public String[] to_string(){
        String[] res = new String[lst.size()];

        Integer counter = 0;
        Iterator<dir_data> iter = lst.iterator();
        while(iter.hasNext()){
            res[counter++] = iter.next().getName();
        }

        return res;
    }

    private Vector<dir_data> lst;
}
