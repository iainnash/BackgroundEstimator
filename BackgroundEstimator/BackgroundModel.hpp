//
//  BackgroundModel.hpp
//  BackgroundEstimator
//
//  Created by Iain Nash on 10/7/15.
//  Copyright © 2015 Iain Nash. All rights reserved.
//

#ifndef BackgroundModel_hpp
#define BackgroundModel_hpp

#include <cmath>
#include <opencv2/opencv.hpp>
#include "PixelVal.h"

#define square(val) std::pow(val, 2)

using namespace cv;

class BackgroundModel {
public:
  PixelValWrap **pixels;
  int width;
  int height;
  int frameCount;
  
  BackgroundModel(int width, int height)
      : width(width), height(height), frameCount(0) {
        
    pixels = new PixelValWrap*[height];
    for (int i = 0; i < height; i++) {
      pixels[i] = new PixelValWrap[width];
    }
  }
  void updateTrainingVal(int x, int y, Vec3b pixel) {
    // increment frame count (0 -> 1), cannot be 0.
    frameCount++;
    
    PixelVal fVal = pixels[y][x].cur;
    // update lastVal
    pixels[y][x].last = pixels[y][x].cur;
    
    // calculate color values
    for (int i = 0; i < 3; i++) {
      int atPixel = pixel[i];
      // Update the average (color)
      fVal.expectedPixelValue[i] +=
        (atPixel - fVal.expectedPixelValue[i]) / frameCount;
      // Update the standard deviation (color)
      fVal.colorStandardDeviation[i] =
        ((frameCount - 2.0) / (frameCount - 1.0) * fVal.colorStandardDeviation[i]) +
        (square(atPixel - fVal.expectedPixelValue[i]) / frameCount);
    }
  
    
    // non-color updates
    
    // Brightness Distortion
    double brightnessTop = 0;
    double brightnessBottom = 0;
    for (int i = 0; i < 3; i++) {
      brightnessTop += (pixel[i] * fVal.expectedPixelValue[i]) /
          (fVal.colorStandardDeviation[i] * fVal.colorStandardDeviation[i]);
      brightnessBottom += square(fVal.expectedPixelValue[i] / fVal.colorStandardDeviation[i]);
    }
    fVal.brightnessVaration = brightnessTop / brightnessBottom;
    
    // Chromaticity Distortion
    double chromaticityDistortionParts = 0;
    for (int i = 0; i < 3; i++) {
      chromaticityDistortionParts += square((pixel[i]*fVal.brightnessVaration*fVal.expectedPixelValue[i]) /
          fVal.colorStandardDeviation[i]);
    }
    fVal.distortion = sqrt(chromaticityDistortionParts);
    
    pixels[y][x].cur = fVal;

  }
  void train(Vec3b **image) {
    for (int yi = 0; yi < height; yi++) {
      for (int xi = 0; xi < width; xi++) {
        updateTrainingVal(xi, yi, image[yi][xi]);
      }
    }
  }
  
  bool** getImageMask(Vec3b **image) {
    if (frameCount == 0) {
      throw new std::runtime_error("Recognizer not trained! ");
    }
    bool** mask = new bool*[height];
    for (int iy = 0; iy < height; iy++) {
      mask[iy] = new bool[width];
      for (int ix = 0; ix < width; ix++) {
        mask[iy][ix] = false;
      }
    }
    // important!
    train(image);
    for (int iy = 0; iy < height; iy++) {
      for (int ix = 0; ix < width; ix++) {
        std::cout << pixels[iy][ix].last.distortion << " : " << pixels[iy][ix].cur.distortion << std::endl;
        if (std::abs(pixels[iy][ix].last.distortion - pixels[iy][ix].cur.distortion) > 46000) {
          mask[iy][ix] = true;
        }
      }
    }
    return mask;
  }
  
};


#endif /* BackgroundModel_hpp */
