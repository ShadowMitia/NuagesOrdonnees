//-*- mode: c++ -*-
#pragma once


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "Flock2d.h"

constexpr int cam_width = 640;
constexpr int cam_height = 480;
constexpr int image_width = cam_width / 2;
constexpr int image_height = cam_height / 2;

class BoidsThread: public ofThread {
public:
  Flock2d flock;
  BoidsThread() {
    flock.setBounds(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    flock.setBoundmode(1);
  };

  void BoidsSetup() {
    int unit= 25;
    flock.isVectorField=false;

    for (int i = 25; i < ofGetWindowWidth(); i += unit) {
      for(int j = 25; j < ofGetWindowHeight(); j += unit) {
	flock.addBoid(ofVec2f(i, j),7, 10, 5, 50, 6, 60, 2, 2);
      }
    }
  };

  vector<Boid2d*>& getBoids(){
    return flock.totalBoid;
  };

  void setBoidsActif(vector<Boid2d*> _boidsActif){
    boidsActif = _boidsActif;
  };

public:
  vector<Boid2d *> boidsActif;
  vector<vector<ofVec2f>> *vect;

  ofThreadChannel<std::vector<Boid2d*>> boidsUpdate;
  ofThreadChannel<std::vector<std::vector<ofVec2f>>> field;

  void threadedFunction() {
    //boidsActif=flock.totalBoid;
    //flock.update(&boidsActif, &boidsActif, vect);
    std::vector<Boid2d*> boids;
    boidsUpdate.receive(boids);
    std::vector<std::vector<ofVec2f>> f;
    //field.receive(f);
    flock.update(&boids, &boids, vect); // il faut faire ça
  };
};

class VectorFieldGenerator : public ofThread {

public:

  void setup() {
    pixelisation.allocate(640, 480, OF_IMAGE_COLOR);
    pixelisation.setUseTexture(false);

    finalPixelisation.allocate(640 / unit, 480 / unit, OF_IMAGE_COLOR);
    finalPixelisation.setUseTexture(false);

    gray.allocate(640 / unit, 480 / unit, OF_IMAGE_GRAYSCALE);
    gray.setUseTexture(false);

    sobelX.allocate(640 / unit, 480 / unit, OF_IMAGE_GRAYSCALE);
    sobelX.setUseTexture(false);

    sobelY.allocate(640 / unit, 480 / unit, OF_IMAGE_GRAYSCALE);
    sobelY.setUseTexture(false);

    gradientVectorField.resize(640 / unit);
    for (auto& v : gradientVectorField) {
      v.resize(480 / unit);
    }
    tmp.resize(480/unit);
  }

  void threadedFunction() {
    pix.receive(pixelisation);
    pixelisation.mirror(false, true);

    for (int i = 0; i < pixelisation.getWidth(); i+= unit) {
      for (int j = 0; j < pixelisation.getHeight(); j+= unit) {
	unsigned int r = 0;
	unsigned int g = 0;
	unsigned int b = 0;
	int width = unit;
	int height = unit;
	for (int k = 0; k < width; k++) {
	  for (int l = 0; l < height; l++) {
	    ofColor col = pixelisation.getColor(i+k, j+l);
	    r += col.r;
	    g += col.g;
	    b += col.b;
	  }
	}
	r /= (width * height);
	g /= (width * height);
	b /= (width * height);
	finalPixelisation.setColor(i/unit, j/unit, ofColor(r, g, b));
      }
    }
    finalPixelisation.update();

    ofxCv::convertColor(finalPixelisation, gray, CV_RGB2GRAY);
    //=================================================================================<<<<<<<
    //ofxCv::imitate(sobelX, gray, CV_8SC1);
    cv::Mat srcMat = ofxCv::toCv(gray), dstMat = ofxCv::toCv(sobelX);

    //ofxCv::imitate(inter_x, gray, CV_32F);
    cv::Sobel(srcMat, inter_x, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
    inter_x.convertTo(inter_x, CV_32S);
    // image en X
    cv::convertScaleAbs(inter_x, dstMat);
    //=================================================================================<<<<<<<

    cv::Mat dstMat_ = ofxCv::toCv(sobelY);

    //ofxCv::imitate(inter_y, gray, CV_32F);
    cv::Sobel(srcMat, inter_y, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
    inter_y.convertTo(inter_y, CV_32S);
    // image en Y
    cv::convertScaleAbs(inter_y, dstMat_);
    //=================================================================================<<<<<<<
    cv::addWeighted(dstMat, 0.5, dstMat_, 0.5, 0, dstMat);

    //gradientVectorField.clear();
    for (unsigned int i=0; i<finalPixelisation.getWidth(); i++) {
      //std::vector<ofVec2f> vectorJ;
      //tmp.clear();
      for (unsigned int j=0; j<finalPixelisation.getHeight(); j++) {
	float x = inter_x.at<int>(j, i)/(float)1020;
	float y = inter_y.at<int>(j, i)/(float)1020;
	gradientVectorField[i][j] = ofVec2f(x,y);
	//tmp.push_back(ofVec2f(x, y));
      }
      //gradientVectorField.push_back(tmp);
    }
    gray.update();
    sobelX.update();
    sobelY.update();
    finalPixelisation.update();
  }


  ofThreadChannel<ofPixels> pix;
  ofThreadChannel<std::vector<std::vector<ofVec2f>>> vecField;

private:

  ofImage pixelisation;
  ofImage finalPixelisation;

  ofImage gray;
  ofImage sobelX;
  ofImage sobelY;
  ofImage gradient;

  cv::Mat src;
  cv::Mat inter_x,inter_y;
  cv::Mat mag;
  cv::Mat angle;

  cv::Mat grad;

  int unit = 2;
public:
  std::vector<std::vector<ofVec2f>> gradientVectorField;
  std::vector<ofVec2f> tmp;
  ofImage t;

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

  VectorFieldGenerator vectorField;

  // Camera stream
  ofVideoGrabber videoStream;

  // Contour Finder
  ofxCv::ContourFinder contourFinder;
  // Contour Finder
  ofxCv::ContourFinder contourFinder2;

  ofImage contourImage;

  //
  std::vector<Boid2d*> boidUpdate;

  int posX = 0;
  int posY = 0;

  ofPolyline poly;

  ofImage t;
};
