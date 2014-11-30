package com.example.yura.optimen;

/**
 * Created by yura on 25.10.14.
 */
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;
import java.util.List;
import java.util.ArrayList;

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

    public List<HashMap<String, String>> get_data_for_view(){
        sort_by_file_type();

        int[] icons = new int[] {R.drawable.folder_icon, R.drawable.file_icon};

        List<HashMap<String,String>> result = new ArrayList<HashMap<String,String>>();
        Iterator<dir_data> iterator = lst.iterator();

        while (iterator.hasNext()){
            dir_data tmp_dir_data = iterator.next();
            Integer index = 0; // dir by default
            if (tmp_dir_data.getType() == dir_data.FLD)
                index = 1;

            HashMap<String, String> tmp_mp = new HashMap<String, String>();
            tmp_mp.put("icon", Integer.toString(icons[index]));
            tmp_mp.put("name", tmp_dir_data.getName());

            result.add(tmp_mp);
        }

        return  result;
    }

    private void sort_by_file_type(){
        Collections.sort(lst);
    }

    private Vector<dir_data> lst;
}
