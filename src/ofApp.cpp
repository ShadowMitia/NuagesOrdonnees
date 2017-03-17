#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
  videoStream.setup(640, 480);

  background.setIgnoreForeground(true);

  gui.setup();
  gui.add(resetBackground.set("Reset Background", false));
  gui.add(learningTime.set("Learning Time", 30, 0, 30));
  gui.add(thresholdValue.set("Threshold Value", 10, 0, 255));
}

//--------------------------------------------------------------
void ofApp::update(){
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
  }
}

//--------------------------------------------------------------
void ofApp::draw(){
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
