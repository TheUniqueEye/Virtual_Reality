// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// The INNER
//
// Description:  "The Inner" is about people’s remembering and forgetting.
// A tunnel of image fragments is created, and let the audience "go through".
// Images are cut into fragments using texture coordinate to express the feeling
// of low resolution in memory. The aliveness of memory is communicated via a
// bumping motion resembling the heart which is done by visualizing the amplitude
// envelope of a beating heart sound wave. Audience can interact with the image
// by holding the spacebar which is a metaphor of remembrance.
//
// Interaction control:  hold the spacebar to reconstructe the current image,
//                       release the spacebar to restore it.
//
// < This is the Simulator, which sends data(pose of images, fragments, camera, time)
// to render through state. >


#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "alloutil/al_Simulator.hpp"
#include "innerCommon.hpp"
#include "Gamma/SamplePlayer.h"
#include <cassert>
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"

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

void printFactsAboutState(int size);

struct MyApp : App, AlloSphereAudioSpatializer, InterfaceServerClient {
  cuttlebone::Maker<State, 9000> maker;
  State state;

  Texture* texture;
  FragmentedImage* fragmentedImage;
  double t,t2,t3;
  float* ratio;
  float scale;
  float density;
  float visualLoudnessMeasure;
  int interactFlag;
  int fileCount;
  FileList file;
  SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> heartBeatSamples, fragmentSoundSamples;
  Mesh soundSourceMesh;
  SoundSource heartBeatSource, fragmentSoundSource;
  bool m = false;
  float m_t;

  MyApp()
  : maker(Simulator::defaultBroadcastIP()),
  InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {

    printFactsAboutState(sizeof(State));

    scale = 0.1;
    density = 1.5;
    interactFlag = 0;

    // load all the images in the folder
    SearchPaths searchPaths;
    searchPaths.addSearchPath("student/jing.yan/final");
    file = searchPaths.glob("(.*)\\.(png|jpg|jpeg|bmp)");
    cout << "Found " << file.count() << " files:" << endl;
    file.print();

    // count the number of images
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
      // set the texture with each image, compute the ratio of each image
      // initialize the fragmentedImage object, create each fragmented image
      texture[i].allocate(image.array());
      ratio[i] = (float) image.width() / image.height();
      fragmentedImage[i] = FragmentedImage(texture[i], ratio[i], density, scale, 30, 30);
      fragmentedImage[i].createImage();
    }

    // load two sound samples
    heartBeatSamples.load(fullPathOrDie("heart_beat.wav").c_str());
    fragmentSoundSamples.load(fullPathOrDie("inner_1.wav").c_str());

