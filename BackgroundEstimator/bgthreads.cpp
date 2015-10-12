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
#include "BackgroundModel.hpp"
#include "Timer.hpp"
#include <boost/multi_array.hpp>

struct colourtuple {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

typedef boost::multi_array<colourtuple, 3> pixels_type;


using namespace cv;
using namespace std;

class FrameImageWorker {
public:
  void operator()() {
    Timer tmr;
    for (int i = 0; i < nframes; i++) {
      Vec3b **imgRaw = new Vec3b*[rect.height];
      for (int yi = 0; yi < rect.height; yi++) {
        imgRaw[yi] = new Vec3b[rect.width];
        for (int xi = 0; xi < rect.width; xi++) {
          colourtuple colors = imgs[i][yi][xi];
          imgRaw[i][xi] = Vec3b(colors.r, colors.g, colors.b);
        }
        bg->train(imgRaw);
        // cleanup memory
        for (int yi = 0; yi < rect.height; yi++) {
          delete [] imgRaw[yi];
        }
        delete [] imgRaw;
      }
      
    }
    double t = tmr.elapsed();
    cout << "Grid @ (" << rect.x << "," << rect.y << ") ran in " << t << endl;
  }
  
  Rect rect;
  BackgroundModel *bg;
  Mat fg;
  pixels_type imgs;
  int nframes;
  
  FrameImageWorker(CvRect rect, int count, pixels_type imgs, int nframes)
      : rect(rect), nframes(nframes), imgs(imgs) {
    bg = new BackgroundModel(rect.width, rect.height);
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
      
      int width = xm - xi;
      int height = ym - yi;
      Rect rect = Rect(xi, yi, width, height);

      boost::array<pixels_type::index, 3> shape = {{ static_cast<long>(frames.size()), height, width }};
      pixels_type img(shape);
      
      for (int i = 0; i < frames.size(); i++) {
        for (int yii = yi; yii < ym; yii++) {
          for (int xii = xi; xii < xm; xii++) {
            Vec3b newFrame = frames[i].at<Vec3b>(yii, xii);
            colourtuple colorTuple;
            colorTuple.r = newFrame[0];
            colorTuple.g = newFrame[1];
            colorTuple.b = newFrame[2];
            img[i][yii][xii] = colorTuple;
          }
        }
      }
      FrameImageWorker *worker = new FrameImageWorker(rect, slides, img, std::move(slides));
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
