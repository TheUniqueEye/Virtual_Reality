// MAT201B
// Fall 2015
// Author(s): Jing Yan / EYE
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
#include "Cuttlebone/Cuttlebone.hpp"  // XXX
#include "alloutil/al_Simulator.hpp"  // XXX
#include "innerCommon.hpp"  // XXX

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

struct MyApp : App , InterfaceServerClient { // XXX
  Texture texture;
  double t;
  float ratio;
  float scale;
  float density;
  Fragment fragmentData[NumOfCol][NumOfRow];
  int Array[NumOfCol][NumOfRow];
  float visualLoudnessMeasure;
  SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> heartBeatSamples, fragmentSoundSamples, nearDoorSoundSamples;

  Mesh soundSourceMesh;
  SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
  StereoPanner* panner;
  // Dbap* panner = new Dbap(speakerLayout);
  // Vbap* panner = new Vbap(speakerLaygout);
  // AmbisonicsSpatializer* panner =
  //   new AmbisonicsSpatializer(speakerLayout, 2, 1); // dimension and order

  Listener* listener;
  SoundSource heartBeatSource, fragmentSoundSource, nearDoorSoundSource;  // XXX
  AudioScene scene;

  cuttlebone::Maker<State> maker;  // XXX
  State state;                     // XXX

  MyApp()
    : scene(BLOCK_SIZE),
      maker(Simulator::defaultBroadcastIP()),
      InterfaceServerClient(Simulator::defaultInterfaceServerIP()){ // XXX

    Image image;
    const char* fileName = "7.jpg";
    if (!image.load(fullPathOrDie(fileName))) {
      cerr << "failed to load " << fileName << endl;
      exit(1);
    }
    texture.allocate(image.array());
    //state.texture = texture;            // XXX

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
        // int indexBorder;
        // if(min(i,NumOfCol-i) < min(j,NumOfRow-j))
        // indexBorder = min(i,NumOfCol-i);
        // else
        // indexBorder = min(j,NumOfRow-j);
        // float Probability = min(i,NumOfCol-i) * min(j,NumOfRow-j) / (NumOfRow*NumOfCol/4);
        // /// 碎片产生的概率 越向中心产生的概率越高
        // // if (x > Probability)
        // //   size = 0;
        // // else
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
    background(0.3);
    t=0;
    cout<<"size:  "<<heartBeatSamples.size()<<endl;

    InterfaceServerClient::setNav(nav());    // XXX
    InterfaceServerClient::setLens(lens());  // XXX
  }

  virtual void onAnimate(double dt) {

    while (InterfaceServerClient::oscRecv().recv())
    ;  // XXX

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
        state.fragmentPose[i][j] = fragmentData[i][j];            // XXX
      }
    }
    state.pose = nav();
    maker.set(state);  // XXX
  }

  virtual void onDraw(Graphics& g) {

    texture.bind();
    for(int i = 0 ; i < NumOfCol ; i++){
      for(int j = 0 ; j < NumOfRow ; j++){
        fragmentData[i][j].onDraw(g);
      }
    }
    texture.unbind();
    // Mesh mm;
    // mm.vertex(0,0,0);
    // mm.color(1,0,0);
    // mm.primitive(Graphics::POINTS);
    // g.draw(mm);

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

int main() {
  MyApp app;
  app.maker.start();  // XXX
  app.start();
}
