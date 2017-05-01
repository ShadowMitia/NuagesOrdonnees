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
        ofxCv::allocate(pixelisationMat,win_width/divGrad_width,win_height/divGrad_height,CV_8UC1);
        
        gradientVectorField.clear();
        gradientVectorField.resize(win_width/divGrad_width,vector<ofVec2f>(win_height/divGrad_height));
        gradientVectorField_Ptr = &gradientVectorField;

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
    ofImage pixelisationImage, inputImage;
    
    void threadedFunction() {
        cv::Mat m;
        pix.receive(m);
        ofxCv::copy(m, inputMat);
        //ofxCv::copy(inputMat,pixelisationMat);

        ofxCv::resize(inputMat, pixelisationMat);
        ofxCv::GaussianBlur(pixelisationMat, 10);
        
        //=================================================================================<<<<<<<
        cv::Sobel(pixelisationMat, inter_x, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
        inter_x.convertTo(inter_x, CV_32S);
        //=================================================================================<<<<<<<
        cv::Sobel(pixelisationMat, inter_y, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
        inter_y.convertTo(inter_y, CV_32S);
        //=================================================================================<<<<<<<
        for (unsigned int i=0; i<pixelisationMat.cols; i++) {
            for (unsigned int j=0; j<pixelisationMat.rows; j++) {
                float x = 4*inter_x.at<int>(j, i)/(float)1020;
                float y = 4*inter_y.at<int>(j, i)/(float)1020;
                // si 3=>1020  si 5=>12240 si 7=> 163200
                if (gradientVectorField[i].size()>0) {
                    gradientVectorField[i][j] = ofVec2f(x,y);
                }
            }
        }
    }
    
};