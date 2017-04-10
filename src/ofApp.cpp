#include "ofApp.h"
//--------------------------------------------------------------
void ofApp::setup() {
    debug = true;
    modeDebug = 2;
    ///////////////////////////Caméra///////////////////////
    imageTest.load("grayGrad8.jpg");
    //imageTest.load("666.png");

    imageTest.resize(win_width, win_height);
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
    contourFinder.setMinAreaRadius(6);
    contourFinder.setMaxAreaRadius(1000);
    contourFinder.setThreshold(40);
    contourFinder.setUseTargetColor(false);
    contourFinder.setAutoThreshold(false);
    contourFinder.setFindHoles(true);
    
    ////////////////////////////////////////////////////////
    ofxCv::imitate(imageTest, imageTempMat, CV_8UC1);
    
    rip.allocate(win_width, win_height, GL_RGBA);
    ///////////////////shader////////////////////////////////
    shader.allocate(win_width, win_height, GL_RGBA);

    string frac = STRINGIFY(
                         uniform vec2 resolution;
                         uniform vec2 tab[1000];
                         uniform int tabSize;
                         float makePoint(float x,float y,float sx,float sy){
                             //float xx=x+sin(t*fx)*sx;
                             //float yy=y+cos(t*fy)*sy;
                             float xx=x+sx;
                             float yy=y+sy;
                             return 1.0/sqrt(xx*xx+yy*yy);
                         }
                         vec3 gu(vec4 a,vec4 b,float f){
                             return mix(a.xyz,b.xyz,(f-a.w)*(1.0/(b.w-a.w)));
                         }
                         vec3 grad(float f){
                             vec4 c01=vec4(0.0,0.0,0.0,0.00);
                             vec4 c02=vec4(0.5,0.0,0.0,0.80);
                             vec4 c03=vec4(1.0,0.0,0.0,0.95);
                             vec4 c04=vec4(1.0,0.75,0.0,0.97);
                             vec4 c05=vec4(1.0,1.0,1.0,1.00);
                             return (f<c02.w)?gu(c01,c02,f):
                             (f<c03.w)?gu(c02,c03,f):
                             (f<c04.w)?gu(c03,c04,f):
                             gu(c04,c05,f);
                         }
                         void main( void ) {
                             vec2 p=(gl_FragCoord.xy/resolution.x)*2.0-vec2(1.0,resolution.y/resolution.x);
                             
                             //p=p*2.0;
                             
                             float x=p.x;
                             float y=p.y;
                             
                             float a = 0.0;
                             for (int i=0; i<tabSize; i++) {
                                 a=a+makePoint(x,y,tab[i].x,tab[i].y);
                             }
                             vec3 a1=grad(a/2900.0);
                             
                             gl_FragColor = vec4(a1.y,a1.x,a1.z,1.0);
                         }
                         );
    shader.setCode(frac);
    
    //shader.dfv[0] = ofVec2f(0, 0);
    //shader.dfv[1] = ofVec2f((float)1/10, 0);
    //shader.dfv[2] = ofVec2f((float)1/20, 0);
    //shader.dfvSize = 3;
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
    }else cout << "NON StartVectorFreldThread" << endl;

    ///////////////////////////////// End If new image ///////////////////////////////////
    
    for (int i=0; i<contourFinder.getPolylines().size() && i<nbThreadBoids ; i++) {
        if (boidsUpdate[i].isMainThread() && !boidsUpdate[i].isThreadRunning()) {
            boidsUpdate[i].boidsUpdate.send(boidUpdate[i]);
            boidsUpdate[i].startThread();
        }else cout << "NON StartBoidsThread:" << std::to_string(i) << endl;
        boidUpdate[i].clear();
    }
    boidsReturnInital.boidsReturnInitial.send(boidReturnInitial);
    boidsReturnInital.startThread();
    
    boidReturnInitial.clear();
    for (int x = 0; x<(win_width/div_width); x++) {
        for (int y = 0; y<(win_height/div_height); y++) {
            Boid2d* b = totalBoids[x * win_width/div_width + y];
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
                b->size = 20; /// attention il ne faut pas que mettre ça ici car il faut que le boids soit à sa position initial
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
    //cout << shader.dfv[3] << endl;
    //cout << boidUpdate[0].size() << endl;
    shader.update();
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
                //rip.draw(0,0);
                shader.draw();
                contourFinder.getPolyline(0).draw();
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
            case '7':
                modeDebug = 7;
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


