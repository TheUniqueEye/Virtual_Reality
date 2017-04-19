// MAT201B
// Fall 2015
// Author(s): Karl Yerkes
//
// Shows how to visualize the amplitude envelope of a beating heart
//
#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"

using namespace al;
using namespace std;
using namespace gam;

#define NumFragment 6

struct MyApp : App {
  Mesh m[NumFragment*NumFragment];
  float visualLoudnessMeasure;
  float tempVLM;
  SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> samplePlayer;

  MyApp() {
    for(int i = 0; i< NumFragment * NumFragment; i++){
      addSphere(m[i]);
    }
    SearchPaths searchPaths;
    searchPaths.addSearchPath(".");
    string filePath = searchPaths.find("heart_beat.wav").filepath();
    cout << filePath << endl;
    samplePlayer.load(filePath.c_str());
    initAudio();
    gam::Sync::master().spu(audioIO().fps()); ///
    initWindow();
    nav().pos(1, 0, 15); /// 设定一个初始的观看的位置
    background(0.3); /// 背景色的设定
  }

  virtual void onDraw(Graphics& g) {

    for(int i = 0; i< (NumFragment*NumFragment) ; i++){
        float x = rnd::uniformS();
        float y = rnd::uniformS();
        g.pushMatrix();
        g.scale(0.3);
        Vec3f center = Vec3f((NumFragment-1.f)/2.f,(NumFragment-1.f)/2.f,5*visualLoudnessMeasure*(i%4-1.5));
        // cout<<center[0]<<center[1]<<center[2]<<endl<<endl;
        // g.translate(Vec3f(x, y, 0) * visualLoudnessMeasure + Vec3f(i/3,i%3,0)*3);
        g.translate((Vec3f(i/NumFragment,i%NumFragment,5*visualLoudnessMeasure*(i%4-1.5)) - center)*3*(visualLoudnessMeasure+1) + center*3);
        g.color(HSV(visualLoudnessMeasure*3, .6, visualLoudnessMeasure*3));
        g.draw(m[i]);
        g.popMatrix();
    }

    tempVLM = visualLoudnessMeasure;


    for(int i = 0; i< (NumFragment*NumFragment) ; i++){
      float x = rnd::uniformS();
      float y = rnd::uniformS();
      g.pushMatrix();
      g.scale(0.3);
      Vec3f center = Vec3f((NumFragment-1.f)/2.f,(NumFragment-1.f)/2.f,5*visualLoudnessMeasure*(i%4-1.5));
      // cout<< center <<endl;
      // g.translate(Vec3f(x, y, 0) * visualLoudnessMeasure + Vec3f(i/3,i%3,0)*3);
      g.translate((Vec3f(i / NumFragment,i % NumFragment,5*visualLoudnessMeasure*(i%4-1.5)) - center)*3*(visualLoudnessMeasure+1) + center*3);
      g.color(HSV(visualLoudnessMeasure*3, .6, visualLoudnessMeasure*3));
      g.draw(m[i]);
      g.popMatrix();
    }

  }

  virtual void onSound(AudioIOData& io) {
    visualLoudnessMeasure = 0;
    while (io()) {
      float s = samplePlayer();
      if (abs(s) > visualLoudnessMeasure) visualLoudnessMeasure = abs(s);
      io.out(0) = s;
      io.out(1) = s;
    }
    visualLoudnessMeasure /= 5;
  }
};

int main() { MyApp().start(); }
