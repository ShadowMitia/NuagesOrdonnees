#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup() {

  Background.load("Background2.jpg");
  Background.resize(win_width, win_height);

  // gui
  gui.setup();
  gui.add(minArea.setup("minArea", 0, 1, 500));
  gui.add(maxArea.setup("maxArea", 0, 1, 1000));
  gui.add(threshold.setup("threshold", 0, 0, 100));
  ///////////////////shader////////////////////////////////
  shader.allocate(win_width, win_height, GL_RGBA);    
  MaskRGB.allocate(win_width, win_height, GL_RGBA);
  MaskAlpha.allocate(win_width, win_height, GL_RGBA);

  bloom.allocate(win_width, win_height, GL_RGBA);

#if USE_KINECT
  // Kinect stuff
  bool opened = kinect.open(0);
  if (!opened) {
    std::cout << "Kinect is not opening\n";
  }
    gui.add(minDistance.setup("minDistance", 500, 500, 6000));
    gui.add(maxDistance.setup("maxDistance", 6000, 500, 6000));
    
#else
  ///////////////////////////CamŽra///////////////////////
  imageTest.load("grayGrad8.jpg");
  //imageTest.load("666.png");

  imageTest.resize(win_width, win_height);
#endif
  debug = true;
  modeDebug = 3;
  MaskRGB.modeTime=1;

  ///////////////////////////VectorField//////////////////
  vectorField.setup();
  ///////////////////////////Flock2d//////////////////////
  flock.setBounds(0, 0, win_width, win_height);
  flock.setBoundmode(0);
  flock.isVectorField=true;
  Flock2d *_Ptr = flock._Ptr;
  flock.gradientVectorField_Ptr = vectorField.gradientVectorField_Ptr;
  for (int i = div_width/2; i < win_width; i += div_width) {
    for(int j = div_height/2; j < win_height; j += div_height) {
      Boid2d * b = new Boid2d(_Ptr);
      b->setLoc(ofVec2f(i,j));
      b->positionInitiale = b->position;
      b->setValSepa(30, 10);
      b->setValCohe(10, 30);
      b->setValAlig(10, 35);
      b->setMaxForce(2);
      b->setMaxSpeed(2);
      b->active=false;
      b->size = div_width;
      totalBoids.push_back(b);
    }
  }
  for(int i=0; i< nbThreadBoids;i++) {
    boidsUpdate[i].BoidsSetup(flock.gradientVectorField_Ptr);
  }

  /////////////////////////contourFinder/////////////////

  //contourFinder.setUseTargetColor(false);
  //contourFinder.setTargetColor(ofColor::white);
  contourFinder.setAutoThreshold(false);
  contourFinder.setFindHoles(true);
    
    cout << "total boids" << totalBoids.size() << endl;
    /////////////////////////contourFinder/////////////////
    contourFinder.setMinAreaRadius(6);
    contourFinder.setMaxAreaRadius(1000);
    contourFinder.setThreshold(40);
    contourFinder.setUseTargetColor(false);
    contourFinder.setAutoThreshold(false);
    contourFinder.setFindHoles(true);
    
    ////////////////////////////////////////////////////////
    ofxCv::imitate(imageTest, imageTempMat, CV_8UC1);
    
    
  cout << "fin ===> setup" << endl;


  //explosion
  explosion = false;
  temps = ofRandom(5, 11);

}
//--------------------------------------------------------------
void ofApp::update() {

  contourFinder.setMinAreaRadius(minArea);
  contourFinder.setMaxAreaRadius(maxArea);
  contourFinder.setThreshold(threshold);
  
  ofSetWindowTitle("FPS: " + std::to_string(ofGetFrameRate()) + "contours: " + std::to_string(contourFinder.size()) + "\n");

#if USE_KINECT
  kinect.minDistance=minDistance;
  kinect.maxDistance=maxDistance;
  kinect.update();
  if (kinect.isFrameNew()) {
      kinectTex.loadData(kinect.getDepthPixels());
#endif
    //////////////////////////////////// If new image /////////////////////////////////////
#if USE_KINECT
    imageTemp.setFromPixels(kinect.getDepthPixels());
    imageTemp.setImageType(OF_IMAGE_GRAYSCALE);
    imageTemp.update();
    imageTemp.resize(win_width, win_height);
    imageTemp.update();
    imageTempMat = ofxCv::toCv(imageTemp);
  
#else
    ofxCv::copyGray(imageTest, imageTemp);
    //imageTemp = imageTest;
#endif
    imageTempMat = ofxCv::toCv(imageTemp);
    ofxCv::threshold(imageTempMat, 120, false);
    contourFinder.findContours(imageTemp);

#if 0
    //  imageTemp.resize(win_width/2, win_height/2);
    //ofxCv::resize(imageTemp, imageTemp, 2, 2);
#endif
    
    if (vectorField.isMainThread() && !vectorField.isThreadRunning() && contourFinder.getContours().size() > 0) {
      vectorField.pix.send(imageTempMat);
      vectorField.startThread();
    } else {
      cout << "NON StartVectorFreldThread" << endl;
    }

    ///////////////////////////////// End If new image ///////////////////////////////////
#if USE_KINECT
  }
#endif

      for (int i=0; i<contourFinder.getPolylines().size() && i<nbThreadBoids ; i++) {
	if (boidsUpdate[i].isMainThread() && !boidsUpdate[i].isThreadRunning()) {
	  boidsUpdate[i].boidsUpdate.send(boidUpdate[i]);
	  boidsUpdate[i].startThread();
	}else cout << "NON StartBoidsThread:" << std::to_string(i) << endl;
	boidUpdate[i].clear();
      }
    boidsReturnInital.boidsReturnInitial.send(boidReturnInitial);
    boidsReturnInital.startThread();
    
#if USE_KINECT
    if (kinect.isFrameNew()) {
#endif

    boidReturnInitial.clear();
    shader.dfvSize = 0;
    for (int x = 0; x<(win_width/div_width); x++) {
        for (int y = 0; y<(win_height/div_height); y++) {
            Boid2d* b = totalBoids[y * win_width/div_width + x];
            bool active = false;
            bool insideHole = false;
            int index = -1;
            for (int i = 0; i< contourFinder.getPolylines().size() && i<nbThreadBoids ; i++) {
                if (contourFinder.getPolyline(i).inside(b->positionInitiale.x,b->positionInitiale.y)){
                    index = i;
                    active = true;
                }
                if (contourFinder.getHole(i) && contourFinder.getPolyline(i).inside(b->positionInitiale.x,b->positionInitiale.y)){
                    active = false;
                    insideHole = true;
                }
            }
            if (!active){
                b->active = false;
                b->size = min(float (div_width -1.0), float (b->size + 6.5));
                boidReturnInitial.push_back(b);
            }
            else {
                b->active = true;
                b->size = max(0.f, float (b->size - 6.5));
                if (b->size == 0.0) {
                    boidUpdate[index].push_back(b);
                    //// tableau du shader
                    ofVec2f vec = ofVec2f((b->position.x/win_width)*2 - 1, (b->position.y/win_height)*2 - 1);
                    shader.dfv[shader.dfvSize] = ofVec2f((float) vec.x, (float) vec.y);
                    shader.dfvSize++;
                    }
            }
        }
    }

#if USE_KINECT
    }
#endif

    shader.update();
    MaskRGB.setTexture(MaskRGB.getTexture(),0);
    MaskRGB.setTexture(shader.getTexture(),1);
    MaskRGB.update();
    MaskAlpha.setTexture(MaskRGB.getTexture(), 1);
    MaskAlpha.setTexture(shader.getTexture(), 0);
    MaskAlpha.update();
    bloom << MaskAlpha;

  
  if (difftime(time(&now), mark) >= temps && explosion == false){
	  for (int i = 0; i < boidUpdate[0].size(); i++) {
		  boidUpdate[0][i]->setValSepa(30, 100);
		  boidUpdate[0][i]->setValCohe(60, 300);
		  boidUpdate[0][i]->setValAlig(10, 35);
		  boidUpdate[0][i]->setMaxForce(3);
		  boidUpdate[0][i]->setMaxSpeed(4);
	  }
	  //cout << "testA" << endl;
	  ofResetElapsedTimeCounter();
	  explosion = true;
  }
  else if (difftime(time(&now), mark) >= 1 && explosion == true) {

	  for (int i = 0; i < boidUpdate[0].size(); i++) {
		  boidUpdate[0][i]->setValSepa(30, 10);
		  boidUpdate[0][i]->setValCohe(10, 30);
		  boidUpdate[0][i]->setMaxForce(2);
		  boidUpdate[0][i]->setMaxSpeed(2);
	  }
	  //cout << "testB" << endl;
	  ofResetElapsedTimeCounter();
	  explosion = false;
	  temps = ofRandom(5, 11);
  }


}
//--------------------------------------------------------------
void ofApp::draw() {



    ofBackground(ofColor::violet);
    Background.draw(0, 0,win_width,win_height);
    MaskAlpha.draw(0,0,win_width,win_height);
    ofSetColor(ofColor::white);
    for (int i = 0; i< totalBoids.size(); i++) {
        Boid2d* b = totalBoids[i];
        ofDrawRectangle((b->position.x - b->size/2),(b->position.y - b->size/2), (b->size),(b->size));
    }

    if (debug) {
        switch (modeDebug) {
            case 1:{
                ofBackground(ofColor::violet);
                MaskAlpha.draw();
                }
                break;
            case 2:{
	      ofBackground(ofColor::violet);
                shader.draw();
		for (int i = 0; i < contourFinder.getContours().size(); i++){
		  contourFinder.getPolyline(i).draw();
			}
                }
                break;
            case 3:{
                ofBackground(ofColor::violet);
                MaskRGB.draw();
                }
                break;
            case 4:{
		}
                break;
            case 5:{
            	}
            break;
            case 6:{
		ofBackground(ofColor::violet);
                ofImage image = imageTest;
                image.update();
                image.draw(0, 0);
                ofSetColor(ofColor::red);
                for (int i=0; i<vectorField.gradientVectorField_Ptr->size(); i++) {
                    for (int j=0; j<vectorField.gradientVectorField_Ptr->at(i).size(); j++) {
                        int _x = 10*vectorField.gradientVectorField_Ptr->at(i).at(j).x;
                        int _y = 10*vectorField.gradientVectorField_Ptr->at(i).at(j).y;
                        ofDrawLine(i * divGrad_width, j*divGrad_height, i*divGrad_width + _x, j*divGrad_height + _y);
                    }
                }
                ofSetColor(ofColor::white);
            }
            break;
            case 7:{
		ofBackground(ofColor::violet);
                for (int i = 0; i< totalBoids.size(); i++) {
                    Boid2d* b = totalBoids[i];
                    ofSetColor(b->color);
                    ofDrawRectangle(b->position.x - b->size/2, b->position.y - b->size/2, b->size,b->size);
                }
                ofSetColor(ofColor::white);
            }
        }
        gui.draw();

        
    }
    
  }
  //--------------------------------------------------------------
  void ofApp::keyPressed(int key){
    if (key == ' ') {
      debug = !debug;
    }
    if (debug) {
      switch (key) {
      case 'b':
	break;
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
      case '7':
	modeDebug = 7;
	break;
      case 'l':
	MaskRGB.modeTime = 1;
	break;
      case 'm':
	ofTexture black;
	black.bind();
	ofBackground(ofColor::black);
	black.unbind();
	MaskRGB.setTexture(black,0);
	MaskRGB.setTexture(shader.getTexture(),1);
	MaskRGB.update();
	break;

      }
    }
  }
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
  if (debug) {
    switch (key) {
    case 'l':
      MaskRGB.modeTime = 0;
      break;
    }
  }
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

void ofApp::exit() {
  //kinect.close();
  vectorField.waitForThread();
  boidsReturnInital.waitForThread();
  for (auto& b : boidsUpdate) {
    b.waitForThread();
  }
			    
}

