#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    debug = true;
    modeDebug = 1;
    //cam
    //imageTest.load("grayGrad8.jpg");
	imageTest.load("etoile.png");
  
    vectorField.setup();
    boids.BoidsSetup();
    boids.gradientVectorField_Ptr = vectorField.gradientVectorField_Ptr;
    contourFinder.setMinAreaRadius(60);
    contourFinder.setMaxAreaRadius(1000);
    contourFinder.setThreshold(40);
    
    contourFinder.setUseTargetColor(false);
    contourFinder.setAutoThreshold(false);
    ofxCv::imitate(imageTest, imageTempMat, CV_8UC1);
    
    
    //sleep(2);
    
}

//--------------------------------------------------------------
void ofApp::update() {
    if (debug) ofSetWindowTitle("FPS: " + std::to_string((int)ofGetFrameRate())+ "  Mode debug: " + std::to_string(modeDebug));
    else ofSetWindowTitle("FPS: " + std::to_string((int)ofGetFrameRate()));
    
    //////////////////////////////////// If new image /////////////////////////////////////
    ofxCv::copyGray(imageTest, imageTemp);
    //imageTemp = imageTest;
    
    imageTempMat = ofxCv::toCv(imageTemp);
    ofxCv::threshold(imageTempMat, 40, false);
    contourFinder.findContours(imageTemp);
    
    if (vectorField.isMainThread() && !vectorField.isThreadRunning() && contourFinder.getContours().size()>0) {
        vectorField.pix.send(imageTempMat);
        vectorField.startThread();
    }

    ///////////////////////////////// End If new image ///////////////////////////////////

    
    if (boids.isMainThread() && !boids.isThreadRunning()) {
        boids.boidsUpdate.send(boidUpdate);
        //boids.field.send(vectorField.gradientVectorField);
        boids.startThread();
    }
    

  poly.clear();
//========================================= coeur ===============================
  float i = 0;
  while (i < TWO_PI) { // make a heart
    float r = (2-2*sin(i) + sin(i)*sqrt(abs(cos(i))) / (sin(i)+1.4)) * - 20;
    float x = ofGetMouseX() + 100 + cos(i) * r;
    float y = ofGetMouseY() + 100 + sin(i) * r;
    poly.addVertex(ofVec2f(x,y));
    i+=0.005*HALF_PI*0.5;
  }
  poly.close(); // close the shape
//========================================= coeur ===============================
// borne du rectangle
  boidUpdate.clear();
/*  auto minX = std::max(0, ofGetMouseY()/25);
  auto minY = std::max(0, ofGetMouseX()/25);
  auto maxX = std::min((ofGetMouseY()+200)/25, ((ofGetWindowHeight())/25)-1);
  auto maxY = std::min((ofGetMouseX()+200)/25, ((ofGetWindowWidth())/25)-1);
//
  for (int i = minX; i < maxX; i++) {
    for (int j = minY; j < maxY; j++) {
      int width = ofGetWindowHeight() / 25;
      Boid2d* b = boids.getBoids()[j*width+i];
      boidUpdate.push_back(b);
      if (poly.inside(b->positionInitiale)) {
          b->active = true;
      }	else {
          b->active = false;
      }
    }
  }*/
    if (contourFinder.getContours().size()>0) {
        ofPolyline popo = contourFinder.getPolyline(0);
        //popo.close();
        for (int x = 0; x<(ofGetWindowWidth()/20); x++) {
            for (int y = 0; y<(ofGetWindowHeight()/20); y++) {
                Boid2d* b = boids.getBoids()[x * ofGetWindowHeight()/20 + y];
                //cout << b->positionInitiale.x << "   " << b->positionInitiale.y << endl;
                if (popo.inside(b->positionInitiale.x,b->positionInitiale.y)){
                    b->active = true;
                    boidUpdate.push_back(b);
                    
                }
                else  b->active = false;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    if (debug) {
        switch (modeDebug) {
            case 1:{
                ofSetColor(ofColor::red);
                for (int i = 0; i< boids.getBoids().size(); i++) {
                    Boid2d* b = boids.flock.totalBoid.at(i);
                    ofDrawRectangle(b->position.x, b->position.y, 3,3);
                    float lm = 10.f;
                    ofDrawLine(b->position.x, b->position.y, b->position.x + b->velocite.x*lm, b->position.y + b->velocite.y*lm);
                }
                ofSetColor(ofColor::white);}
                break;
            case 2:{
                imageTest.draw(0, 0);}
                break;
            case 3:{
                ofImage image = imageTest;
                image.update();
                image.draw(0, 0);
                }
                break;
            case 4:{
                ofSetColor(ofColor::azure);
                contourFinder.getPolyline(0).draw();
                ofSetColor(ofColor::white);}
                break;
            case 5:{
                ofImage image, drawImage;
                ofxCv::toOf(vectorField.getPixelisationMat(), image);
                image.update();
                drawImage = image;
                drawImage.draw(0, 0);
            }
            break;
            case 6:{
                ofImage image = imageTest;
                image.update();
                image.draw(0, 0);
                ofSetColor(ofColor::red);
                for (int i=0; i<vectorField.gradientVectorField_Ptr->size(); i++) {
                    for (int j=0; j<vectorField.gradientVectorField_Ptr->at(i).size(); j++) {
                        int _x = vectorField.gradientVectorField_Ptr->at(i).at(j).x;
                        int _y = vectorField.gradientVectorField_Ptr->at(i).at(j).y;
                        ofDrawLine(i * divGrad_width, j*divGrad_height, i*divGrad_width + _x, j*divGrad_height + _y);
                    }
                }
                ofSetColor(ofColor::white);
            }
            break;
        }
        //t.setImageType(OF_IMAGE_GRAYSCALE);
        //ofxCv::toOf(contourFinder.getContourThreshold(), t);
        //t.draw(0, 0);
    }
  
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == ' ') {
        debug = !debug;
    }
    
    if (debug) {
        switch (key) {
            case '1':
                modeDebug = 1;
                break;
            case '2':
                modeDebug = 2;
                break;
            case '3':
                modeDebug = 3;
                break;
            case '4':
                modeDebug = 4;
                break;
            case '5':
                modeDebug = 5;
                break;
            case '6':
                modeDebug = 6;
                break;
        }
    }
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


