//  Audience Monitor: Presence detection module
//  main.cpp (a single .cpp file)
//  
//
//  Created by mora and elhart
//  Updated: 02/02/17
//  Version: 1.0
//

//openCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio/videoio.hpp>

//OpenGL, GLEW, GLFW includes
#include <GLFW/glfw3.h>

//C++ Libraries
#include <algorithm>
#include <ctime>
#include <mysql.h>
#include <time.h>
#include <iostream>
#include <math.h>
#include <stdio.h>

using namespace cv;
using namespace std;

bool fileReading = true;
//string filePath = "/Volumes/projects/ResearchProjects/audienceMonitoring/videodata/openNIdataset1/";
string filePath = "/Users/Cristian/Documents/MasterThesis/";
string fileName = filePath + "20160405-1126001.oni";

const int WINDOWS_WIDTH = 1280;
const int WINDOWS_HEIGHT = 720;

bool createdatabase = true;

typedef struct{
    float area;     //size of the blob
    int x, y;       //position of the blob
    string time;    //reference to timestamp and date
    int id;         //unique id of the blob
} Blob;

typedef struct{
    GLfloat x, y, z;    //position
    GLfloat r, g, b, a; //color and alpha channels
} Vertex;

typedef struct{
    GLfloat x, y, z;
} Data;

GLfloat alpha=30.0f, beta=-70.0f, zoom=-2.0f;

GLboolean locked = GL_FALSE;
GLboolean freeze = GL_FALSE;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    const float fovY = 45.0f;
    const float front = 0.1f;
    const float back = 128.0f;
    float ratio = 1.0f;
    if (height > 0)
        ratio = (float) width / (float) height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(fovY, ratio, front, back);
    
    const double DEG2RAD = M_PI / 180;
    // tangent of half fovY
    double tangent = tan(fovY/2 * DEG2RAD);
    // half height of near plane
    double height_f = front * tangent;
    // half width of near plane
    double width_f = height_f * ratio;
    //Create the projection matrix based on the near clipping
    //plane and the location of the corners
    glFrustum(-width_f, width_f, -height_f, height_f, front, back);
    
}//framebuffer_size_callback

void drawOrigin(){
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    float transparency = 0.5f;
    //draw a red line for the x-axis
    glColor4f(1.0f, 0.0f, 0.0f, transparency);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glColor4f(1.0f, 0.0f, 0.0f, transparency);
    glVertex3f(0.5f, 0.5f, 0.5f);
    //draw a green line for the y-axis
    glColor4f(0.0f, 1.0f, 0.0f, transparency);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glColor4f(0.0f, 1.0f, 0.0f, transparency);
    glVertex3f(0.5f, -0.5f, 0.5f);
    //draw a blue line for the z-axis
    glColor4f(0.0f, 0.0f, 1.0f, transparency);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glColor4f(0.0f, 0.0f, 1.0f, transparency);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glEnd();
}//drawOrigin


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action != GLFW_PRESS)
        return;
    switch (key){
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_SPACE:
            freeze=!freeze;
            break;
        case GLFW_KEY_LEFT:
            alpha += 5.0f;
            break;
        case GLFW_KEY_RIGHT:
            alpha -= 5.0f;
            break;
        case GLFW_KEY_UP:
            beta -= 5.0f;
            break;
        case GLFW_KEY_DOWN:
            beta += 5.0f;
            break;
        case GLFW_KEY_COMMA:
            zoom -= 0.25f;
            //if (zoom < 0.0f)
            //    zoom = 0.0f;
            break;
        case GLFW_KEY_PERIOD:
            zoom += 0.25f;
            break;
        default:
            break;
    }//switch
}//key_callback

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if(button != GLFW_MOUSE_BUTTON_LEFT)
        return;
    if(action == GLFW_PRESS){glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        locked = GL_TRUE;
    }else{
        locked = GL_FALSE;
        glfwSetInputMode(window, GLFW_CURSOR,GLFW_CURSOR_NORMAL);
    }//if
}//mouse_button_callback

int cursorX,cursorY;

void cursor_position_callback(GLFWwindow* window, double x, double y){
    //if the mouse button is pressed
    if (locked){
        alpha += (GLfloat) (x - cursorX) / 10.0f;
        beta += (GLfloat) (y - cursorY) / 10.0f;
    }//if
    //update the cursor position
    cursorX = (int) x;
    cursorY = (int) y;
}//cursor_position_callback

void scroll_callback(GLFWwindow* window, double x, double y){
    zoom += (float) y / 4.0f;
    //if (zoom < 0.0f)
    //    zoom = 0.0f;
}//scroll_callback

