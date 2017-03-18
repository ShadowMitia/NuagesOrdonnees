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
}

//--------------------------------------------------------------
void ofApp::update() {
  if (boids.isMainThread()) {
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


}

//--------------------------------------------------------------
void ofApp::draw() {
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

  ofSetColor(0, 0, 0, 25);
  for (int i = 0; i< boids.getBoids()->size(); i++) {
    Boid2d  *b = boids.flock.totalBoid.at(i);
    ofDrawRectangle(b->position.x, b->position.y, 5,5);
    float lm = 10.f;
    ofDrawLine(b->position.x, b->position.y, b->position.x + b->velocite.x*lm, b->position.y + b->velocite.y*lm);
  }

  gui.draw();
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
