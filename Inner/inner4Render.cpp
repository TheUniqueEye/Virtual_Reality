// MAT201B
// Fall 2015
// Author(s): Jing Yan / EYE
// [INNER] Version 4: heartBeatSource added
//
// press key 1 to reconstruct the image;
// press key 2 to expand and loosen the image

#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"
#include <cassert>
#include "alloutil/al_OmniStereoGraphicsRenderer.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "innerCommon.hpp"

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

struct MyApp : OmniStereoGraphicsRenderer{  // XXX
  Texture texture;
  Fragment fragmentData[NumOfCol][NumOfRow];

  cuttlebone::Taker<State> taker;  // XXX
  State state;                     // XXX

  MyApp(){

    Image image;
    const char* fileName = "7.jpg";
    if (!image.load(fullPathOrDie(fileName))) {
      cerr << "failed to load " << fileName << endl;
      exit(1);
    }

    //texture = state.texture;            // XXX
    texture.allocate(image.array());

    initWindow();
    nav().pos(0,0,25);
    // background(HSV(0.3, 0.7, 0.3));
    omni().clearColor() = Color(0.3);

    float ratio = image.width()/ (float)image.height();
    float scale = 0.1;
    float density = 2;
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){

        Vec3f imageCenter = Vec3f((NumOfRow-1)/2*scale,(NumOfCol-1)/2*scale,0);
        Vec3f center = (Vec3f(i*ratio*scale,j*scale,0) - imageCenter)*density + imageCenter;

        float x = rnd::uniformS();
        float size;

        size = 1.3*(x*0.3+0.7)*scale;

        fragmentData[i][j] = Fragment(center,size,ratio,i,j);
        fragmentData[i][j].createRecMesh();

      }
    }

    cout << fragmentData[10][10].texCoord2s().size() << endl;
  }

  virtual void onAnimate(double dt) {
    taker.get(state);  // XXX

    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
        if (i == 0 && j ==0)
          cout << fragmentData[i][j].pos() << endl;
        fragmentData[i][j].set(state.fragmentPose[i][j]);
      }
    }
    pose = state.pose;
  }

  virtual void onDraw(Graphics& g) {
    shader().uniform("lighting", 0.0);
    shader().uniform("texture", 1.0);
    texture.bind();
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
        fragmentData[i][j].onDraw(g);
      }
    }
    texture.unbind();

    //texture.quad(g);
/*
    Mesh mm;
    mm.vertex(0,0,0);
    mm.color(1,1,1);
    g.pointSize(50);
    mm.primitive(Graphics::POINTS);
    g.draw(mm);
*/
  }
};

  int main() {
    MyApp app;
    app.taker.start(); // XXX
    app.start();
  }
