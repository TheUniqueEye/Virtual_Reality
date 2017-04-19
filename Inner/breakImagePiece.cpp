// MAT201B
// Fall 2015
// Oringal: textured triangles
//
// Shows how to: cut image into pieces
//

#include "allocore/io/al_App.hpp"
#include <cassert>  // gets you assert()

using namespace al;
using namespace std;

#define NumOfRow 8
#define NumOfCol 8

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(".");
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

void createRecMesh(Mesh& mesh, Vec3f center, float size , float ratio , int i, int j){
  mesh.primitive(Graphics::TRIANGLES);
  float x = center[0]; // center
  float y = center[1];
  float z = center[2];

  mesh.vertex(x - size/2 * ratio , y - size/2, z); // mesh vertex
  mesh.vertex(x + size/2 * ratio , y - size/2, z);
  mesh.vertex(x - size/2 * ratio , y + size/2, z);
  mesh.vertex(x + size/2 * ratio , y - size/2, z);
  mesh.vertex(x - size/2 * ratio , y + size/2, z);
  mesh.vertex(x + size/2 * ratio , y + size/2, z);

  mesh.texCoord((float)i/NumOfCol, (float)j/NumOfRow); // texture
  mesh.texCoord((float)(i+1)/NumOfCol, (float)j/NumOfRow);
  mesh.texCoord((float)i/NumOfCol, (float)(j+1)/NumOfRow);
  mesh.texCoord((float)(i+1)/NumOfCol, (float)j/NumOfRow);
  mesh.texCoord((float)i/NumOfCol, (float)(j+1)/NumOfRow);
  mesh.texCoord((float)(i+1)/NumOfCol, (float)(j+1)/NumOfRow);

}

struct AlloApp : App {
  Texture texture;
  Mesh fragments[NumOfCol][NumOfRow];

  AlloApp() {
    Image image;
    const char* fileName = "chart.png";
    if (!image.load(fullPathOrDie(fileName))) {
      cerr << "failed to load " << fileName << endl;
      exit(1);
    }
    texture.allocate(image.array()); // allocate image - array - texture

    float ratio = image.width()/ (float)image.height();
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
          Vec3f center = Vec3f(i*ratio,j, 0.3* sin((float)(pow(i,j))/NumOfCol));
          float size = abs(sin((float)(i*j)/NumOfCol));
          createRecMesh(fragments[i][j] , center , size, ratio ,i , j);
      }
    }
    nav().pos(1, 0, 20);
    initWindow();
  }

  double t = 0;
  virtual void onAnimate(double dt) {
    t += dt;
  }

  virtual void onDraw(Graphics& g, const Viewpoint& v) {
    texture.bind();
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
          g.draw(fragments[i][j]);
      }
    }
    texture.unbind();
  }
};

int main() { AlloApp().start(); }
