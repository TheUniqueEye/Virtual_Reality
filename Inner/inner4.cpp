// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// [INNER] Version 4: heartBeatSource added
//
// press key 1 to reconstruct the image;
// press key 2 to expand and loosen the image

#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"
#include "allocore/sound/al_Vbap.hpp" //
#include "allocore/sound/al_Dbap.hpp" //
#include "allocore/sound/al_Ambisonics.hpp"
#include <cassert>

using namespace al;
using namespace std;
using namespace gam;

#define NumOfRow 30
#define NumOfCol 30
#define BLOCK_SIZE 256

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(".");
  string filePath = searchPaths.find(fileName).filepath();
  assert(filePath != "");
  return filePath;
}

class Fragment : Mesh, Pose{
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
  }

  void createRecMesh(){
    primitive(Graphics::TRIANGLES);

    vertex(- size/2 * ratio , - size/2, 0);
    vertex(+ size/2 * ratio , - size/2, 0);
    vertex(- size/2 * ratio , + size/2, 0);
    vertex(+ size/2 * ratio , - size/2, 0);
    vertex(- size/2 * ratio , + size/2, 0);
    vertex(+ size/2 * ratio , + size/2, 0);

    texCoord((float)i/NumOfCol, (float)j/NumOfRow);
    texCoord((float)(i+1)/NumOfCol, (float)j/NumOfRow);
    texCoord((float)i/NumOfCol, (float)(j+1)/NumOfRow);
    texCoord((float)(i+1)/NumOfCol, (float)j/NumOfRow);
    texCoord((float)i/NumOfCol, (float)(j+1)/NumOfRow);
    texCoord((float)(i+1)/NumOfCol, (float)(j+1)/NumOfRow);

    pos(center); // pos的用法
  }

  void updateMesh(){

    Vec3f currentCenter = pos();
    float moveStep = 0.03;
    float distance = (center - currentCenter).mag();
    Vec3f direction = (center - currentCenter).normalize();
    if ((currentCenter - center).mag() > moveStep ){
      currentCenter = currentCenter + direction * moveStep;
    }
    if ((currentCenter - center).mag() < moveStep ){
      currentCenter = center;
    }

    // float amt = lerp(amt,0.1,0.1);
    // currentCenter.lerp(center, amt);

    pos(currentCenter);

    // pos(afterCenter[0],afterCenter[1],afterCenter[2]);
    // pos(currentCenter[0],currentCenter[1],currentCenter[2]);

    vertices()[0] = Vec3f(- size/2 * ratio , - size/2, 0);
    vertices()[1] = Vec3f(+ size/2 * ratio , - size/2, 0);
    vertices()[2] = Vec3f(- size/2 * ratio , + size/2, 0);
    vertices()[3] = Vec3f(+ size/2 * ratio , - size/2, 0);
    vertices()[4] = Vec3f(- size/2 * ratio , + size/2, 0);
    vertices()[5] = Vec3f(+ size/2 * ratio , + size/2, 0);

    // Pose::print();
  }

  void moveCenter(Vec3f c1 = Vec3f(0,-5,0)){
    center[1] = c1[1];
  }

  void onDraw(Graphics& g) {
    g.pushMatrix();
    g.translate(pos());
    g.rotate(quat()); /// quat?
    g.draw(*this);
    g.popMatrix();
  }
};


struct MyApp : App {
    Texture texture;
    double t,t2;
    float ratio;
    float scale;
    float density;
    Fragment fragmentData[NumOfCol][NumOfRow];
    int Array[NumOfCol][NumOfRow];
    float visualLoudnessMeasure;
    SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> heartBeatSamples, fragmentSoundSamples, nearDoorSoundSamples;
    Pose imagePose;

    Mesh soundSourceMesh;
    SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
    StereoPanner* panner;
    // Dbap* panner = new Dbap(speakerLayout);
    // Vbap* panner = new Vbap(speakerLayout);
    // AmbisonicsSpatializer* panner =
    //   new AmbisonicsSpatializer(speakerLayout, 2, 1); // dimension and order

    Listener* listener;
    SoundSource heartBeatSource, fragmentSoundSource, nearDoorSoundSource;  // XXX
    AudioScene scene;

