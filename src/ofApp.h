//-*- mode: c++ -*-
#pragma once


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "Flock2d.h"

class BoidsThread: public ofThread {
public:
    Flock2d flock;
    BoidsThread(){
        flock.setBounds(0, 0, ofGetWindowWidth()  , ofGetWindowHeight());
        flock.setBoundmode(1);
    };

    void BoidsSetup(){
        int unit= 30;
        flock.isVectorField=false;
        for (int i = (ofGetWidth()/unit); i < ofGetWidth(); i = i + ofGetWidth()/unit) {
            for(int j = (ofGetHeight()/unit);  j < ofGetHeight(); j = j+ ofGetHeight()/unit){
                flock.addBoid(ofVec2f(i, j),7, 10, 5, 50, 6, 60, 2, 2);
            }
        }
        cout << flock.totalBoid.size();
    };

    vector<Boid2d*>* getBoids(){
        return &flock.totalBoid;
    };

    void setBoidsActif(vector<Boid2d*> _boidsActif){
        boidsActif = _boidsActif;
    };

private:
    vector<Boid2d *> boidsActif;
    vector<vector<ofVec2f>> *vect;


    void threadedFunction(){
        //boidsActif=flock.totalBoid;
        //flock.update(&boidsActif, &boidsActif, vect);
        flock.update(&flock.totalBoid, &flock.totalBoid, vect);
    };
};



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

  BoidsThread boids;

  // Camera stream
  ofVideoGrabber videoStream;
  // Background detection things
  ofxCv::RunningBackground background;
  ofImage thresholded;
  ofImage backgroundImage;
  // GUI stuff
  ofxPanel gui;
  ofParameter<bool> resetBackground;
  ofParameter<float> learningTime, thresholdValue;

  // Contour Finder
  ofxCv::ContourFinder contourFinder;
  // Contour Finder
  ofxCv::ContourFinder contourFinder2;

  ofImage contourImage;
};
