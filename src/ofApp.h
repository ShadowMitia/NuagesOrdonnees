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

/////////////////////////////////////////
    Flock2d flock;
    std::vector<Boid2d*> totalBoids;
    BoidsUpdateThread boidsUpdate[nbThreadBoids];
    std::vector<Boid2d*> boidUpdate[nbThreadBoids];
    std::vector<Boid2d*> boidReturnInitial;
    BoidsReturnInitialThread boidsReturnInital;
    VectorFieldGenerator vectorField;

  // Camera stream
  ofVideoGrabber videoStream;

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
    ofxMaskRGB MaskRGB;
    ofxMaskAlpha MaskAlpha;
    
    ofImage  imageGravure;
    ofTexture textureGravure;
    
    bool  boidUpdateBool;
};
