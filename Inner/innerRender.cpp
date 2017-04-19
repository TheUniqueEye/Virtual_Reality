// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// The INNER
//
// < This is the Render, which loads images, creates fragments and uses
// the data(pose,time) sent from simulator to translate the pose of each frament
// and each image. >

#include "allocore/io/al_App.hpp"
#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "innerCommon.hpp"
#include "Gamma/SamplePlayer.h"
#include <cassert>

using namespace al;
using namespace std;
using namespace gam;

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(".");
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

struct MyApp : OmniStereoGraphicsRenderer{
  int fileCount;
  FileList file;
  Texture* texture;
  float* ratio;
  float scale;
  float density;
  Fragment *fragments;
  FragmentedImage *fragmentedImage;

  cuttlebone::Taker<State, 9000> taker;
  State state;

  double t2,t3;

  MyApp(){
    scale = 0.1;
    density = 1.5;

    SearchPaths searchPaths;
    searchPaths.addSearchPath("student/jing.yan/final");
    file = searchPaths.glob("(.*)\\.(png|jpg|jpeg|bmp)");
    cout << "Found " << file.count() << " files:" << endl;
    file.print();

    fileCount = file.count();
    texture = new Texture[fileCount];
    ratio = new float[fileCount];
    fragmentedImage = new FragmentedImage[fileCount];
    for (int i = 0; i < fileCount; i++) {
      Image image;
      if (!image.load(file[i].filepath())) {
        cout << "ERROR" << endl;
        exit(1);
      }
      texture[i].allocate(image.array());
      ratio[i] = (float) image.width() / image.height();
      fragmentedImage[i] = FragmentedImage(texture[i], ratio[i], density, scale, 30, 30);
      fragmentedImage[i].createImage();
    }
    initWindow();
    nav().pos(0,0.5,10);
    omni().clearColor() = Color(0);
  }

  virtual void onAnimate(double dt){
    taker.get(state);
    t2 = state.t2;
    t3 = state.t3;

    // get the pose of each fragmented Image and each fragment from
    // simulator.cpp to render.cpp
    for(int i = 0 ; i<fileCount ; i++){
      for(int col = 0 ; col<fragmentedImage[i].COLUMNS ; col++){
        for(int rol = 0 ; rol<fragmentedImage[i].ROWS ; rol++){
          fragmentedImage[i].fragments[col*fragmentedImage[i].ROWS+rol].set(state.fragmentPose[i][col][rol]);
        }
      }
      fragmentedImage[i].set(state.fragmentedImagePose[i]);
    }

    // get the pose of camera to render.cpp to synchronize the
    // perspective of simulator and render
    pose = state.pose;
  }

  virtual void onDraw(Graphics& g) {
    shader().uniform("lighting", 0.0);
    shader().uniform("texture", 1.0);

    if (t3 < 66) g.translate(0,0,3*t3);
    else g.translate(0,0,3*66 - (t2-132)*20);

    for(int i = 0 ; i<fileCount ; i++){
      fragmentedImage[i].onDraw(g);
    }
  }
};

int main() {
  MyApp app;
  app.taker.start();
  app.start();
}
