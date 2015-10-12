//
//  PixelVal.h
//  BackgroundEstimator
//
//  Created by Iain Nash on 10/11/15.
//  Copyright © 2015 Iain Nash. All rights reserved.
//

#ifndef PixelVal_h
#define PixelVal_h

struct PixelVal {
  double expectedPixelValue[3];
  double colorStandardDeviation[3];
  double brightnessVaration;
  double distortion;
};

/*
 struct PixelVal {
 double E[3];
 double B[3];
 
 double negS;
 double negA;
 double negB;
 };*/

struct PixelValWrap {
  PixelVal last;
  PixelVal cur;
};


#endif /* PixelVal_h */
