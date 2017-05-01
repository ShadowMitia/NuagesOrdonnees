//
//  Constant.h
//  curiositas2017
//
//  Created by FatDazz_mac on 02/04/2017.
//
//
#pragma once
//Mettre à 1 pour avoir la Kinect 2, 0 sinon
#define USE_KINECT 0

constexpr int win_width = 1920/2;
constexpr int win_height = 1080/2;
constexpr int cam_width = 1920/2;
constexpr int cam_height = 1080/2;
constexpr int div_width = 20;
constexpr int div_height = 20;
constexpr int divGrad_width = 8;
constexpr int divGrad_height = 8;
constexpr int nbThreadBoids = 1;
constexpr int nbBoids = (win_width/div_width) * (win_height/div_height);
constexpr int image_width = cam_width / 2;
constexpr int image_height = cam_height / 2;
