//
//  bgthreads.cpp
//  BackgroundEstimator
//
//  Created by Iain Nash on 10/7/15.
//  Copyright © 2015 Iain Nash. All rights reserved.
//

#include "bgthreads.hpp"
#include <iostream>
#include "fi2mat.hpp"
#include "FreeImage.h"
#include <boost/thread.hpp>
#include <stdio.h>
#include <time.h>
#include <opencv2/opencv.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace cv;
using namespace std;


class FrameImageWorker {
public:
  void operator()() {
    for (std::vector<Mat>::iterator img_i = images->begin(); img_i != images->end(); ++img_i) {
      bg->operator()(*img_i, fg);
    }
  }
  
  cv::Rect rect;
  cv::BackgroundSubtractorMOG2 *bg;
  cv::Mat fg;
  std::vector<Mat> *images;
  
  FrameImageWorker(CvRect rect, int count, std::vector<Mat> *images) {
    this->rect = rect;
    this->images = images;
    this->bg = new cv::BackgroundSubtractorMOG2(count, 23);
  }
  
  
};

void run_with_threads(const char *giffile, const char *bgfile, const int numThreads)
{
  
  FIMULTIBITMAP* image = FreeImage_OpenMultiBitmap(FreeImage_GetFileType(giffile, 0), giffile, false, true, true, GIF_PLAYBACK);

  
  std::vector<Mat> frames;
  int slides = FreeImage_GetPageCount(image);
  for (int i = 0; i < slides; i++) {
    FIBITMAP *dib = FreeImage_LockPage(image, i);
    Mat cv_img;
    Mat cv_img_fin;
    FI2MAT(dib, cv_img);
    cvtColor(cv_img, cv_img_fin, CV_BGRA2BGR);
    cv_img.copyTo(cv_img_fin);
    frames.push_back(cv_img_fin);
    FreeImage_UnlockPage(image, dib, false);
    cout << "Read file " << i << endl;
  }
  
  int perRows = frames[0].rows / numThreads;
  int perCols = frames[0].cols / numThreads;
  
  std::vector<FrameImageWorker*> workers;
  boost::thread_group slide_threads;
  
  for (int yi = 0; yi <= frames[0].rows - perRows; yi += perRows) {
    int ym = min(yi + perRows, frames[0].rows);
    for (int xi = 0; xi <= frames[0].cols - perCols; xi += perCols) {
      int xm = min(xi + perCols, frames[0].cols);
      
      std::vector<Mat> framesSq(slides);
      Rect cropped(xi, yi, xm - xi, ym - yi);

      for (int i = 0; i < frames.size(); i++) {
        framesSq.push_back(frames.at(i)(cropped));
      }
      FrameImageWorker *worker = new FrameImageWorker(cropped, slides, &framesSq);
      workers.push_back(worker);
      slide_threads.add_thread(new boost::thread(boost::ref(*worker)));
    }
  }

  clock_t st, et;
  st = clock();
  slide_threads.join_all();
  et = clock();
  std::cout << "Took " << et - st << "ms";
  

}
