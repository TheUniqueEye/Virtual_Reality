// MAT201B Fall 2015
// FINAL PROJECT: The Inner
// New Author: Jing Yan
// Original Author(s): Karl Yerkes
//
// visualize the amplitude envelope of a beating heart

#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"
#include "Cuttlebone/Cuttlebone.hpp"  // XXX
#include "alloutil/al_Simulator.hpp"  // XXX
#include "finalCommon.hpp"  // XXX

using namespace al;
using namespace std;
using namespace gam;

struct MyApp : App, InterfaceServerClient { // XXX
  Mesh m;
  float visualLoudnessMeasure;
  float tempVLM;
  Vec3f destination[NumFragment*NumFragment]; // XXX
  HSV hsv[NumFragment*NumFragment]; // XXX
  SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> samplePlayer;

  cuttlebone::Maker<State> maker;  // XXX
  State state;                     // XXX

  MyApp()
    : maker(Simulator::defaultBroadcastIP()),                        // XXX
      InterfaceServerClient(Simulator::defaultInterfaceServerIP()){

    addSphere(m);
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

    InterfaceServerClient::setNav(nav());    // XXX
    InterfaceServerClient::setLens(lens());  // XXX
  }

  virtual void onAnimate(double dt) {

    while (InterfaceServerClient::oscRecv().recv())
      ;  // XXX

      for (int i = 0; i < NumFragment*NumFragment; i++) {
        Vec3f center = Vec3f((NumFragment-1.f)/2.f,(NumFragment-1.f)/2.f,5*visualLoudnessMeasure*(i%4-1.5));
        cout<<center[0]<<center[1]<<center[2]<<endl<<endl;
        destination[i] = (Vec3f(i/NumFragment,i%NumFragment,5*visualLoudnessMeasure*(i%4-1.5)) - center)*3*(visualLoudnessMeasure+1) + center*3;
        hsv[i] = HSV(visualLoudnessMeasure*3, .6, visualLoudnessMeasure*3);

        state.destination[i] = destination[i];            // XXX
        state.hsv[i] = hsv[i];            // XXX
      }
      state.pose = nav();
      maker.set(state);  // XXX
  }

  virtual void onDraw(Graphics& g) {

    for(int i = 0; i< (NumFragment*NumFragment) ; i++){
        float x = rnd::uniformS();
        float y = rnd::uniformS();
        g.pushMatrix();
        g.scale(0.3);
        g.translate(destination[i]);
        g.color(hsv[i]);
        g.draw(m);
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
      g.draw(m);
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

int main() {
  MyApp app;
  app.maker.start();  // XXX
  app.start();
}
