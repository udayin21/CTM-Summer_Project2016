/*******
**@displayboard
**@author:saurabh
********/ 
#include <iostream>
#include <pqxx/pqxx>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/fl_draw.H>
#include <string>
#include <queue>   
#include <deque>   
#include <map>
#include <unistd.h>
#include <FL/Fl_Box.H>
using namespace std;
using namespace pqxx;

#define MAX_ROWS 20
#define MAX_COLS 3		// A-Z

// Derive a class from Fl_Table
class MyTable : public Fl_Table {

  string data[MAX_ROWS][MAX_COLS];		// data array for cells
  queue<string> feed;
  

  // Draw the row/col headings
  //    Make this a dark thin upbox with the text inside.
  //
  void DrawHeader(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
      fl_draw_box(FL_THIN_UP_BOX, X,Y,W,H, row_header_color());
      fl_color(FL_BLACK);
      fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
    fl_pop_clip();
  } 
  // Draw the cell data
  //    Dark gray text on white background with subtle border
  //
  void DrawData(const char *s, int X, int Y, int W, int H) {
    fl_push_clip(X,Y,W,H);
      // Draw cell bg
      fl_color(FL_WHITE); fl_rectf(X,Y,W,H);
      // Draw cell data
      fl_color(FL_GRAY0); fl_draw(s, X,Y,W,H, FL_ALIGN_CENTER);
      // Draw box border
      fl_color(color()); fl_rect(X,Y,W,H);
    fl_pop_clip();
  } 
  // Handle drawing table's cells
  //     Fl_Table calls this function to draw each visible cell in the table.
  //     It's up to us to use FLTK's drawing functions to draw the cells the way we want.
  //
  
  static void Timer_CB(void *data){
	MyTable *o = (MyTable*)data;
        o->redraw();
        Fl::add_timeout(0.60,Timer_CB,data);
}

public:
  // Constructor
  //     Make our data array, and initialize the table options.
  //
  MyTable(int X, int Y, int W, int H, const char *L=0) : Fl_Table(X,Y,W,H,L) {
    // Fill data array
     
    for ( int r=0; r<MAX_ROWS; r++ )
     for ( int c=0; c<MAX_COLS; c++ )
       data[r][c] = "    ";
    // Rows
    //this->upCells();
    rows(MAX_ROWS);             // how many rows
    row_header(1);              // enable row headers (along left)
    row_height_all(40);         // default height of rows
    row_resize(0);              // disable row resizing
    // Cols
    cols(MAX_COLS);             // how many columns
    col_header(1);              // enable column headers (along top)
    col_width_all(250);          // default width of columns
    col_resize(1);              // enable column resizing
    Fl::add_timeout(0.60,Timer_CB, (void*) this);
    end();			// end the Fl_Table group
  }
  ~MyTable() { }
   
  void draw_cell(TableContext context, int ROW=0, int COL=0, int X=0, int Y=0, int W=0, int H=0) {
    static char s[100];
    try{
      connection C("dbname=lprs_db user=lprs password=lprs \
      hostaddr=127.0.0.1 port=5432");
      if (C.is_open()) {
         //cout << "Opened database successfully: " << C.dbname() << endl;
      } else {
         //cout << "Can't open database" << endl;
         exit(0);
      }	
      string sql ="";
      sql = "SELECT * FROM lprs_db.smart_events WHERE (speed_violation = 1 OR wwd_violation=1 OR lane_violation = 1 OR park_violation = 1 ) ORDER BY event_time DESC LIMIT 20;";
      //sql = "SELECT * FROM lprs_db.smart_events WHERE resolved=0 AND (speed_violation = 1 OR wwd_violation=1 OR lane_violation = 1 OR park_violation = 1 ) ORDER BY event_time DESC LIMIT 20;";
      nontransaction N(C);

      /*execute the query and fetch result */
      result R(N.exec(sql));
      /* using queue to store id of violations to be marked as resolved later */
      queue<string> str_id;
      //cout<<"this here"<<endl;
      for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
        str_id.push(c[0].as<string>());
        string msg1="     ";
        msg1+=c[1].as<string>();
        msg1+="    ";
        if((c[3].as<string>()).compare("1") == 0 ){
          msg1+= "Wrong Way Driving "; 
        } 
        if((c[4].as<string>()).compare("1") == 0 ){
          msg1+= "Speed : ";
	  msg1= msg1+" "+ c[6].as<string>().substr(0,2)+" Kmph"; 	 
        }
        if((c[8].as<string>()).compare("1") == 0 ){
          msg1+= "lane Violation ";  
        }
        if((c[9].as<string>()).compare("1") == 0 ){
          msg1+="park Violation ";  
        }
        msg1+="     Time : ";
	msg1+=c[2].as<string>().substr(11,8);
	feed.push(msg1);
      }
      while(feed.size()>20){
        feed.pop();
      }
      int cnt1=feed.size();
      int index=0;
      queue<string> temp;
      string str="";
      while(!feed.empty()){
                cnt1--;
 	        str=feed.front();
                feed.pop();
/*		
	        data[cnt1][0] = str.substr(0,15);
	        data[cnt1][1] = str.substr(15,22);
		data[cnt1][2] = str.substr(38);
                cout<<str<<endl;
*/
	        data[index][0] = str.substr(0,15);
	        data[index][1] = str.substr(15,22);
		data[index][2] = str.substr(37);
		index++;
		temp.push(str);     
      }

      while(!temp.empty()){
	  str=temp.front();
          temp.pop();
          feed.push(str);
      }

       N.abort();
      while (0 && !str_id.empty()) {
        sql = "UPDATE lprs_db.smart_events SET resolved2 = 1 WHERE transid = '"+str_id.front() +"';";
        str_id.pop();
        work W(C);
        W.exec(sql);
        W.commit();
      }
      //this->redraw();


      }catch (const std::exception &e){
      cerr << e.what() << std::endl;
    	}
    switch ( context ) {
      case CONTEXT_STARTPAGE:                   // before page is drawn..
        fl_font(FL_HELVETICA, 18);              // set the font for our drawing operations
        return; 
      case CONTEXT_COL_HEADER:                  // Draw column headers
        sprintf(s,"%c",'A'+COL);                // "A", "B", "C", etc.
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_ROW_HEADER:                  // Draw row headers
        sprintf(s,"%03d:",ROW);                 // "001:", "002:", etc
        DrawHeader(s,X,Y,W,H);
        return; 
      case CONTEXT_CELL:                        // Draw data in cells
	char *s2;
	s2=new char[data[ROW][COL].length() + 1];
	memcpy(s2,data[ROW][COL].c_str(),data[ROW][COL].length() + 1);
        sprintf(s,"%s",s2);
        DrawData(s,X,Y,W,H);
        return;
      default:
        return;
    }
  } 
};

/*
void updateCells(Fl_Widget* w,void *){
    table.upCells();
    
  }
*/


int main(int argc, char **argv) {
  Fl_Double_Window win(800, 1000, "COP315 Display Board");
  Fl_Box box(0,0,800,150);
  Fl_JPEG_Image jpg("./HeaderDB.jpg");
  box.image(jpg);
  MyTable table(0,150,790,900);
  //win.fullscreen();  
  //win.callback(updateCells);
   //updateCells();
   //table.upCells();
   //table.damage(1);
   win.end();
   win.resizable(table);
   win.show(argc,argv);
   //table.redraw();
   //win.hide();
   //Fl::flush();
  //
  return(Fl::run());
}

//
// End of "$Id: table-simple.cxx 9086 2011-09-29 21:10:59Z greg.ercolano $".
//
