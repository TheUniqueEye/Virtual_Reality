// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// [INNER] Version 1: cut image into fragments and make them bumping like heart.
//
// press key 1 to reconstruct the image;
// press key 2 to expand and loosen the image

#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"
#include <cassert>

using namespace al;
using namespace std;
using namespace gam;

// #define NumFragment 6
#define NumOfRow 30
#define NumOfCol 30

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(".");
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

class Fragment{
public:
  Vec3f center;
  float size;
  float ratio;
  int i;
  int j;

  Fragment(Vec3f _center = Vec3f(0,0,0), float _size = 1, float _ratio = 1, int _i = 0, int _j=0){
      center = _center;
      size = _size;
      ratio = _ratio;
      i = _i;
      j = _j;
  } /// Fragment的构造函数，用来做什么呢？

  void createRecMesh(Mesh& mesh){
    mesh.primitive(Graphics::TRIANGLES);
    float x = center[0]; // center
    float y = center[1];
    float z = center[2];
    // cout<< x << " " << y << " " << z << " " <<endl;

    mesh.vertex(x - size/2 * ratio , y - size/2, z); // mesh vertex append-新增点
    mesh.vertex(x + size/2 * ratio , y - size/2, z);
    mesh.vertex(x - size/2 * ratio , y + size/2, z);
    mesh.vertex(x + size/2 * ratio , y - size/2, z);
    mesh.vertex(x - size/2 * ratio , y + size/2, z);
    mesh.vertex(x + size/2 * ratio , y + size/2, z);

    // for(int i = 0; i< 6; i++){
    //   cout<<i<<" "<<mesh.vertices()[i]<<endl;
    // }
    mesh.texCoord((float)i/NumOfCol, (float)j/NumOfRow); // texture
    mesh.texCoord((float)(i+1)/NumOfCol, (float)j/NumOfRow);
    mesh.texCoord((float)i/NumOfCol, (float)(j+1)/NumOfRow);
    mesh.texCoord((float)(i+1)/NumOfCol, (float)j/NumOfRow);
    mesh.texCoord((float)i/NumOfCol, (float)(j+1)/NumOfRow);
    mesh.texCoord((float)(i+1)/NumOfCol, (float)(j+1)/NumOfRow);
  }

  void updateMesh(Mesh& mesh){ // ASK Karl，改掉原来vertex的位置

    Vec3f currentCenter = ( mesh.vertices()[0] + mesh.vertices()[5])/ 2;
    float moveStep = 0.05;
    float distance = (center - currentCenter).mag();
    Vec3f direction = (center - currentCenter).normalize();
    if ((currentCenter - center).mag() > moveStep ){
      currentCenter = currentCenter + direction * moveStep;
    }
    if ((currentCenter - center).mag() < moveStep ){
      currentCenter = center;
    }

    float x = currentCenter[0];
    float y = currentCenter[1];
    float z = currentCenter[2];

    mesh.vertices()[0] = Vec3f(x - size/2 * ratio , y - size/2, z);
    mesh.vertices()[1] = Vec3f(x + size/2 * ratio , y - size/2, z);
    mesh.vertices()[2] = Vec3f(x - size/2 * ratio , y + size/2, z);
    mesh.vertices()[3] = Vec3f(x + size/2 * ratio , y - size/2, z);
    mesh.vertices()[4] = Vec3f(x - size/2 * ratio , y + size/2, z);
    mesh.vertices()[5] = Vec3f(x + size/2 * ratio , y + size/2, z);


  }

  void moveCenter(Vec3f c1 = Vec3f(0,-5,0)){ ///函数定义一次多次使用
    center[1] = c1[1];
  }
};

struct MyApp : App {
  Texture texture;
  float ratio;
  float scale;
  Mesh fragments[NumOfCol][NumOfRow]; /// mesh
  Fragment fragmentData[NumOfCol][NumOfRow]; /// data
  float visualLoudnessMeasure;
  float tempVLM;
  SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> samplePlayer;
  double t;
  int Array[NumOfCol][NumOfRow];
  float density;

