package controllers;

import java.sql.*;
import java.text.*;
import java.io.*;
import java.nio.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.Date;

import javax.inject.Inject;
import javax.swing.*;
import javax.swing.text.html.parser.AttributeList;

import com.fasterxml.jackson.databind.JsonNode;
import com.mysql.fabric.xmlrpc.base.Array;

import play.mvc.*;
import play.twirl.api.Content;
import views.html.*;
import static play.libs.Json.toJson;
import play.api.libs.json.Json;
import play.db.*;

public class DataBaseController extends Controller {
    @Inject Database db;
    // ...
    //Define table with registers of all Blobs
    public String table = "history"; 
    public int flag_movement = 0;
    
    //Filter by movement
    public String filter = " id_blob in (select id_blob from "+table+"_movement where xmov>80 or ymov>80)";

    public int getrows (ResultSet rs){
	    int tot;
	    try{
		    rs.last();
		    tot=rs.getRow();
		    rs.beforeFirst();
	    }catch (Exception e){
		    return 0;
	    }//try
	    return tot;
    }//get rows

    public int maxblnum () throws SQLException{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select max(id_blob) from "+table).executeQuery();
	    return rs.getInt("max(id_blob)"); 
    }//max blob id

    public Result query() throws Exception {
	    if (flag_movement == 0){
		    //Connection connection = db.getConnection();
		    //connection.createStatement().execute("drop table if exists "+table+"_movement");
		    //connection.createStatement().execute("create table "+table+"_movement as select id_blob,max(x) as xmax,min(x) as xmin,max(y) as ymax,min(y) as ymin ,min(time) as tmin,max(time) as tmax,max(x)-min(x) as xmov,max(y)-min(y) as ymov from "+table+" group by id_blob");
		    //connection.createStatement().execute("create table "+table+"_heat as select round(x/20)*2,round(y/20)*2, count(x) from "+table+" where"+filter+"group by round(x / 20)*2,round(y/20)*2");
		    //connection.createStatement().execute("create table "+table+"_day as select count(id_blob)as samples,concat(extract(day from time), \"-\" ,extract(month from time)) as day from "+table+" where "+filter+" group by day");
		    //connection.createStatement().execute("create table "+table+"_hour as select count(id_blob)as samples,extract(hour from time) as hour from "+table+" where "+filter+" group by hour");
		    flag_movement=9;
	    }
	    return ok(bd.render("Human tracking"));
    }//Page title
    
    public Result query_ids() throws Exception{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select id_blob from "+table+"_movement where "+filter).executeQuery();
	    int ids[]= new int[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    int reg= 0;
		    try{
			    reg= rs.getInt("id_blob");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    ids[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(ids));
    }//query_hour

    public Result query_id(long id) throws Exception {
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select * from "+table+" where id_blob=" + id).executeQuery();
	    int i=0;
	    blob[] blo= new blob[getrows(rs)];
	    while (rs.next()) {
		    blob bl = new blob();
		    try{
			    bl.id= rs.getInt("id_blob");
			    bl.x= rs.getInt("x");
			    bl.y = rs.getInt("y");
			    bl.time=rs.getString("time");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    blo[i]=bl;
		    i++;
	    }//while
	    return ok(toJson(blo));
    }//query by id
    
    public Result query_time(String date1, String date2) throws Exception {
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select * from "+table+" where time between \""+ date1 +"\" and \""+ date2+"\"").executeQuery();
	    int i=0;
	    blob[] blo= new blob[getrows(rs)];
	    while (rs.next()) {
		    blob bl = new blob();
		    try{
			    bl.id= rs.getInt("id_blob");
			    bl.x= rs.getInt("x");
			    bl.y = rs.getInt("y");
			    bl.time=rs.getString("time");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    blo[i]=bl;
		    i++;
	    }//while
	    return ok(toJson(blo));
    }//query time
    
    public Result query_maxmintime() throws Exception {
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select  id_blob,tmax, tmin,xmin,xmax,ymin,ymax from "+table+"_movement where xmov>80 or ymov>80").executeQuery();
	    register[] regs= new register[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    register reg= new register();
		    try{
			    reg.id= rs.getInt("id_blob");
			    reg.mintime= rs.getString("tmin");
			    reg.maxtime = rs.getString("tmax");
			    reg.maxy= rs.getInt("ymax");
			    reg.maxx= rs.getInt("xmax");
			    reg.miny= rs.getInt("ymin");
			    reg.minx= rs.getInt("xmin");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    regs[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(regs));
    }//query max min
    
    public class register extends Controller{
	    public int minx, maxx, miny,maxy;   //Maximum and minimum positions of the blob
	    public String mintime, maxtime; // Maximum and min timestamp
	    public int id; //id of the blob
    }//register
    
    public class register_h extends Controller{
	    public int x,y,count;
    }//register_h
    
    public class register_t extends Controller{
	    public int value;
	    public String date;
    }//register_h
    
    public Result query_heat() throws Exception{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select * from "+table+"_heat").executeQuery();
	    register_h[] regs= new register_h[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    register_h reg= new register_h();
		    try{
			    reg.x= rs.getInt("round(x/20)*2");
			    reg.y= rs.getInt("round(y/20)*2");
			    reg.count= rs.getInt("count(x)");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    regs[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(regs));
    }//query_heat
    
    public Result query_perday() throws Exception{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select * from "+table+"_day").executeQuery();
	    register_t[] regs= new register_t[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    register_t reg= new register_t();
		    try{
			    reg.value= rs.getInt("samples");
			    reg.date= rs.getString("day");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    regs[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(regs));
    }//query_perday samples
    
    public Result query_pepday() throws Exception{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select count(id_blob) as people, concat(extract(day from tmin), '-' ,extract(month from tmin)) as day from "+table+"_movement where "+filter+" group by day").executeQuery();
	    register_t[] regs= new register_t[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    register_t reg= new register_t();
		    try{
			    reg.value= rs.getInt("people");
			    reg.date= rs.getString("day");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    regs[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(regs));
    }//query_perday people
    
    public Result query_perhour() throws Exception{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select * from "+table+"_hour").executeQuery();
	    register_t[] regs= new register_t[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    register_t reg= new register_t();
		    try{
			    reg.value= rs.getInt("samples");
			    reg.date= rs.getString("hour");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    regs[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(regs));
    }//query_hour Samples
    public Result query_pephour() throws Exception{
	    Connection connection = db.getConnection();
	    ResultSet rs = connection.prepareStatement("select count(id_blob) as people, extract(hour from tmin) as hour from "+table+"_movement where "+filter+" group by hour").executeQuery();
	    register_t[] regs= new register_t[getrows(rs)];
	    int i=0;
	    while (rs.next()) {
		    register_t reg= new register_t();
		    try{
			    reg.value= rs.getInt("people");
			    reg.date= rs.getString("hour");
		    } catch (Exception e){
			    return badRequest(e.toString());
		    }//try
		    regs[i]= reg;
		    i++;
	    }//while
	    return ok(toJson(regs));
    }//query_hour Samples People
}//DataBase Controller
