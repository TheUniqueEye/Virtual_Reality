// MAT201B Fall 2015
// FINAL PROJECT: The Inner
// New Author: Jing Yan
// Original Author(s): Karl Yerkes
//
// visualize the amplitude envelope of a beating heart
// #include "allocore/io/al_App.hpp"
#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "finalCommon.hpp"

using namespace al;
using namespace std;
// using namespace gam;

struct MyApp : OmniStereoGraphicsRenderer { // XXX
  float visualLoudnessMeasure;
  float tempVLM;

  Mesh m;
  Vec3f destination[NumFragment*NumFragment]; // XXX
  HSV hsv[NumFragment*NumFragment]; // XXX
  cuttlebone::Taker<State> taker;  // XXX
  State state;                     // XXX

  MyApp() {

    addSphere(m);
    initWindow();
    nav().pos(1, 0, 15); /// 设定一个初始的观看的位置
    background(HSV(0.3, 0.7, 0.3)); /// 背景色的设定
  }

  virtual void onAnimate(double dt) {
    taker.get(state);  // XXX
    for (int i = 0; i < NumFragment*NumFragment; i++) {
      destination[i] = state.destination[i];            // XXX
      hsv[i] = state.hsv[i];            // XXX
    }
    pose = state.pose;
  }


  virtual void onDraw(Graphics& g) {

    for(int i = 0; i< (NumFragment*NumFragment) ; i++){
        g.pushMatrix();
        g.scale(0.3);
        g.translate(destination[i]);
        g.color(hsv[i]);
        g.draw(m);
        g.popMatrix();
    }


    // tempVLM = visualLoudnessMeasure;


    // for(int i = 0; i< (NumFragment*NumFragment) ; i++){
    //   float x = rnd::uniformS();
    //   float y = rnd::uniformS();
    //   g.pushMatrix();
    //   g.scale(0.3);
    //   Vec3f center = Vec3f((NumFragment-1.f)/2.f,(NumFragment-1.f)/2.f,5*visualLoudnessMeasure*(i%4-1.5));
    //   // cout<< center <<endl;
    //   // g.translate(Vec3f(x, y, 0) * visualLoudnessMeasure + Vec3f(i/3,i%3,0)*3);
    //   g.translate((Vec3f(i / NumFragment,i % NumFragment,5*visualLoudnessMeasure*(i%4-1.5)) - center)*3*(visualLoudnessMeasure+1) + center*3);
    //   g.color(HSV(visualLoudnessMeasure*3, .6, visualLoudnessMeasure*3));
    //   g.draw(m);
    //   g.popMatrix();
    // }

  }
};

int main() {
  MyApp app;
  app.taker.start(); // XXX
  app.start();
}
