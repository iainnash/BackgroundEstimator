//
//  main.cpp
//  BackgroundEstimator
//
//  Created by Iain Nash on 10/6/15.
//  Copyright © 2015 Iain Nash. All rights reserved.
//

#include "FreeImage.h"
#include <iostream>
#include "fi2mat.hpp"
#include <opencv2/opencv.hpp>
#include "bgthreads.hpp"

using namespace cv;

void run_without_threads(const char* test, const char* newbgf);
int main(int argc, const char * argv[]) {
  const char* test = argv[1];
  const char* newbgf = argv[2];
  
  //run_without_threads(test, newbgf);
  
  run_with_threads(test, newbgf, 4);
  
  return 0;
}


void run_without_threads(const char* test, const char* newbgf) {
  FIMULTIBITMAP* image = FreeImage_OpenMultiBitmap(FreeImage_GetFileType(test, 0), test, false, true, true, GIF_PLAYBACK);
  
  int slides = FreeImage_GetPageCount(image);
  
  Ptr<BackgroundSubtractor> pMOG2 = new BackgroundSubtractorMOG2(slides, 140.0, false);
  Mat fgMask;
  
  for (int i = 0; i < slides; i++) {
    FIBITMAP *dib = FreeImage_LockPage(image, i);
    Mat cv_img;
    Mat cv_img_fin;
    FI2MAT(dib, cv_img);
    cvtColor(cv_img, cv_img_fin, CV_BGRA2BGR);
    pMOG2->operator()(cv_img_fin, fgMask);
    FreeImage_UnlockPage(image, dib, false);
  }
  
  FIBITMAP *lastframe = FreeImage_LockPage(image, slides - 1);
  Mat foregroundImage_raw;
  Mat foregroundImage;
  FI2MAT(lastframe, foregroundImage_raw);
  cvtColor(foregroundImage_raw, foregroundImage, CV_BGRA2BGR);
  FreeImage_UnlockPage(image, lastframe, false);
  
  FreeImage_CloseMultiBitmap(image);
  
  Mat backgroundImage;
  
  pMOG2->getBackgroundImage(backgroundImage);
  
  Mat newBg = imread(newbgf);
  
  Mat newFGMask;
  cvtColor(fgMask, newFGMask, CV_GRAY2BGR);
  
  for (int j = 0; j < newFGMask.rows; j++) {
    for (int i = 0; i < newFGMask.cols; i++) {
      if (newFGMask.at<Vec3b>(j, i)[0] != 0) { // black = bg
        Vec3b a = foregroundImage.at<Vec3b>(j, i);
        newFGMask.at<Vec3b>(j, i) = a;
      } else {
        Vec3b a = newBg.at<Vec3b>(j, i);
        newFGMask.at<Vec3b>(j, i) = a;
      }
    }
  }
  
  imshow("asdf", newFGMask );
  
  waitKey(0);

}
