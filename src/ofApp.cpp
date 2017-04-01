#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
  videoStream.setup(640, 480);

  boids.BoidsSetup();
  vectorField.setup();

  ofBackground(0, 0, 0);
}

//--------------------------------------------------------------
void ofApp::update() {
  ofSetWindowTitle("FPS: " + std::to_string(ofGetFrameRate()));

  if (boids.isMainThread() && !boids.isThreadRunning()) {
    boids.startThread();
  }

  if (vectorField.isMainThread() && !vectorField.isThreadRunning()) {
    vectorField.startThread();
  }

  videoStream.update();

  if (videoStream.isFrameNew()) {

    contourFinder.setMinAreaRadius(60);
    contourFinder.setMaxAreaRadius(1000);
    /*contourFinder.setThreshold(40);
      contourFinder.setAutoThreshold(false);*/
    contourFinder.findContours(videoStream);

    vectorField.pix.send(videoStream.getPixels());

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
  boids.field.send(vectorField.gradientVectorField);
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofPushMatrix();
  ofNoFill();
  ofSetColor(ofColor::blue);
  ofDrawRectangle(ofGetMouseX(), ofGetMouseY(), 200, 200);
  poly.draw();
  ofFill();
  ofPopMatrix();

  ofPushMatrix();
  ofSetColor(ofColor::red);
  for (int i = 0; i< boids.getBoids().size(); i++) {
    Boid2d* b = boids.flock.totalBoid.at(i);
    ofDrawRectangle(b->position.x, b->position.y, 5,5);
    float lm = 10.f;
    ofDrawLine(b->position.x, b->position.y, b->position.x + b->velocite.x*lm, b->position.y + b->velocite.y*lm);
  }

  ofPopMatrix();
  ofSetColor(ofColor::white);


  t.setImageType(OF_IMAGE_GRAYSCALE);
  ofxCv::toOf(contourFinder.getContourThreshold(), t);
  t.draw(0, 0);
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
