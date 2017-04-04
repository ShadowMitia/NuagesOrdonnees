#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    debug = true;
    modeDebug = 1;
    ///////////////////////////Caméra///////////////////////
    //imageTest.load("grayGrad8.jpg");
    imageTest.load("etoile.png");
    imageTest.resize(win_width, win_height);
    ///////////////////////////VectorField//////////////////
    vectorField.setup();
    ///////////////////////////Flock2d//////////////////////
    flock.setBounds(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
    flock.setBoundmode(1);
    flock.isVectorField=true;
    Flock2d *_Ptr = flock._Ptr;
    flock.gradientVectorField_Ptr = vectorField.gradientVectorField_Ptr;
    for (int i = div_width/2; i < win_width; i += div_width) {
        for(int j = div_height/2; j < win_height; j += div_height) {
            Boid2d * b = new Boid2d(_Ptr);
            b->setLoc(ofVec2f(i,j));
            b->positionInitiale = b->position;
            b->setValSepa(10, 10);
            b->setValCohe(5, 50);
            b->setValAlig(6, 60);
            b->setMaxForce(2);
            b->setMaxSpeed(2);
            b->active=false;
            totalBoids.push_back(b);
        }
    }
    boids.BoidsSetup(flock.gradientVectorField_Ptr);
    
    /////////////////////////contourFinder/////////////////
    contourFinder.setMinAreaRadius(60);
    contourFinder.setMaxAreaRadius(1000);
    contourFinder.setThreshold(40);
    contourFinder.setUseTargetColor(false);
    contourFinder.setAutoThreshold(false);
    ////////////////////////////////////////////////////////
    ofxCv::imitate(imageTest, imageTempMat, CV_8UC1);
    
    
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
        boids.startThread();
    }
    
//========================================= coeur ===============================
// borne du rectangle
  
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
    boidUpdate.clear();
    if (contourFinder.getContours().size()>0) {
        ofPolyline popo = contourFinder.getPolyline(0);
        for (int x = 0; x<(win_width/div_width); x++) {
            for (int y = 0; y<(win_height/div_height); y++) {
                Boid2d* b = totalBoids[x * win_width/div_width + y];
                //cout << b->positionInitiale.x << "   " << b->positionInitiale.y << endl;
                if (popo.inside(b->positionInitiale.x,b->positionInitiale.y)){
                    b->active = true;
                    b->color=ofColor::blue;
                    boidUpdate.push_back(b);
                }
                else{
                    b->active = false;
                    b->color=ofColor::red;
                }
            }
        }
    }
    //cout << boidUpdate.size() << endl;
}

//--------------------------------------------------------------
void ofApp::draw() {
    if (debug) {
        switch (modeDebug) {
            case 1:{
                for (int i = 0; i< totalBoids.size(); i++) {
                    Boid2d* b = totalBoids[i];
                    ofSetColor(b->color);
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


