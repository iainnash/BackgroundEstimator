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
#include "BackgroundModel.hpp"

using namespace cv;
using namespace std;

void run_without_threads(const char* test, const char* newbgf);
int main(int argc, const char * argv[]) {
  const char* test = argv[1];
  const char* newbgf = argv[2];
  
  run_without_threads(test, newbgf);
  
  //run_with_threads(test, newbgf, 4);
  
  return 0;
}

Vec3b **FIBITMAP2VEC(FIBITMAP *dib) {
  int width = FreeImage_GetWidth(dib);
  int height = FreeImage_GetHeight(dib);
  
  int bytesapp = FreeImage_GetLine(dib) / width;
  Vec3b **imgout = new Vec3b*[height];
  
  for (unsigned y = 0; y < height; y++) {
    imgout[y] = new Vec3b[width];
    
    BYTE *bits = FreeImage_GetScanLine(dib, y);
    for (unsigned x = 0; x < width; x++) {
      imgout[y][x] = cv::Vec3b(bits[FI_RGBA_RED], bits[FI_RGBA_GREEN], bits[FI_RGBA_BLUE]);
      bits += bytesapp;
    }
  }
  return imgout;
}


void run_without_threads(const char* test, const char* newbgf) {
  FIMULTIBITMAP* image = FreeImage_OpenMultiBitmap(FreeImage_GetFileType(test, 0), test, false, true, true, GIF_PLAYBACK);
  
  int slides = FreeImage_GetPageCount(image);
  
  //Ptr<BackgroundSubtractor> pMOG2 = new BackgroundSubtractorMOG2(slides, 16.0, false);
  
  Mat fgMask;
  FIBITMAP *dib = FreeImage_LockPage(image, 0);
  int width = FreeImage_GetWidth(dib);
  int height = FreeImage_GetHeight(dib);
  BackgroundModel model(width, height);
  FreeImage_UnlockPage(image, dib, false);
  
  cout << "starting analysis" << endl;
  
  for (int i = 0; i < slides; i++) {
    FIBITMAP *dib = FreeImage_LockPage(image, i);
    
    Vec3b** imgout = FIBITMAP2VEC(dib);
    
    cout << "reading in frame " << i << endl;
    
    //pMOG2->operator()(cv_img_fin, fgMask);
    model.train(imgout);
    
    FreeImage_UnlockPage(image, dib, false);
  }
  
  FIBITMAP *lastframe = FreeImage_LockPage(image, slides / 2);
  Mat foregroundImage_raw;
  Mat foregroundImage;
  FI2MAT(lastframe, foregroundImage_raw);
  cvtColor(foregroundImage_raw, foregroundImage, CV_BGRA2BGR);
  
  cout << "Getting mask" << endl;
  
  bool **mask = model.getImageMask(FIBITMAP2VEC(lastframe));
  
  FreeImage_UnlockPage(image, lastframe, false);
  
  FreeImage_CloseMultiBitmap(image);
  
  Mat backgroundImage;

  Mat newBg = imread(newbgf);
  
  cout << "Has mask" << endl;
  
  for (int j = 0; j < newBg.rows; j++) {
    for (int i = 0; i < newBg.cols; i++) {
      if (mask[j][i]) {
        //cout << "replacing pixel at " << j << "," << i << endl;
        newBg.at<Vec3b>(j, i) = foregroundImage.at<Vec3b>(j, i);
      }
    }
  }
  
  imshow("replaced", newBg);
  
  waitKey(0);

}
