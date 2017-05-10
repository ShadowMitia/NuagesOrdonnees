//-*- mode: c++ -*-
#pragma once


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "BoidsThread.h"
#include "VectorFieldGenerator.h"
#include "ofxFX.h"
#include "ShaderFx.h"
#include "ofxKinectV2.h"
#include "ofxBloom.h"


class ofApp : public ofBaseApp {

public:
  void setup();
  void update();
  void draw();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y );
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);
  void exit();

  /////////////////////////////////////////
  Flock2d flock;
  std::vector<Boid2d*> totalBoids;
  BoidsUpdateThread boidsUpdate[nbThreadBoids];
  std::vector<Boid2d*> boidUpdate[nbThreadBoids];
  std::vector<Boid2d*> boidReturnInitial;
  BoidsReturnInitialThread boidsReturnInital;
  VectorFieldGenerator vectorField;
  
  
  // Camera stream
#if USE_KINECT
  ofxKinectV2 kinect;
  ofTexture kinectTex;
  ofxFloatSlider minDistance,maxDistance;
#endif
  

  // Contour Finder
  ofxCv::ContourFinder contourFinder;

  ofImage contourImage;

  int posX = 0;
  int posY = 0;

  ofImage imageTest;
  ofImage imageTemp;
  cv::Mat imageTempMat;
    

  bool debug;
  int modeDebug;

  //ofxRipples rip;
  ///////////////// shader ////////////////
  ofxBoidFx shader;
  ofVec2f tab[nbBoids];
  int tabSize;
  ofImage  imageGravure;
  ofTexture textureGravure;

  ofxMaskRGB MaskRGB;
  ofxMaskAlpha MaskAlpha;

  ofxBloom bloom;

  ofTexture black;

  ofShader boidTrail;

  ofxPanel gui;

  ofxFloatSlider minArea;
  ofxFloatSlider maxArea;
  ofxFloatSlider threshold;


  ofImage Background;

  time_t mark;
  time_t now;
  bool explosion;
  int temps;
  
};
