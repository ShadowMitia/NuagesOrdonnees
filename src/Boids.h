//
//  Boids.h
//  curiositas2017
//
//  Created by FatDazz_mac on 04/04/2017.
//
//
#pragma once
#include <ofMain.h>
#include "Constant.h"
class Flock2d;

class Boid2d {
public:
    //////////////////////////Variable////////////////////////////
    Flock2d * flockPtr;
    ofVec2f position, velocite, acceleration, positionInitiale;
    float separateGroup, alignGroup, cohesionGroup;
    float distSeparationGroup, distAlignGroup, distCohesionGroup;
    float maxSpeed, maxForce;
    ofColor color;
    float size;
    bool active;
    ///////////////////////////////////////////////////////////////////
    
    Boid2d(Flock2d * flock);
    Boid2d * setFlock(Flock2d * flock);
    
    
    ////// set Valeurs
    Boid2d * setValTotal(ofVec2f _position,
                         float _sepa,float _alig, float _cohe,
                         float _distSepa,float _distAlig, float _distCohe,
                         float _maxSpeed, float _maxForce) {
        
        this->setLoc(_position);
        this->setValSepa(_sepa, _distSepa);
        this->setValCohe(_cohe, _distCohe);
        this->setValAlig(_alig, _distAlig);
        maxSpeed = _maxSpeed;
        maxForce = _maxForce;
        return this;
    }
    
    Boid2d * setLoc(ofVec2f _position) {
        position = _position;
        return this;
    }
    Boid2d * setVel(ofVec2f _velocite) {
        velocite=_velocite;
        return this;
    }
    Boid2d * setValSepa(float _sepa, float _distSepa){
        separateGroup = _sepa;
        distSeparationGroup = _distSepa;
        return this;
    }
    Boid2d * setValAlig(float _alig, float _distAlig){
        alignGroup = _alig;
        distAlignGroup = _distAlig;
        return this;
    }
    Boid2d * setValCohe(float _cohe, float _distCohe){
        cohesionGroup = _cohe;
        distCohesionGroup = _distCohe;
        return this;
    }
    Boid2d * setMaxSpeed(float _maxSpeed){
        maxSpeed=_maxSpeed;
        return this;
    }
    Boid2d * setMaxForce(float _maxForce){
        maxForce=_maxForce;
        return this;
    }

    ////// fin set Valeurs alex
    void bounds();
    void boudsColision();
 
    
    
    ///// fonction Utile /////
    void update(vector<Boid2d*> *otherBoids,vector<vector<ofVec2f>> *fieldVector);
    void returnInitial();

    
private:
    float* flockfullNew(float *vec, vector<Boid2d*> *otherBoids);
    float* steer(float* target, float amount);
    float* foncSep(const float dx, const float dy, const float invD, Boid2d *other, float *sep);
    float* foncCohe(const float d, const float variable,Boid2d *other, float *coh);
    float* foncAlig(Boid2d *other, float *ali);
    
};