//
//  VectorFieldGenerator.h
//  curiositas2017
//
//  Created by FatDazz_mac on 02/04/2017.
//
//
#pragma once
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "Constant.h"

class VectorFieldGenerator : public ofThread {
public:
    std::vector<std::vector<ofVec2f>> *gradientVectorField_Ptr;
    
    ofThreadChannel<cv::Mat> pix;
    void setup() {
        ofxCv::allocate(inputMat, win_width, win_height, CV_8UC1);
        //ofxCv::allocate(pixelisationMat,win_width/divGrad_width,win_height/divGrad_height,CV_8UC1);
        
        pixelisationImage.allocate(win_width/divGrad_width, win_height/divGrad_height, OF_IMAGE_GRAYSCALE);
        pixelisationImage.setUseTexture(false);
        pixelisationMat = ofxCv::toCv(pixelisationImage);
        gradientVectorField.clear();
        gradientVectorField.resize(win_width/divGrad_width,vector<ofVec2f>(win_height/divGrad_height));
        gradientVectorField_Ptr = &gradientVectorField;
        /*
        gradientVectorField.resize(win_width/divGrad_width);
         for (auto& v : gradientVectorField) {
         v.resize(win_height/divGrad_height);
         }
         */

        ofxCv::imitate(inter_x, pixelisationMat, CV_32F);
        ofxCv::imitate(inter_y, pixelisationMat, CV_32F);
        cout << "inputMat cols:" << inputMat.cols << " rows:" << inputMat.rows << endl;
        cout << "pixelisation cols:" << pixelisationMat.cols << " rows:" << pixelisationMat.rows << endl;

    }
    cv::Mat getPixelisationMat(){
        return pixelisationMat;
    }
    cv::Mat getInputMat(){
        return inputMat;
    }
    cv::Mat getInter_x(){
        return inter_x;
    }
    cv::Mat getInter_y(){
        return inter_y;
    }
private:
    std::vector<std::vector<ofVec2f>> gradientVectorField;
    ofMutex dataMutex;
    cv::Mat inputMat,pixelisationMat;
    cv::Mat inter_x,inter_y;
    ofImage pixelisationImage;
    
    void threadedFunction() {
        cv::Mat m;
        pix.receive(m);
        ofxCv::copy(m, inputMat);
        //ofxCv::copy(inputMat,pixelisationMat);
        dataMutex.lock();
        pixelisation(&inputMat, &pixelisationMat);
        
        //pixelisationMat.convertTo(pixelisationMat, CV_8UC1);
        dataMutex.unlock();
        
        //=================================================================================<<<<<<<
        cv::Sobel(pixelisationMat, inter_x, CV_32F, 1, 0, 7, 1, 0, cv::BORDER_DEFAULT);
        inter_x.convertTo(inter_x, CV_32S);
        //=================================================================================<<<<<<<
        cv::Sobel(pixelisationMat, inter_y, CV_32F, 0, 1, 7, 1, 0, cv::BORDER_DEFAULT);
        inter_y.convertTo(inter_y, CV_32S);
        //=================================================================================<<<<<<<
        for (unsigned int i=0; i<pixelisationMat.cols; i++) {
            for (unsigned int j=0; j<pixelisationMat.rows; j++) {
                float x = inter_x.at<int>(j, i)/(float)1020;
                float y = inter_y.at<int>(j, i)/(float)1020;
                gradientVectorField[i][j] = ofVec2f(x,y);
            }
        }
    }
    void pixelisation(cv::Mat *src, cv::Mat *dst){
        for (int i=0; i<src->cols; i+=divGrad_width) {
            for (int j=0; j<src->rows; j+=divGrad_height) {
                int valeur = 0;
                for (int k=0; k<divGrad_width; k++) {
                    for (int l=0; l<divGrad_height; l++) {
                        valeur = valeur + src->at<int>(i+k, j+l);
                    }
                }
                valeur = valeur/(divGrad_width * divGrad_height);
                dst->at<int>(i/divGrad_width, j/divGrad_height) = valeur;
            }
        }
    }
};