    MyApp() :scene(BLOCK_SIZE){

      Image image;
      const char* fileName = "9.jpg";
      if (!image.load(fullPathOrDie(fileName))) {
        cerr << "failed to load " << fileName << endl;
        exit(1);
      }
      texture.allocate(image.array());

      ratio = image.width()/ (float)image.height();
      scale = 0.1;
      density = 2;

      for(int i = 0 ; i < NumOfCol ; i++){
          for(int j = 0 ; j < NumOfRow ; j++){
              // Vec3f center = Vec3f(i*ratio * scale* density, j * scale *density , 0); /// *ratio 防止碎片之间的重叠 ， *scale改变碎片中心的疏密
              // float size = abs(sin((float)(i*j)/NumOfCol));
              Vec3f imageCenter = Vec3f((NumOfRow-1)/2*scale,(NumOfCol-1)/2*scale,0);
              Vec3f center = (Vec3f(i*ratio*scale,j*scale,0) - imageCenter)*density + imageCenter;

              float x = rnd::uniformS();
              float size;
              int indexBorder;
              if(min(i,NumOfCol-i) < min(j,NumOfRow-j))
                indexBorder = min(i,NumOfCol-i);
              else
                indexBorder = min(j,NumOfRow-j);
              float Probability = min(i,NumOfCol-i) * min(j,NumOfRow-j) / (NumOfRow*NumOfCol/4);
              /// 碎片产生的概率 越向中心产生的概率越高
              // if (x > Probability)
              //   size = 0;
              // else
              size = 1.3*(x*0.3+0.7)*scale;

              fragmentData[i][j] = Fragment(center,size,ratio,i,j);
              fragmentData[i][j].createRecMesh();
              Array[i][j] = 1;
          }
      }

      heartBeatSamples.load(fullPathOrDie("heart_beat.wav").c_str());
      fragmentSoundSamples.load(fullPathOrDie("fragment.wav").c_str()); // XXX
      nearDoorSoundSamples.load(fullPathOrDie("nearDoor.wav").c_str()); // XXX

      panner = new StereoPanner(speakerLayout);
      listener = scene.createListener(panner);
      scene.addSource(heartBeatSource); // XXX
      scene.addSource(fragmentSoundSource); // XXX
      scene.addSource(nearDoorSoundSource); // XXX
      scene.usePerSampleProcessing(false);
      heartBeatSource.dopplerType(DOPPLER_NONE);
      listener->compile();
      panner->print();
      initAudio(44100, BLOCK_SIZE);

      gam::Sync::master().spu(audioIO().fps()); ///
      initWindow();
      nav().pos(0,0,25);
      background(HSV(0,0,.3));
      t=0;
      t2=0;
      cout<<"size:  "<<heartBeatSamples.size()<<endl;
  }

  virtual void onAnimate(double dt) {
    t += dt;
    if (t > 0.3){
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
              Vec3f imageCenter = Vec3f((NumOfRow-1)/2*scale,(NumOfCol-1)/2*scale,0);
              fragmentData[i][j].center = (Vec3f(i*ratio*scale,j*scale,y*0.3) - imageCenter)*(3*visualLoudnessMeasure+density) + imageCenter;
          }
          fragmentData[i][j].updateMesh();
        }
    }

    t2 += dt;
    float degree = 1*t2;
    float radius = 2 +.1*t2;
    // imagePose.pos(Vec3f(cos(degree)*radius,0,sin(degree)*radius)); //同名函数 多态 设置参数值，返回参数值
  }

  virtual void onDraw(Graphics& g) {
    g.pushMatrix();
    g.translate(imagePose.pos()); //同名函数 多态 设置参数值，返回参数值
    // g.rotate(quat()); /// quat?
    texture.bind();
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
          fragmentData[i][j].onDraw(g);
      }
    }
    texture.unbind();
    g.popMatrix();
  }

  virtual void onSound(AudioIOData& io) {
    visualLoudnessMeasure = 0;

    static unsigned int sampleCount = 0;
    if(sampleCount > (std::min(fragmentSoundSamples.size(),heartBeatSamples.size())-10)) sampleCount = 0;
    int numFrames = io.framesPerBuffer();

    float maximumSampleThisBlock = 0;
    for (int i = 0; i < numFrames; i++) {
      double secondsCounter = (sampleCount / io.fps());

      // Vec3f position = pos();///
      // heartBeatSource.pos(nav().pos().x,nav().pos().y,nav().pos().z);
      Vec3f imageCenter = Vec3f((NumOfRow-1)/2*scale,(NumOfCol-1)/2*scale,0);
      heartBeatSource.pos(imageCenter.x,imageCenter.y,imageCenter.z); //XXX
      fragmentSoundSource.pos(nav().pos().x,nav().pos().y,nav().pos().z); //XXX
      // envelope

      float sample = heartBeatSamples[sampleCount];
      if (abs(sample) > maximumSampleThisBlock)
        maximumSampleThisBlock = abs(sample);

      heartBeatSource.writeSample(heartBeatSamples[sampleCount]);
      fragmentSoundSource.writeSample(fragmentSoundSamples[sampleCount]);
      ++sampleCount;
    }

    visualLoudnessMeasure = maximumSampleThisBlock;
    visualLoudnessMeasure /= 5;
    scene.render(io);
  }

  virtual void onKeyDown(const ViewpointWindow&, const Keyboard& k){
    if (k.key() == '1'){
        density = -density * .7;
    }
    if (k.key() == '2'){
        density -= 0.3;
    }
    if (k.key() == '3'){
        fragmentData[1][1].center -= Vec3f(0.1,0.1,0.1);
    }
  }
};


int main() { MyApp().start(); }
