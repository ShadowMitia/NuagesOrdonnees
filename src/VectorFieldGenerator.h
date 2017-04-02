//
//  VectorFieldGenerator.h
//  curiositas2017
//
//  Created by FatDazz_mac on 02/04/2017.
//
//
#pragma once

class VectorFieldGenerator : public ofThread {
    
public:
    ofImage finalPixelisation;
    
    void setup() {
        pixelisation.allocate(600, 600, OF_IMAGE_COLOR);
        pixelisation.setUseTexture(false);
        
        finalPixelisation.allocate(600 / unit, 600 / unit, OF_IMAGE_GRAYSCALE);
        finalPixelisation.setUseTexture(false);
        
        gray.allocate(600 / unit, 600 / unit, OF_IMAGE_GRAYSCALE);
        gray.setUseTexture(false);
        
        sobelX.allocate(600 / unit, 600 / unit, OF_IMAGE_GRAYSCALE);
        sobelX.setUseTexture(false);
        
        sobelY.allocate(600 / unit, 600 / unit, OF_IMAGE_GRAYSCALE);
        sobelY.setUseTexture(false);
        
        /*gradientVectorField.resize(600 / unit);
         for (auto& v : gradientVectorField) {
         v.resize(600 / unit);
         }*/
        
        gradientVectorField.resize(600/unit,vector<ofVec2f>(600/unit));
        
        tmp.resize(600/unit);
    }
    
    
    
    
    ofThreadChannel<cv::Mat> pix;
    ofThreadChannel<std::vector<std::vector<ofVec2f>>> vecField;
    
private:
    void threadedFunction() {
        cv::Mat m;
        pix.receive(m);
        ofxCv::toOf(m, pixelisation);
        pixelisation.mirror(false, true);
        
        for (int i = 0; i < pixelisation.getWidth(); i+= unit) {
            for (int j = 0; j < pixelisation.getHeight(); j+= unit) {
                unsigned int r = 0;
                unsigned int g = 0;
                unsigned int b = 0;
                int width = unit;
                int height = unit;
                for (int k = 0; k < width; k++) {
                    for (int l = 0; l < height; l++) {
                        ofColor col = pixelisation.getColor(i+k, j+l);
                        r += col.r;
                        g += col.g;
                        b += col.b;
                    }
                }
                r /= (width * height);
                g /= (width * height);
                b /= (width * height);
                finalPixelisation.setColor(i/unit, j/unit, ofColor(r, g, b));
            }
        }
        finalPixelisation.update();
        ofxCv::copy(finalPixelisation, gray);
        //ofxCv::convertColor(finalPixelisation, gray, CV_RGB2GRAY);
        //=================================================================================<<<<<<<
        ofxCv::imitate(sobelX, gray, CV_8SC1);
        cv::Mat srcMat = ofxCv::toCv(gray), dstMat = ofxCv::toCv(sobelX);
        
        //ofxCv::imitate(inter_x, gray, CV_32F);
        cv::Sobel(srcMat, inter_x, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT);
        inter_x.convertTo(inter_x, CV_32S);
        // image en X
        cv::convertScaleAbs(inter_x, dstMat);
        //=================================================================================<<<<<<<
        ofxCv::imitate(sobelY, gray, CV_8SC1);
        
        cv::Mat dstMat_ = ofxCv::toCv(sobelY);
        
        //ofxCv::imitate(inter_y, gray, CV_32F);
        cv::Sobel(srcMat, inter_y, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT);
        inter_y.convertTo(inter_y, CV_32S);
        // image en Y
        cv::convertScaleAbs(inter_y, dstMat_);
        //=================================================================================<<<<<<<
        cv::addWeighted(dstMat, 0.5, dstMat_, 0.5, 0, dstMat);
        
        //gradientVectorField.clear();
        for (unsigned int i=0; i<finalPixelisation.getWidth(); i++) {
            //std::vector<ofVec2f> vectorJ;
            //tmp.clear();
            for (unsigned int j=0; j<finalPixelisation.getHeight(); j++) {
                float x = inter_x.at<int>(j, i)/(float)1020;
                float y = inter_y.at<int>(j, i)/(float)1020;
                gradientVectorField[i][j] = ofVec2f(x,y);
                //tmp.push_back(ofVec2f(x, y));
            }
            //gradientVectorField.push_back(tmp);
        }
        gray.update();
        sobelX.update();
        sobelY.update();
        finalPixelisation.update();
    }
    ofImage pixelisation;
    
    
    ofImage gray;
    ofImage sobelX;
    ofImage sobelY;
    ofImage gradient;
    
    cv::Mat src;
    cv::Mat inter_x,inter_y;
    cv::Mat mag;
    cv::Mat angle;
    
    cv::Mat grad;
    
    int unit = 20;
public:
    std::vector<std::vector<ofVec2f>> gradientVectorField;
    std::vector<ofVec2f> tmp;
    ofImage t;
    
};
