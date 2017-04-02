//
//  BoidsThread.h
//  curiositas2017
//
//  Created by FatDazz_mac on 02/04/2017.
//
//
#pragma once
#include "Flock2d.h"
#include "ofMain.h"

class BoidsThread: public ofThread {
public:
    Flock2d flock;
    BoidsThread() {
        flock.setBounds(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
        flock.setBoundmode(1);
    };
    
    void BoidsSetup() {
        int unit= 20;
        flock.isVectorField=false;
        
        for (int i = unit/2; i < ofGetWindowWidth(); i += unit) {
            for(int j = unit/2; j < ofGetWindowHeight(); j += unit) {
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
    
public:
    vector<Boid2d *> boidsActif;
    vector<vector<ofVec2f>> *vect;
    
    ofThreadChannel<std::vector<Boid2d*>> boidsUpdate;
    ofThreadChannel<std::vector<std::vector<ofVec2f>>> field;
    
    void threadedFunction() {
        std::vector<Boid2d*> boids;
        boidsUpdate.receive(boids);
        std::vector<std::vector<ofVec2f>> f;
        field.receive(f);
        flock.update(&boids, &boids, &f);
    };
};
