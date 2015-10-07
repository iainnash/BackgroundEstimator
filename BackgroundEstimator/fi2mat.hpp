//
//  fi2mat.hpp
//  BackgroundEstimator
//
//  Created by Iain Nash on 10/7/15.
//  Copyright © 2015 Iain Nash. All rights reserved.
//

#ifndef fi2mat_hpp
#define fi2mat_hpp

#include <FreeImage.h>
#include <opencv2/opencv.hpp>

using namespace cv;

void FI2MAT(FIBITMAP* src, Mat& dst);

#endif /* fi2mat_hpp */