  MyApp() {

    Image image;
    const char* fileName = "1.bmp";
    if (!image.load(fullPathOrDie(fileName))) {
      cerr << "failed to load " << fileName << endl;
      exit(1);
    }
    texture.allocate(image.array()); // allocate image - array - texture

   ratio = image.width()/ (float)image.height();
   scale = 0.2; /// 改变每块碎片的大小
   density = 2;
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){

          Vec3f center = Vec3f(i*ratio * scale , j * scale , 0); /// *ratio 防止碎片之间的重叠 ， *scale改变碎片中心的疏密
          // float size = abs(sin((float)(i*j)/NumOfCol));

          float x = rnd::uniformS(); /// 生成随机数 来使碎片随机不被画出 产生边缘的不完整
          float size;
          // int borderWidth = 3;
          int indexBorder; /// 最边缘的位置：i,j,NumOfRow-j,NumOfCol-i,取最小值，
          if(min(i,NumOfCol-i) < min(j,NumOfRow-j))
          indexBorder = min(i,NumOfCol-i);
          else
          indexBorder = min(j,NumOfRow-j);

          cout<<"i:"<<i<<endl;
          cout<<"j:"<<j<<endl;
          cout<<"indexBorder:"<<indexBorder<<endl;
          cout<<"Probability:"<<(0.7 / (2*indexBorder+1))<<endl;
          cout<<endl;
          float Probability = min(i,NumOfCol-i) * min(j,NumOfRow-j) / (NumOfRow*NumOfCol/4);
          /// 碎片产生的概率 越向中心产生的概率越高
          if (x > Probability)
            size = 0;
          else
            size = 0.9*(x*0.3+0.7)*scale;

          fragmentData[i][j] = Fragment(center,size,ratio,i,j);  /// 运用构造函数
          cout<<i<<" " <<j<<": " <<fragmentData[i][j].center<<endl;

          fragmentData[i][j].createRecMesh(fragments[i][j]); /// 。。

          Array[i][j] = 1;
      }
    }

    SearchPaths searchPaths;
    searchPaths.addSearchPath(".");
    string filePath = searchPaths.find("heart_beat.wav").filepath();
    cout << filePath << endl;
    samplePlayer.load(filePath.c_str());
    initAudio();
    gam::Sync::master().spu(audioIO().fps()); ///
    initWindow();
    nav().pos(1, 0, 50); /// 设定一个初始的观看的位置
    background(0.3); /// 背景色的设定
    t=0;


  }

  virtual void onAnimate(double dt) {
      t += dt;
      // cout<<visualLoudnessMeasure<<endl;
      bool selectFinished = false;
      if (t > 0.1){
        t = 0;
        float x = rnd::uniform();
        float y = rnd::uniform();
        int i = x * (NumOfCol-1);
        int j = y * (NumOfRow-1);
        // cout<<i<<" "<<j<<endl;
        fragmentData[i][j].moveCenter();
        Array[i][j] = 0;

      }

      for(int i = 0 ; i < NumOfCol ; i++){
          for(int j = 0 ; j < NumOfRow ; j++){
              float x = rnd::uniformS();

              if (Array[i][j] == 1){

                float y = rnd::uniformS();
                /// 计算整个图像中心点的位置，随之改变碎片中心点的位置，通过visualLoudnessMeasure
                Vec3f imageCenter = Vec3f((NumOfRow-1)/2*scale,(NumOfCol-1)/2*scale,0);
                fragmentData[i][j].center = (Vec3f(i*ratio*scale,j*scale,y*0.3) - imageCenter)*(3*visualLoudnessMeasure+density) + imageCenter;
                /// 改变原有点的data 。。。
            }
            fragmentData[i][j].updateMesh(fragments[i][j]);
          }
      }
  }

  virtual void onDraw(Graphics& g) {

    texture.bind();
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
          g.draw(fragments[i][j]);
      }
    }
    texture.unbind();
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

  virtual void onKeyDown(const ViewpointWindow&, const Keyboard& k){
    if (k.key() == '1'){
        density = -density * .7;
    }
    if (k.key() == '2'){
        density -= 0.3;
    }
  }
};


int main() { MyApp().start(); }
