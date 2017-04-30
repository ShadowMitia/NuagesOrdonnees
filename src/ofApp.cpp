#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup() {

  // gui

  gui.setup();

  gui.add(minArea.setup("minArea", 0, 1, 5000));
  gui.add(maxArea.setup("maxArea", 0, 1, 10000));
  gui.add(threshold.setup("threshold", 0, 0, 100));

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
  debug = true;
  modeDebug = 2;
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
    
    ///////////////////shader////////////////////////////////
    shader.allocate(win_width, win_height, GL_RGBA);    
    MaskRGB.allocate(win_width, win_height, GL_RGBA);
    MaskAlpha.allocate(win_width, win_height, GL_RGBA);
    /*
    shader.update();
    ofTexture black;
    black.allocate(win_width, win_height, GL_RGBA);
    
    black.bind();
    ofBackground(ofColor::black);
    black.unbind();
    gravure.setTexture(black,0);
    gravure.setTexture(shader.getTexture(),1);
    gravure.update();
    */
  
  boidUpdateBool =true;

    
  cout << "fin ===> setup" << endl;

  //boidTrail.load("trail");


  //explosion
  time(&now);
  explosion = false;
  srand (time(NULL));
  temps = rand() % 5 + 5;
}
//--------------------------------------------------------------
void ofApp::update() {

  contourFinder.setMinAreaRadius(minArea);
  contourFinder.setMaxAreaRadius(maxArea);
  contourFinder.setThreshold(threshold);

  
  ofSetWindowTitle("FPS: " + std::to_string(ofGetFrameRate()) + "contours: " + std::to_string(contourFinder.size()) + "\n");

#if USE_KINECT
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
#else
    ofxCv::copyGray(imageTest, imageTemp);
    //imageTemp = imageTest;
#endif
    imageTempMat = ofxCv::toCv(imageTemp);
    ofxCv::threshold(imageTempMat, 120, false);
    
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
            for (int i = 0; i< contourFinder.getPolylines().size() && i<nbThreadBoids ; i++) {
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
       
	//ofVec2f vec =ofVec2f(((float) b->position.x/win_width), ((float) b->position.y/win_height));

      //ofVec2f vec =ofVec2f(((float) b->position.x/win_width)*2-1, ((float) b->position.y/win_width)*2-win_height/win_width);
        //((b->position)/win_width)*2.0 - ofVec2f(1, win_height/win_width);
	
	ofVec2f vec = ofVec2f((b->position.x/win_width)*2 - 1, (b->position.y/win_height)*2 - 1);
        
	shader.dfv[i] = ofVec2f((float) vec.x, (float) vec.y);

    }
    /*
    for (int i=0; i<totalBoids.size(); i++) {
        Boid2d* b = totalBoids.at(i);
        ofVec2f vec =ofVec2f(((float) b->position.x/win_width)*2-1, ((float) b->position.y/win_width)*2-win_height/win_width);
        shader.dfv[i]= ofVec2f((float) vec.x, (float) vec.y);
    }
    shader.dfvSize = totalBoids.size();
    */
    shader.dfvSize = boidUpdate[0].size();
    //shader.dfv[0]=ofVec2f(1.0,0.0);
    //shader.dfv[0]=ofVec2f(0.5,0.5);
    //shader.dfvSize = 1;

    shader.update();

    MaskRGB.setTexture(MaskRGB.getTexture(),0);
    MaskRGB.setTexture(shader.getTexture(),1);
    MaskRGB.update();
    MaskAlpha.setTexture(MaskRGB.getTexture(), 1);
    MaskAlpha.setTexture(shader.getTexture(), 0);
    MaskAlpha.update();


#if USE_KINECT
  }
#endif

  
  if (difftime(time(&now), mark) >= temps && explosion == false){
	  for (int i = 0; i < boidUpdate[0].size(); i++) {
		  boidUpdate[0][i]->setValSepa(300, 100);
		  boidUpdate[0][i]->setValCohe(100, 300);
		  boidUpdate[0][i]->setValAlig(10, 35);
		  boidUpdate[0][i]->setMaxForce(3);
		  boidUpdate[0][i]->setMaxSpeed(4);
	  }
	  //cout << "testA" << endl;
	  time(&mark);
	  explosion = true;
  }
  else if (difftime(time(&now), mark) >= 2 && explosion == true) {
	  for (int i = 0; i < boidUpdate[0].size(); i++) {
		  boidUpdate[0][i]->setValSepa(30, 10);
		  boidUpdate[0][i]->setValCohe(10, 30);
		  boidUpdate[0][i]->setValAlig(10, 35);
		  boidUpdate[0][i]->setMaxForce(2);
		  boidUpdate[0][i]->setMaxSpeed(2);
	  }
	  //cout << "testB" << endl;
	  time(&mark);
	  explosion = false;
	  temps = rand() % 5 + 5;
  }

}
//--------------------------------------------------------------
void ofApp::draw() {

    if (debug) {
        switch (modeDebug) {
            case 1:{
                ofBackground(ofColor::violet);
                MaskAlpha.draw();
                }
                break;
            case 2:{
                //rip.draw(0,0);
	      ofBackground(ofColor::orange);
	      imageTest.draw(0,0);	      
                shader.draw();
		for (int i = 0; i < contourFinder.getContours().size(); i++){
		  contourFinder.getPolyline(i).draw();
		}

                }
                break;
            case 3:{
                ofImage image = imageTest;
                image.update();
                image.draw(0, 0);
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
                MaskRGB.draw();
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
        }
    }
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

