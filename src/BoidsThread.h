//
//  BoidsThread.h
//  curiositas2017
//
//  Created by FatDazz_mac on 02/04/2017.
//
//
#pragma once
#include "Constant.h"
#include "Flock2d.h"
#include "ofMain.h"

class BoidsThread: public ofThread {
public:
    
    Flock2d flock;
    ofThreadChannel<std::vector<Boid2d*>> boidsUpdate;
    std::vector<std::vector<ofVec2f>> *gradientVectorField_Ptr;
    
    BoidsThread() {
        flock.setBounds(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
        flock.setBoundmode(1);
    };
    
    void BoidsSetup() {
        
        flock.isVectorField=true;
        
        for (int i = div_width/2; i < win_width; i += div_width) {
            for(int j = div_height/2; j < win_height; j += div_height) {
                flock.addBoid(ofVec2f(i, j),7, 10, 5, 50, 6, 60, 2, 2);
            }
        }
    };
    
    vector<Boid2d*>& getBoids(){
        return flock.totalBoid;
    };
    
    void setBoidsActif(vector<Boid2d*> _boidsActif){
        boidsActif = _boidsActif;
    };
private:

        ofMutex dataMutex;
        vector<Boid2d *> boidsActif;
        vector<vector<ofVec2f>> *vect;
    

    void threadedFunction() {
        std::vector<Boid2d*> boids;
        boidsUpdate.receive(boids);
        //cout << gradientVectorField_Ptr->at(trunc(max(0.0f, min((float) (6*35) /divGrad_width,(float)divGrad_width-1)))).at(trunc(max(0.0f, min((float) 200 / divGrad_height,(float)divGrad_height-1))))<< endl;
        //cout <<gradientVectorField_Ptr->at(35).at(35) << endl;
        //cout << (6*35) /divGrad_width << endl;
        std::vector<std::vector<ofVec2f>> *g =gradientVectorField_Ptr;
        /*
        for (int i=0; i<g->size(); i++) {
            for (int j=0; j<g->at(i).size(); j++) {
                if (g->at(i).at(j).x != 0) {
                    cout << i << "  " << j << endl;
                    
                }
            }
        }
        */
        //cout << trunc(max(0.0f, min((float) (6*35) /divGrad_width,(float) (win_width/divGrad_width)-1))) << endl;
        flock.update(&boids, &boids, gradientVectorField_Ptr);
    };
};