void finish_with_error(MYSQL *con)
{
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
}//Error in database

string write_db (int id_blob, int x, int y, string time){
    string query;
    if (time==""){
        return "explain history";
    }else{
        time.erase(std::remove(time.begin(), time.end(), '\n'), time.end());
        query= "INSERT INTO history ( id_blob, x, y, time )   VALUES ("+std::to_string(id_blob)+","+std::to_string(x)+","+std::to_string(y)+",\""+time+"\");";
        return query;
    }//if
}//function

//Calculates radio of Blob
int radio(int area){
    if (sqrt(area/M_PI)<55)
        return  55;
    else
        return sqrt(area/M_PI);
}

void cleanBlobArray(Blob (&a)[20]){
    for(Blob p1:a){
        p1.area=0;
        p1.x=0;
        p1.y=0;
        p1.time=" ";
        p1.id=0;
    }
}

//Query database
void queryall(string query, MYSQL connection){
    if (mysql_query(&connection, query.c_str())){
        finish_with_error(&connection);
    }//if
    MYSQL_RES *result = mysql_store_result(&connection);
    if (result == NULL){
        finish_with_error(&connection);
    }//if
    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))){
        for(int i = 0; i < num_fields; i++){
            printf("%s ", row[i] ? row[i] : "NULL");
        }//for
        printf("\n");
    }//while
}//function

