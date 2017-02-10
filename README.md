# Audience Monitor
This is the main repository for the Audience Monitor project. The main goal of the project is to provide an open source tool for monitoring display audience using depth information. The tool uses an off-the-shelf Microsoft Kinect depth sensor and openCV computer vision algorithms to monitor the space in front of a digital display. 

# System Overview
The system uses a standard Microsoft Kinect device to obtain a three-dimensional representation of the monitored scene and consists of two modules: _presence detection_ and _presence visualization_. The modules work as separate components interfaced through a MySQL database.

![Overview of the system architecture](/images/overview.png)

## Presence Detection

The presence detection module collects and analyzes the raw sensor readings, detects the presence of people, and stores detection and tracking information into a database. The module collects raw data from a Microsoft Kinect device that provides 640x480 pixels of 16-bit depth information, at the rate of 30 frames per second. Each detected presence is stored in a database containing a unique id, timestamp, x and y coordinates, and the size of the detected area. The module has been developed in C++ using the Xcode development environment and Mac OSX (https://developer.apple.com/xcode/).

The module uses OpenNI, OpenCV, and GLFW libraries:

 * OpenNI (https://github.com/OpenNI), a framework for obtaining raw sensor data from Kinect 
 * OpenCV (http://opencv.org), a computer vison library for analyzing raw data in real-time and detecting the presence and movement of passers-by
  * Here is a link to the openCV tutorial for using Kinect and openNI library http://docs.opencv.org/3.1.0/d7/d6f/tutorial_kinect_openni.html (Note: configure OpenCV with OpenNI support by setting WITH_OPENNI flag in CMake) 
 * GLFW openGL library (http://www.glfw.org) for transforming and rotating the three-dimensional view of the scene depending on the installation position and angle of the Kinect device
  * Here is a link to a video tutorial for setting up GLFW in Xcode on Mac https://www.youtube.com/watch?v=lTmM3Y8SMOM


## Presence Visualization

The presence visualization module provides insights into the collected datasets by the presence detection module. The module has been implemented using Play, a Java based web application development framework (https://www.playframework.com/), and D3.js, a JavaScript visualization library (https://d3js.org/). The module provides visualization of individual paths by presence id, visualization of audience presence at a specific time instance (or over a specific time interval), histograms of the presence detection over weeks, days, and hours of the deployment, dwell time of passers-by in front of the display, and distance representation using a heat map.