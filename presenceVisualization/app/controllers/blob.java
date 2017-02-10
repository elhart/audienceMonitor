package controllers;

import javax.inject.*;
import play.*;
import play.mvc.EssentialFilter;
import play.http.HttpFilters;
import play.mvc.*;


public class blob extends Controller{
        public int x, y;   	//position of the blob
        public String time; //reference to timestamp and date
        public int id; 		//id of the blob
    }