int main(int ac, char** av){
    
    Blob prev[20],prev1[20];
    int prevBlob=0;
    int a=0,g_id_max=0, frame=1;
    
    //initialise prev
    cleanBlobArray(prev);
    cleanBlobArray(prev1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    //Time definition
    struct tm * timeinfo;
    time_t rawtime;
    
    //Connect to Database
    //--------
    std::vector<std::string> tables;
    //MYSQL_RES *result;
    //MYSQL_ROW row;
    MYSQL *connection, mysql;

    mysql_init(&mysql);
    connection=mysql_real_connect(&mysql, "localhost", "root", "masterthesis", "APDA", 0, 0, 0);
    printf("MySQL client version: %s\n", mysql_get_client_info());
    if (createdatabase== true){
        if (mysql_query(connection, "DROP TABLE IF EXISTS history;"))
        {
            finish_with_error(connection);
        }//if
        if (mysql_query(connection, "CREATE TABLE history( id int unsigned NOT NULL auto_increment, id_blob int unsigned NOT NULL, x int unsigned NOT NULL, y int unsigned NOT NULL, time varchar(19) NOT NULL, PRIMARY KEY (id));"))
        {
            finish_with_error(connection);
        }//if
    }//if
    
    if (connection == NULL)
    {
        std::cout << mysql_error(&mysql) << std::endl;
    }
    

    
    //create an empty capture object
    VideoCapture capture;
    
    //GLFW window object
    GLFWwindow* window;
    int width, height;
    
    if(!glfwInit()){
        cout<< "Failed to initialize GLFW." <<endl;
        exit( EXIT_FAILURE );
    }//if
    
    //glfwWindowHint(GLFW_DEPTH_BITS, 16);
    
    window = glfwCreateWindow( 640, 480, "3D View of the Scene", NULL, NULL );
    if (!window){
        cout<< "Faild to open a GLFW window." <<endl;
        glfwTerminate();
        exit( EXIT_FAILURE );
    }//if
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);
    
    // Set callback functions
    //keyboard input callback
    glfwSetKeyCallback(window, key_callback);
    //framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //mouse button callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    //mouse movement callback
    glfwSetCursorPosCallback(window, cursor_position_callback);
    //mouse scroll callback
    glfwSetScrollCallback(window, scroll_callback);
    
    //enable anti-aliasing
    glEnable(GL_BLEND);
    //smooth the points
    glEnable(GL_LINE_SMOOTH);
    //smooth the lines
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    //needed for alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST) ;
    
    //open the capture device
    if (!capture.isOpened()){
        if(fileReading){ //open the capture device, and pass the file name of the file that contains depth data
            cout<< "Opening the kinect file in "+fileName<<endl;
            capture.open(fileName);
            if (capture.isOpened()){
                cout<< "The file has been opened."<<endl;
                //it is not possible to set the output mode, it has been locked for changes
                //capture.set( CV_CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CV_CAP_OPENNI_VGA_30HZ );
            }else{
                cout<< "Error: the file cannot be opened."<<endl;
            }//else
        }else{//open the capture device, and indicate to get a depth stream from the kinect device
            cout<< "Openinng the capture device."<<endl;
            capture.open(CV_CAP_OPENNI);
            if (capture.isOpened()){
                cout<< "Device has been opened."<<endl;
            }else{
                cout<< "Error: device cannot be opened."<<endl;
            }//else
        }//else
    }else{
        cout<< "Device is already open."<<endl;
    }//else
    
    //create the map objects for raw kinect data
    Mat bgrImage;
    Mat depthMap;
    
    Mat depthMapReal;
    Mat depthMapMask;
    Mat depthMapBgRm;
    Mat depthMapBgRmConv;
    Mat depthMapRotation;
    Mat depthMapBloobs;
    
    //print out some info about the depth data
    cout << "\nDepth generator output mode:" << endl <<
            "FRAME_WIDTH      " << capture.get( CAP_PROP_FRAME_WIDTH ) << endl <<
            "FRAME_HEIGHT     " << capture.get( CAP_PROP_FRAME_HEIGHT ) << endl <<
            "FRAME_MAX_DEPTH  " << capture.get( CAP_PROP_OPENNI_FRAME_MAX_DEPTH ) << " mm" << endl <<
            "FPS              " << capture.get( CAP_PROP_FPS ) << endl <<
            "REGISTRATION     " << capture.get( CAP_PROP_OPENNI_REGISTRATION ) << endl<<endl;
    
    cout << "\nMore info:" << endl <<
            "Focal length     " << capture.get( CAP_PROP_OPENNI_FOCAL_LENGTH ) << endl <<
            "baseline         " << capture.get( CV_CAP_PROP_OPENNI_BASELINE ) << endl <<
            "FPS              " << capture.get( CAP_PROP_FPS ) << endl <<
            "pos_msec         " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CV_CAP_PROP_POS_MSEC ) << endl <<
            "pos_frames       " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CV_CAP_PROP_POS_FRAMES ) << endl << endl;
    
    cout << "\nImage generator output mode:" << endl <<
            "FRAME_WIDTH   " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CAP_PROP_FRAME_WIDTH ) << endl <<
            "FRAME_HEIGHT  " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CAP_PROP_FRAME_HEIGHT ) << endl <<
            "FPS           " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CAP_PROP_FPS ) << endl <<endl;
    
    /*  available data from the kinect sensor and thheir types
        CV_CAP_OPENNI_DEPTH_MAP - depth values in mm (CV_16UC1)
        CV_CAP_OPENNI_POINT_CLOUD_MAP - XYZ in meters (CV_32FC3)
        CV_CAP_OPENNI_DISPARITY_MAP - disparity in pixels (CV_8UC1)
        CV_CAP_OPENNI_DISPARITY_MAP_32F - disparity in pixels (CV_32FC1)
        CV_CAP_OPENNI_VALID_DEPTH_MASK - mask of valid pixels (not ocluded, not shaded etc.) (CV_8UC1)
     
        CV_CAP_OPENNI_BGR_IMAGE - color image (CV_8UC3)
        CV_CAP_OPENNI_GRAY_IMAGE - gray image (CV_8UC1)
    */
    //as long as there are frames in the capture device object
    for(;;){
        //grab one frame
        if(!capture.grab()){
            cout<< "Error: device cannot grab the data."<<endl;
            return -1;
        }//if
        
        //cout <<  "pos_msec         " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CV_CAP_PROP_POS_MSEC ) << endl <<
        //         "pos_frames       " << capture.get( CAP_OPENNI_IMAGE_GENERATOR+CV_CAP_PROP_POS_FRAMES ) << endl << endl;

        
        //BGR IMAGE
        //from the grbbed frame retrieve rgb image and show it
        if(capture.retrieve(bgrImage, CV_CAP_OPENNI_BGR_IMAGE)){
            imshow("RGB Image", bgrImage);
        }else{
            cout<< "Error: device cannot retrieve the rgb data."<<endl;
            return -1;
        }//
        
        //DEPTH MAP
        //from the grabbed frame retrieve the depth image and show it
        if(capture.retrieve(depthMap, CV_CAP_OPENNI_DEPTH_MAP)){ //depth map min-max 0-3900
            //const float scaleFactor = 0.05f;
            //depthMap.convertTo(depthMapConverted, CV_8UC1, scaleFactor); //0-183
            depthMap.convertTo(depthMapReal, CV_16UC1, 8); //0-31000
            imshow("DEPTH Map CV_16UC1", depthMap);
            imshow("DEPTH Map Real CV_16UC1", depthMapReal);
            
        }else{
            cout<< "Error: device cannot retrieve the depth data."<<endl;
            return -1;
        }//
        
        //DISPARITY_MAP
        if(capture.retrieve(depthMapMask, CV_CAP_OPENNI_DISPARITY_MAP)){ //depth mask min-max 0-255
            //imshow("DISPARITY_MAP", depthMapMask);
        }else{
            cout<< "Error: device cannot retrieve the depth data mask."<<endl;
            return -1;
        }
        
        //get mat measures
        //double min, max;
        //cv::minMaxLoc(depthMapMask, &min, &max);
        // cout << "min: " << min << endl;
        // cout << "max: " << max << endl;
        
        //REMOVE BACKGROUND
        //-----------------
        depthMap.copyTo(depthMapBgRm);
        //cout << "depthMapMask t cols: " << depthMapMaskTresholded.cols << endl;
        //cout << "depthMapMask t rows: " << depthMapMaskTresholded.rows << endl << endl;
        
        for(int y = 0; y < depthMap.rows; y++){
            //cout << endl << "y=" << y << endl << endl;
            for(int x = 0; x < depthMap.cols; x++){
              //  cout << "x=" << x << "-v:" << depthMap.at<ushort>(y,x) << " " << endl;
                if(depthMapBgRm.at<ushort>(y,x) > 3100 || depthMapBgRm.at<ushort>(y,x) < 1000) //remove distant and close objects
                    depthMapBgRm.at<ushort>(y,x) = 0;
                //if(depthMapBgRm.at<ushort>(y,x) > 0) // enhance data that has been left in the desired range
                //    depthMapBgRm.at<ushort>(y,x) = 65535;
                //remove the pole in top left corner mirror image
                if(x<160 && y<180)
                    depthMapBgRm.at<ushort>(y,x) = 0;
//                //remove the pole in top right corner image
//                if(x>480 && y<180)
//                    depthMapBgRm.at<ushort>(y,x) = 0;
                //remove the public display
                if(x<350 && x>300 && y>430)
                    depthMapBgRm.at<ushort>(y,x) = 0;
            }//for
        }//for y
        //depthMapBgRm.at<ushort>(160,450) = 1;
        //SHOW the sceen without background (it depends on the tresholds)
        //imshow("DEPTH Map without background", depthMapBgRm);
        
        const float scaleFactor = 0.05f;
        depthMapBgRm.convertTo(depthMapBgRmConv, CV_8UC1, scaleFactor); //0-255
        //imshow("DEPTH Map without background -> CV_8UC1", depthMapBgRmConv);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        
        //camera matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0, 0.0, zoom);
        // rotate by beta degrees around the x-axis
        glRotatef(beta, 1.0, 0.0, 0.0);
        // rotate by alpha degrees around the z-axis
        glRotatef(alpha, 0.0, 0.0, 1.0);
        
        drawOrigin();
        
        //depthMapReal.convertTo(depthMapRotation, CV_8UC1, scaleFactor); //0-255
        depthMapBgRmConv.copyTo(depthMapRotation);
        
        Vertex point;
        glPointSize(3.0f);
        glBegin(GL_POINTS);
        //convert the scee from opencv (mat) to opengl (vertex)
        for(int y = 0; y < depthMapRotation.rows; y++){
            //cout<<"y = "<<y<<endl;
            for(int x = 0; x < depthMapRotation.cols; x++){
                //cout<<"x = "<<x;
                point.x = (float)(x-depthMapRotation.cols/2)/depthMapRotation.cols;
                //cout<<"  X: "<<point.x;
                point.y = (float)(-1)*(y-depthMapRotation.rows/2)/depthMapRotation.rows;
                //cout<<"  Y: "<<point.y<<endl;
                point.z = 0+(float)depthMapRotation.at<ushort>(y, x, 0)/255/2;
                point.r = 1.0f;
                point.g = 1.0f;
                point.b = 1.0f;
                point.a = 0.5f;
                glColor4f(point.r, point.g, point.b, point.a);
                glVertex3f(point.x, point.y, point.z);
            }//for x
        }//for y
        glEnd();
        
        //mark the first point (-.5, -.5, 0.0)
        glPointSize(10.0f);
        glBegin(GL_POINTS);
            glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
            glVertex3f(-0.5f, -0.5f, 0.0f);
        glEnd();
        
        //mark the origin (0,0,0)
        glPointSize(10.0f);
        glBegin(GL_POINTS);
        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glEnd();
        
        //update the window
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        //BLOOB detection
        //--------
        depthMapBgRmConv.copyTo(depthMapBloobs);
        
        // Set up the detector with default parameters.
        cv::SimpleBlobDetector::Params params;
        params.minDistBetweenBlobs = 00.0f;
        params.filterByInertia = false;
        params.filterByConvexity = false;
        params.filterByColor = false;
        //params.filterByCircularity = false;
        params.filterByArea = true;
        params.minArea = 600;
        params.maxArea = 20000;
        // Filter by Circularity
        params.filterByCircularity = false;
        //params.minCircularity = 0.1;
        
        std::vector<KeyPoint> keypoints;
        cv::Ptr<cv::SimpleBlobDetector> detector = cv::SimpleBlobDetector::create(params);
        detector->detect(depthMapBloobs, keypoints );
        
        // Draw detected blobs as red circles.
        // DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures the size of the circle corresponds to the size of blob
        Mat im_with_keypoints;
        drawKeypoints(depthMapBloobs, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
        drawKeypoints(im_with_keypoints, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DEFAULT);
        
        cout<<"This is frame: " << frame++<<" with " << keypoints.size()<<" Blobs"<<endl;
       
        //cv::KeyPoint::KeyPoint(float x, float y,float _size, float _angle = -1,float _response = 0, int _octave = 0, int _class_id = -1);
        // Show blobs
        imshow("keypoints", im_with_keypoints );
        
        //Update to local time on format date time %m/%d/%y %H:%M:%S
        time_t rawtime;
        struct tm * timeinfo;
        char buffer [19];
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        strftime (buffer,19,"%F %T",timeinfo);
        
        std::vector<Blob> current(keypoints.size());
        int currBlob=keypoints.size();
        a=0;
        for(KeyPoint k : keypoints){
            cout<<"Blob x position: "<<k.pt.x<<" Blob Y position: "<<k.pt.y<<endl;
            current[a].area=k.size;
            current[a].x=k.pt.x;
            current[a].y=k.pt.y;
            current[a].time= buffer;
            //Check before previous frame if two blobs became one
            if (currBlob<prevBlob){
                for(Blob p:prev1){
                    if(p.area > 1){
                        if(std::abs(current[a].x-p.x)<radio(current[a].area) && std::abs(current[a].y-p.y)<radio(current[a].area)){
                            current[a].id = p.id;
                            break;
                        }//if
                    }//if
                }//for
            }//if
            
            for(Blob p:prev){
                if(p.area > 1){
                    if(std::abs(current[a].x-p.x)<radio(current[a].area) && std::abs(current[a].y-p.y)<radio(current[a].area)){
                        current[a].id = p.id;
                        break;
                    }//if
                }//if
            }//for
            cout<<current[a].area<<" FROM ID: "<<current[a].id<<endl;

//            //Minimum distance first algorithm
//            int id=0,dis,pdis=2000;
//            for(Blob p:prev){
//                if(p.area > 1){
//                    dis= sqrt(pow((current[a].x-p.x),2)+ pow((current[a].y-p.y),2));
//                    if(dis<pdis){
//                        id=p.id;
//                        pdis=dis;
//                    }
//                }//if
//            }//for
//            for(Blob p:prev){
//                if(p.area > 1){
//                    dis= sqrt(pow((current[a].x-p.x),2)+ pow((current[a].y-p.y),2));
//                    if(dis<pdis){
//                        id=p.id;
//                        pdis=dis;
//                    }
//                }//if
//            }//for
//            if(pdis<(2*current[a].area)){
//                current[a].id=id;
//            }

            if(current[a].id == 0){
                current[a].id = g_id_max;
                g_id_max++;
            }//
            
            a++;
        }//for
        prevBlob=currBlob;
        
        //update previous blobs (clear them, put current into previous)
        //if(keypoints.size()!=0){
        cleanBlobArray(prev1);
        int a=0;
        for(Blob p:prev){
            prev1[a].area=p.area;
            prev1[a].x=p.x;
            prev1[a].y=p.y;
            prev1[a].time= p.time;
            prev1[a].id=p.id;
            a++;
        }
        for(Blob p1:prev){
            p1.area=0;
            p1.x=0;
            p1.y=0;
            p1.time=" ";
            p1.id=0;
        }
        for(int x=0;x<20;x++){
            prev[x].x=0;
            prev[x].y=0;
            prev[x].id=0;
            prev[x].area=0;
            prev[x].time="";
        }
        a=0;
        for(Blob c : current){
            prev[a].area=c.area;
            prev[a].x=c.x;
            prev[a].y=c.y;
            prev[a].time= c.time;
            prev[a].id=c.id;
            //write current blobs array to db
            string query= write_db(c.id,c.x,c.y,c.time);
            if (mysql_query(connection, query.c_str())){
                finish_with_error(connection);
            }//if
            a++;
        }//for
        //}//if
        

        if(frame==2012){
            cout<<"enough for this video";
        }
        
        if(waitKey( 30 ) >= 0){}
        //    break;
    }//for
    
}//main
