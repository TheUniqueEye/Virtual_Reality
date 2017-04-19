#include "allocore/io/al_App.hpp"

using namespace al;
using namespace std;

#define FragmentWidth 120
#define FragmentHeight 90
#define NumFragment 8
#define ImageActualHeight 0.8
#define ImageActualWidth 0.6

Image loadImageFile(const char* fileSearchPath , const char* imageFilename ){
  SearchPaths searchPaths;
  searchPaths.addSearchPath(fileSearchPath);
  string imageFullPath = searchPaths.find(imageFilename).filepath();
  if (imageFullPath == "") {
    cerr << "FAIL: your image " << imageFilename
         << " was not found in the given file path (" << fileSearchPath
         << endl;
    exit(1);
  }
  cout << "Full path to your image file is " << imageFullPath << endl;

  Image image;
  bool loadResult = image.load(imageFullPath);
  if (!loadResult) {
    cerr << "FAIL: your image " << imageFullPath << " did not load." << endl;
    exit(1);
  }
  return image;
}

struct Fragment {
  //RGB rgbArray[FragmentWidth][FragmentHeight];
  Texture texture;
  Vec3f position;
  Vec3f orientation;
};

void makeMeshByFragment(Mesh& mesh, Fragment fragment){
  // Mesh mesh;
  mesh.primitive(Graphics::POINTS); //Karl: how to make the points denser?

  for (int row = 0; row < FragmentHeight; ++row) {
    for (int col = 0; col < FragmentWidth; ++col) {
      mesh.vertex(
          fragment.position +
          Vec3f((float)ImageActualHeight/NumFragment*(row-FragmentHeight/2)/FragmentHeight,
                (float)ImageActualWidth/NumFragment*(col-FragmentWidth/2)/FragmentWidth,
                0)
      );
      mesh.color(fragment.rgbArray[col][row]);
    }
  }
  // return mesh;
}

struct AlloApp : App {
  Image image;
  Fragment fragments[NumFragment][NumFragment];
  Mesh m[NumFragment][NumFragment];

  AlloApp() {
    image = loadImageFile("student/","12.bmp");
    al::Array array(image.array());

    Image::RGBPix<uint8_t> pixel;
    for (int i = 0; i < NumFragment; ++i) {
      for (int j = 0; j < NumFragment; ++j) {

        for (size_t row = 0; row < FragmentHeight; ++row) {
          for (size_t col = 0; col < FragmentWidth; ++col) {
              array.read(&pixel.r, i*FragmentWidth + col, j*FragmentHeight + row);
              fragments[i][j].rgbArray[row][col] = RGB(pixel.r / 256.0, pixel.g / 256.0, pixel.b / 256.0);
              // cout<<"R:"<< fragments[i][j].rgbArray[row][col].r <<endl;
              // cout<<"G:"<< fragments[i][j].rgbArray[row][col].g <<endl;
              // cout<<"B:"<< fragments[i][j].rgbArray[row][col].b <<endl;
          }
        }
        fragments[i][j].orientation = Vec3f(0,0,1);
        fragments[i][j].position = Vec3f(((float)i+.5)/NumFragment*ImageActualHeight*1.1,((float)j+.5)/NumFragment*ImageActualWidth*1.1,0);
        // fragments[i][j].position = Vec3f(((float)i+.5)/NumFragment,((float)j+.5)/NumFragment,0);
      }
    }

    for (int i = 0; i < NumFragment; ++i) {
      for (int j = 0; j < NumFragment; ++j) {
        makeMeshByFragment(m[i][j],fragments[i][j]);
      }
    }
    initWindow();
  }

  virtual void onDraw(Graphics& g, const Viewpoint& v) {
    for (int i = 0; i < NumFragment; ++i) {
      for (int j = 0; j < NumFragment; ++j) {
        g.draw(m[i][j]);
      }
    }
  }

  virtual void onMouseMove(const ViewpointWindow& w, const Mouse& m){
  }
  virtual void onMouseDown(const ViewpointWindow& w, const Mouse& m){}
  virtual void onMouseUp(const ViewpointWindow& w, const Mouse& m){}
  virtual void onMouseDrag(const ViewpointWindow& w, const Mouse& m){}
};
int main(int argc, char* argv[]) {
  AlloApp app;
  app.start();
  return 0;
}