    AlloSphereAudioSpatializer::initAudio();
    AlloSphereAudioSpatializer::initSpatialization();
    gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());
    scene()->usePerSampleProcessing(false);

    // set the location of listener, add sound source
    listener()->pos(0,0,0);
    scene()->addSource(heartBeatSource);

    // set interface server nav/lens to App's nav/lens
    InterfaceServerClient::setNav(nav());
    InterfaceServerClient::setLens(lens());

    initWindow();
    nav().pos(0,0.5,10);
    background(0);
    t=0;
    t2=0;
    t3=0;
  }

  virtual void onAnimate(double dt) {
    while (InterfaceServerClient::oscRecv().recv());

    // t is a timer which triggers an event and resets every 2 seconds.
    // t2 is a timer which records how long the program is running.
    t += dt;
    t2 += dt;
    if(((int)t2)%4 == 0 || ((int)t2)%4 == 1) {}
    else t3 += dt;

    // drop one fragment from the fragmented images every 2 seconds
    if (t > 2){
      t = 0;
      for(int i = 0 ; i<fileCount ; i++)
      fragmentedImage[i].dropOneFragment();
    }

    // keep the image bumping resembling the heart beat motion
    for(int i = 0 ; i<fileCount ; i++){
      fragmentedImage[i].heartBeatMove(visualLoudnessMeasure);

      // set the orignial postion of 8 images, set depth in Z coordinate
      float a = 2.5;
      fragmentedImage[0].setPosition(Vec3f(0,0,-5*a));
      fragmentedImage[1].setPosition(Vec3f(-1,1,-15*a));
      fragmentedImage[2].setPosition(Vec3f(0.5,-0.5,-25*a));
      fragmentedImage[3].setPosition(Vec3f(-1,-1,-35*a));
      fragmentedImage[4].setPosition(Vec3f(1,0,-45*a));
      fragmentedImage[5].setPosition(Vec3f(-0.5,-1,-55*a));
      fragmentedImage[6].setPosition(Vec3f(-0.75,0.75,-65*a));
      fragmentedImage[7].setPosition(Vec3f(-0.25,0,-75*a));

      // transfer the pose of each fragmented Image and each fragment from
      // simulator.cpp to render.cpp
      for(int i = 0 ; i<fileCount ; i++){
        state.fragmentedImagePose[i] = fragmentedImage[i];
        for(int col = 0 ; col<fragmentedImage[i].COLUMNS ; col++){
          for(int rol = 0 ; rol<fragmentedImage[i].ROWS ; rol++){
            state.fragmentPose[i][col][rol] = fragmentedImage[i].fragments[col*fragmentedImage[i].ROWS+rol];
          }
        }
      }

      // transfer the pose of camera to render.cpp to synchronize the
      // perspective of simulator and render
      state.pose = nav();
      state.t2 = t2;
      state.t3 = t3;
      maker.set(state);

      if (m) {
        m_t += dt;
        cout << m_t << endl;
      }
    }
  }

  virtual void onDraw(Graphics& g) {

    // t3 is used to control the distance between image and viewers
    // translate the coordinate of z-axis to move images forward
    if (t3 < 66) g.translate(0,0,3*t3);
    else g.translate(0,0,3*66 - (t2-132)*20);

    // t2 is reset to 0 to restart the show of image and create a loop.
    if(3*66 - (t2-132)*20 < 0){
      t2 = 0;
      t3 = 0;
    }

    // draw each fragmented image
    for(int i = 0 ; i<fileCount ; i++){
      fragmentedImage[i].onDraw(g);
    }
  }

  virtual void onSound(AudioIOData& io) {
    static unsigned int sampleCount = 0;
    int numFrames = io.framesPerBuffer();
    visualLoudnessMeasure = 0;
    for (int i = 0; i < numFrames; i++) {
      double secondsCounter = (sampleCount / io.fps());

      // Create an oscillating trajectory for the sound source
      float x = 7.0 * sin(secondsCounter * 0.5 * 2 * M_PI);
      float z = 7.0 * cos(secondsCounter * 0.5 * 2 * M_PI);
      heartBeatSource.pos(0,0,4);
      fragmentSoundSource.pos(x,0,z); //XXX

      // Generate a test signal with decaying envelope
      float sample = heartBeatSamples();
      float sample2 = fragmentSoundSamples();

      // record the "loudest" sample from this block
      if (abs(sample) > visualLoudnessMeasure)
      visualLoudnessMeasure = abs(sample);

      // Write sample to the source
      heartBeatSource.writeSample(sample*2 + sample2/2);
      ++sampleCount;
    }
    listener()->pose(nav());
    scene()->render(io);
  }

  // hold the spacebar to reconstructe the current image,
  // release the spacebar to restore it.
  virtual void onKeyDown(const ViewpointWindow&, const Keyboard& k){
    if (k.key() == ' '){
      m = true;
      fragmentedImage[(int)t2/17].density *= 0.3;
    }
  }
  virtual void onKeyUp(const ViewpointWindow&, const Keyboard& k){
    if (k.key() == ' '){
      m_t = 0;
      m = false;
      fragmentedImage[(int)t2/17].density /= 0.3;
    }
  }
};

int main() {
  MyApp app;
  app.AlloSphereAudioSpatializer::audioIO().start();  // start audio
  app.InterfaceServerClient::connect();  // handshake with interface server
  app.maker.start();
  app.start();
}

// print some facts about the state in the terminal
void printFactsAboutState(int size) {
  cout << "==================================================" << endl;
  cout << "Your state type takes " << size << " bytes in memory." << endl;

  if (size > 1024.0f * 1024.0f * 1024.0f) {
    cout << "That is " << size / (1024.0f * 1024.0f * 1024.0f) << " gigabytes."
    << endl;
    cout << "That's too big." << endl;
  } else if (size > 1024.0f * 1024.0f * 10) {
    cout << "That is " << size / (1024.0f * 1024.0f) << " megabytes." << endl;
    cout << "That's a very big state." << endl;
  } else if (size > 1024.0f * 1024.0f) {
    cout << "That is " << size / (1024.0f * 1024.0f) << " megabytes." << endl;
  } else if (size > 1024.0f) {
    cout << "That is " << size / 1024.0f << " kilobytes." << endl;
  } else {
  }

  // Standard 1Gb Ethernet effective bandwidth
  //
  float gbe = 1.18e+8f;

  cout << "On a 1Gb Ethernet LAN (like the MAT network), ";
  if (gbe / size > 60.0f)
  cout << "you will use \n" << 100.0f * (size * 60.0f) / gbe
  << "\% of the effective bandwidth with a framerate of 60 Hz." << endl;
  else
  cout << "your framerate will be *network limited* to " << gbe / size
  << " Hz." << endl;

  cout << "On a 10Gb Ethernet LAN (like the AlloSphere network), ";
  if (10 * gbe / size > 60.0f)
  cout << "you will use \n" << 100.0f * (size * 60.0f) / (10 * gbe)
  << "\% of the effective bandwidth with a framerate of 60 Hz." << endl;
  else {
    cout << "your framerate will be *network limited* to " << 10 * gbe / size
    << " Hz." << endl;
    cout << "Your state is very large." << endl;
  }
  cout << "==================================================" << endl;
}
