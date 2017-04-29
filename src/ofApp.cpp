#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup() {
  // gui

  gui.setup();

  gui.add(minArea.setup("minArea", 0, 1, 600));
  gui.add(maxArea.setup("maxArea", 0, 1, 600));
  gui.add(threshold.setup("threshold", 0, 0, 1000));


  
  debug = true;
  modeDebug = 2;
  gravure.modeTime=1;
  gravure._i=0;

  #if USE_KINECT
  // Kinect stuff
  bool opened = kinect.open(0);
  if (!opened) {
    std::cout << "Kinect is not opening\n";
  }

  #else
  ///////////////////////////CamŽra///////////////////////
  imageTest.load("grayGrad8.jpg");
  //imageTest.load("666.png");

  imageTest.resize(win_width, win_height);
  #endif

  ///////////////////////////VectorField//////////////////
  vectorField.setup();
  ///////////////////////////Flock2d//////////////////////
  flock.setBounds(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
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

  contourFinder.setUseTargetColor(true);
  contourFinder.setTargetColor(ofColor::white);
  contourFinder.setAutoThreshold(false);
  contourFinder.setFindHoles(true);
    
  ////////////////////////////////////////////////////////
  ofxCv::imitate(imageTest, imageTempMat, CV_8UC1);
    
  ///////////////////shader////////////////////////////////
  shader.allocate(win_width, win_height, GL_RGBA);    
  gravure.allocate(win_width, win_height, GL_RGBA);
  shader.update();
    
  black.allocate(win_width, win_height, GL_RGBA);
  black.bind();
  ofBackground(ofColor::black);
  black.unbind();
  gravure.setTexture(black,0);
  gravure.setTexture(shader.getTexture(),1);
  gravure.update();
  boidUpdateBool =true;
    
  cout << "fin ===> setup" << endl;

  //boidTrail.load("trail");
}
//--------------------------------------------------------------
void ofApp::update() {

  contourFinder.setMinAreaRadius(minArea);
  contourFinder.setMaxAreaRadius(maxArea);
  contourFinder.setThreshold(threshold);

  
  if (debug) ofSetWindowTitle("FPS: " + std::to_string(ofGetFrameRate())+ "  Mode debug: " + std::to_string(modeDebug));
  else ofSetWindowTitle("FPS: " + std::to_string(ofGetFrameRate()));

  #if USE_KINECT
  kinect.update();

  kinectTex.loadData(kinect.getDepthPixels());

  std::cout << "Depth: " << kinectTex.getWidth() << " " << kinectTex.getHeight() << "\n";
  
  if (kinect.isFrameNew()) {
#endif
    //////////////////////////////////// If new image /////////////////////////////////////
    #if USE_KINECT
    ofImage img;
    imageTemp.setFromPixels(kinect.getDepthPixels());
    imageTemp.setImageType(OF_IMAGE_GRAYSCALE);
    imageTemp.update();
    imageTemp.resize(1920, 1080);
    #else
    ofxCv::copyGray(imageTest, imageTemp);
    //imageTemp = imageTest;
    #endif
    imageTempMat = ofxCv::toCv(imageTemp);
    ofxCv::threshold(imageTempMat, 180, false);

    std::cout << "w " << imageTempMat.size().height << " " << imageTempMat.size().width << "\n";
    
    contourFinder.findContours(imageTemp);
    
    if (vectorField.isMainThread() && !vectorField.isThreadRunning() && contourFinder.getContours().size() > 0) {
      vectorField.pix.send(imageTempMat);
      vectorField.startThread();
    } else {
      cout << "NON StartVectorFreldThread" << endl;
    }

    ///////////////////////////////// End If new image ///////////////////////////////////
    
    if (boidUpdateBool) {
      for (int i=0; i<contourFinder.getPolylines().size() && i<nbThreadBoids ; i++) {
	if (boidsUpdate[i].isMainThread() && !boidsUpdate[i].isThreadRunning()) {
	  boidsUpdate[i].boidsUpdate.send(boidUpdate[i]);
	  boidsUpdate[i].startThread();
	}else cout << "NON StartBoidsThread:" << std::to_string(i) << endl;
	boidUpdate[i].clear();
      }
    }else boidUpdate[0].clear();
    
    boidsReturnInital.boidsReturnInitial.send(boidReturnInitial);
    boidsReturnInital.startThread();
    
    boidReturnInitial.clear();
    for (int x = 0; x<(win_width/div_width); x++) {
      for (int y = 0; y<(win_height/div_height); y++) {
	Boid2d* b = totalBoids[y * win_width/div_width + x];
	bool active = false;
	bool insideHole = false;
	int index = -1;
	for (int i = 0; i < contourFinder.getPolylines().size() && i < nbThreadBoids ; i++) {
	  if (contourFinder.getPolyline(i).inside(b->positionInitiale.x,b->positionInitiale.y)){
	    index = i;
	    active = true;
	  }
	}
	for (int i = 0; i< contourFinder.getPolylines().size() && i<nbThreadBoids ; i++) {
	  if (contourFinder.getHole(i) && contourFinder.getPolyline(i).inside(b->positionInitiale.x,b->positionInitiale.y)){
	    active = false;
	    insideHole = true;
	  }
	}
	if (!active){
	  b->active = false;
	  if (insideHole) {
	    b->color = ofColor::black;
	  } else {
	    b->color=ofColor::black;
	  }
	  b->size = 20; /// attention il ne faut pas que mettre a ici car il faut que le boids soit ˆ sa position initial
	  boidReturnInitial.push_back(b);
	} else {
	  b->active = true;
	  b->color = ofColor::blueViolet;
	  b->size = max(3.f, float (b->size - 0.25));
	  if (b->size == 3) {
	    boidUpdate[index].push_back(b);
	  }
	}
      }
    }
    

    for (int i=0; i<boidUpdate[0].size(); i++) {
      Boid2d* b = boidUpdate[0].at(i);
        
      ofVec2f vec =ofVec2f(((float) b->position.x/win_width)*2-1, ((float) b->position.y/win_width)*2-win_height/win_width);
        
      //((b->position)/win_width)*2.0 - ofVec2f(1, win_height/win_width);
      shader.dfv[i]= ofVec2f((float) vec.x, (float) vec.y);
    }
    shader.dfvSize = boidUpdate[0].size();
    shader.update();
    gravure.setTexture(gravure.getTexture(),0);
    gravure.setTexture(shader.getTexture(),1);
    gravure.update();
#if USE_KINECT
  }
#endif
}
//--------------------------------------------------------------
void ofApp::draw() {
  if (debug) {
    switch (modeDebug) {
    case 1:{
      ofBackground(ofColor::violet);
      gravure.draw();
      shader.draw();
    }
      break;
    case 2:{
      //rip.draw(0,0);
      ofBackground(ofColor::orangeRed);
      shader.draw();
      ofSetColor(ofColor::black);
      if (contourFinder.size() > 0) {
	contourFinder.getPolyline(0).draw();
      }
      ofSetColor(ofColor::white);
    }
      break;
    case 3:{
      //ofImage image = imageTest;
      imageTest.update();
      imageTest.draw(0, 0);
    }
      break;
    case 4:{
      ofSetColor(ofColor::green);
      for (int i = 0; i < contourFinder.getContours().size(); i++){
	contourFinder.getPolyline(i).draw();
      }
      ofSetColor(ofColor::white);}
      break;
    case 5:{
      ofBackground(ofColor::paleVioletRed);
      /*ofImage image, drawImage;
	ofxCv::toOf(vectorField.getPixelisationMat(), image);
	image.update();
	drawImage = image;
	drawImage.draw(0, 0);*/ //<<==== code pour la pixelisation
      gravure.draw();
      //shader.draw();
    }
      break;
    case 6:{
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
      for (int i = 0; i< totalBoids.size(); i++) {
	Boid2d* b = totalBoids[i];
	ofSetColor(b->color);
	ofDrawRectangle(b->position.x - b->size/2, b->position.y - b->size/2, b->size,b->size);
      }
      ofSetColor(ofColor::white);
    }
      break;
    }
  }


  //imageTemp.draw(0, 0);
  //ofSetColor(ofColor::blue);
  //if (contourFinder.size() > 0) {
  //  contourFinder.getPolyline(0).draw();
  //} else {
  //  std::cout << "Pas de contour\n";
  //}
  gui.draw();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
  if (key == ' ') {
    debug = !debug;
  }
  if (debug) {
    switch (key) {
    case 'b':
      boidUpdateBool=!boidUpdateBool;
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
      gravure.modeTime = 1;
      break;
    case 'm':
      black.bind();
      ofBackground(ofColor::black);
      black.unbind();
      gravure.setTexture(black,0);
      gravure.setTexture(shader.getTexture(),1);
      gravure.update();
      break;
    case (OF_KEY_LEFT):
      gravure._i+=0.01;
      cout << "i: " << gravure._i << "\n";
      break;
    case (OF_KEY_RIGHT):
      gravure._i-=0.01;
      cout << "i: " << gravure._i << "\n";	
      break;

    }
  }
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){
  if (debug) {
    switch (key) {
    case 'l':
      gravure.modeTime = 0;
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

