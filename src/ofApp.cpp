#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
  videoStream.setup(640, 480);

  background.setIgnoreForeground(true);

  gui.setup();
  gui.add(resetBackground.set("Reset Background", false));
  gui.add(learningTime.set("Learning Time", 30, 0, 30));
  gui.add(thresholdValue.set("Threshold Value", 10, 0, 255));

  boids.BoidsSetup();

  ofBackground(0, 0, 0);


}

//--------------------------------------------------------------
void ofApp::update() {
  ofSetWindowTitle("FPS: " + std::to_string(ofGetFrameRate()));

  if (boids.isMainThread() && !boids.isThreadRunning()) {
    boids.startThread();
  }

  videoStream.update();
  if(resetBackground) {
    background.reset();
    resetBackground = false;
  }

  if(videoStream.isFrameNew()) {
    background.setLearningTime(learningTime);
    background.setThresholdValue(thresholdValue);
    background.update(videoStream, thresholded);
    thresholded.update();
    contourFinder.setMinAreaRadius(60);
    contourFinder.setMaxAreaRadius(1000);
    contourFinder.setThreshold(thresholdValue);
    contourFinder.findContours(videoStream);
    ofxCv::toOf(background.getBackground(), backgroundImage);
    backgroundImage.update();
    ofxCv::subtract(backgroundImage, videoStream, contourImage);
    contourImage.update();
    contourImage.setImageType(OF_IMAGE_GRAYSCALE);
    contourFinder2.setMinAreaRadius(60);
    contourFinder2.setMaxAreaRadius(1000);
    contourFinder2.setThreshold(40);
    contourFinder2.findContours(contourImage);
  }


  poly.clear();
  float i = 0;
  while (i < TWO_PI) { // make a heart
    float r = (2-2*sin(i) + sin(i)*sqrt(abs(cos(i))) / (sin(i)+1.4)) * - 20;
    float x = ofGetMouseX() + 100 + cos(i) * r;
    float y = ofGetMouseY() + 100 + sin(i) * r;
    poly.addVertex(ofVec2f(x,y));
    i+=0.005*HALF_PI*0.5;
  }
  poly.close(); // close the shape

  boidUpdate.clear();
  auto minX = std::max(0, ofGetMouseY()/25);
  auto minY = std::max(0, ofGetMouseX()/25);
  auto maxX = std::min((ofGetMouseY()+200)/25, (ofGetWindowHeight())/25);
  auto maxY = std::min((ofGetMouseX()+200)/25, (ofGetWindowWidth())/25);
  for (auto i = minX; i < maxX; i++) {
    for (auto j = minY; j < maxY; j++) {
      auto width = ofGetWindowHeight() / 25;
      auto& b = boids.getBoids()[j*width+i];
      boidUpdate.push_back(b);
      if (poly.inside(b->positionInitiale)) {
	  b->active = true;
      }	else {
	b->active = false;
      }
    }
  }
  boids.boidsUpdate.send(boidUpdate);
}

//--------------------------------------------------------------
void ofApp::draw() {

  /*
  videoStream.draw(0, 0);
  if(thresholded.isAllocated()) {
    thresholded.draw(640, 0);
  }
  contourFinder.draw();
  ofPushMatrix();
  ofTranslate(640, 0);
  contourFinder.draw();
  ofPopMatrix();


  backgroundImage.draw(0, 480);
  contourImage.draw(640, 480);
  ofPushMatrix();
  ofTranslate(640, 480);
  contourFinder2.draw();
  ofPopMatrix();
  */
  ofPushMatrix();
  ofNoFill();
  ofSetColor(ofColor::blue);
  ofDrawRectangle(ofGetMouseX(), ofGetMouseY(), 200, 200);
  poly.draw();
  //ofDrawRectangle(0, 0, 200, 200);
  ofFill();
  ofPopMatrix();

  ofPushMatrix();
  ofSetColor(ofColor::red);
  for (int i = 0; i< boids.getBoids().size(); i++) {
    Boid2d  *b = boids.flock.totalBoid.at(i);
    ofDrawRectangle(b->position.x, b->position.y, 5,5);
    float lm = 10.f;
    ofDrawLine(b->position.x, b->position.y, b->position.x + b->velocite.x*lm, b->position.y + b->velocite.y*lm);
  }

  ofPopMatrix();
  ofSetColor(ofColor::white);
  //gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
