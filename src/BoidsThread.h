//
//  BoidsThread.h
//  curiositas2017
//
//  Created by FatDazz_mac on 02/04/2017.
//
//
#pragma once
#include "Constant.h"
#include "ofMain.h"
#include "Boids.h"

///////////////////////////// Flock  //////////////////////////
class Flock2d {
public:
    ////////////////////////////////////// Vector Boid ////////////////////////////////
    Flock2d *_Ptr;
    /////////////////////////////////////// bounds//////////////////////////////////////
    float minX, minY, maxX, maxY, boundsWidth, boundsHeight;
    int boundmode;
    /////////////////////////////////////// vectorField ////////////////////////////////
    bool                            isVectorField;
    std::vector<std::vector<ofVec2f>> *gradientVectorField_Ptr;
    ////////////////////////////////////// dt //////////////////////////////////////
    float dt;
    
    Flock2d(){
        //clear();
        minX = maxX = maxY = boundsWidth = boundsHeight = 0;
        boundmode = 0;
        dt = 1.0f;
        _Ptr=this;
    }

    
    void update(vector<Boid2d *> *vectorThis, vector<Boid2d *> *otherBoids, vector<vector<ofVec2f>> *_vectorField){
        for (int i =0; i<vectorThis->size(); i++) {
            Boid2d *b = vectorThis->at(i);
            b->update(otherBoids, _vectorField);
            
        }
    }
    ///////////////////////////////// Bounds //////////////////////////////////
    Flock2d * setBounds(float minx, float miny, float maxx, float maxy) {
        minX = minx;
        minY = miny;
        maxX = maxx;
        maxY = maxy;
        boundsWidth = maxX - minX;
        boundsHeight = maxY - minY;
        return this;
    }
    int getBoundmode() {
        return boundmode;
    }
    Flock2d * setBoundmode(int boundmode) {
        this->boundmode = boundmode;
        return this;
    }

};
///////////////////////////// ThreadUpdate ////////////////////
class BoidsUpdateThread: public ofThread {
public:
    
    ofThreadChannel<std::vector<Boid2d*>> boidsUpdate;
    std::vector<std::vector<ofVec2f>> *gradientVectorField_Ptr;
    
    BoidsUpdateThread() {};
    
    void BoidsSetup(std::vector<std::vector<ofVec2f>> *_gradientVectorField_Ptr) {
        gradientVectorField_Ptr = _gradientVectorField_Ptr;
    };

private:
        ofMutex dataMutex;
        vector<vector<ofVec2f>> *vect;
    void threadedFunction() {
        std::vector<Boid2d*> boids;
        boidsUpdate.receive(boids);
        for (int i=0; i<boids.size(); i++) {
            boids[i]->update(&boids,gradientVectorField_Ptr);
        }
    };
};
////////////////////// ThreadReturnInitial ////////////////////
class BoidsReturnInitialThread: public ofThread {
public:
    ofThreadChannel<std::vector<Boid2d*>> boidsReturnInitial;
    BoidsReturnInitialThread() {};
    
    void BoidsSetup() {};
    
private:
    ofMutex dataMutex;
    void threadedFunction() {
        std::vector<Boid2d*> boids;
        boidsReturnInitial.receive(boids);
        for (int i=0; i<boids.size(); i++) {
            boids[i]->returnInitial();
        }
    };
};
