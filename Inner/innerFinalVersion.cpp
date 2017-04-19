// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// The INNER [Final Version]
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


#include "allocore/io/al_App.hpp"
#include "Cuttlebone/Cuttlebone.hpp"
#include "alloutil/al_Simulator.hpp"
#include "Gamma/SamplePlayer.h"
#include <cassert>
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"

#define MAX_POSITION_COUNT (100)
struct State {
  Vec3f position[MAX_POSITION_COUNT];
};

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

  void createRecMesh(int ROWS, int COLUMNS){
    primitive(Graphics::TRIANGLES);

    vertex(- size/2 * ratio , - size/2, 0);
    vertex(+ size/2 * ratio , - size/2, 0);
    vertex(- size/2 * ratio , + size/2, 0);
    vertex(+ size/2 * ratio , - size/2, 0);
    vertex(- size/2 * ratio , + size/2, 0);
    vertex(+ size/2 * ratio , + size/2, 0);

    texCoord((float)i/COLUMNS, (float)j/ROWS);
    texCoord((float)(i+1)/COLUMNS, (float)j/ROWS);
    texCoord((float)i/COLUMNS, (float)(j+1)/ROWS);
    texCoord((float)(i+1)/COLUMNS, (float)j/ROWS);
    texCoord((float)i/COLUMNS, (float)(j+1)/ROWS);
    texCoord((float)(i+1)/COLUMNS, (float)(j+1)/ROWS);

    pos(center);
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

    pos(currentCenter);
    vertices()[0] = Vec3f(- size/2 * ratio , - size/2, 0);
    vertices()[1] = Vec3f(+ size/2 * ratio , - size/2, 0);
    vertices()[2] = Vec3f(- size/2 * ratio , + size/2, 0);
    vertices()[3] = Vec3f(+ size/2 * ratio , - size/2, 0);
    vertices()[4] = Vec3f(- size/2 * ratio , + size/2, 0);
    vertices()[5] = Vec3f(+ size/2 * ratio , + size/2, 0);
  }

  void moveCenter(Vec3f c1 = Vec3f(0,-5,0)){
    center[1] = c1[1];
  }

  void onDraw(Graphics& g) {
    g.pushMatrix();
    g.translate(pos());
    g.rotate(quat());
    g.draw(*this);
    g.popMatrix();
  }
};

class FragmentedImage : Pose {
public:
  Texture texture;
  Fragment* fragments;
  float ratio;
  float density;
  float scale;
  int ROWS, COLUMNS;
  int* Array;
  FragmentedImage(){

  }
  FragmentedImage(Texture& _texture , float _ratio,
      float _density, float _scale, int _ROWS, int _COLUMNS){
      texture = _texture;
      ratio = _ratio;
      density = _density;
      scale = _scale;
      ROWS = _ROWS;
      COLUMNS = _COLUMNS;
  }

  void createImage() {
    fragments = new Fragment[ROWS * COLUMNS];
    Array = new int[ROWS * COLUMNS];

    for(int i = 0 ; i < COLUMNS ; i++){
        for(int j = 0 ; j < ROWS ; j++){

          Vec3f imageCenter = Vec3f((ROWS-1)/2*scale,(COLUMNS-1)/2*scale,0);
          Vec3f center = (Vec3f(i*ratio*scale,j*scale,0) - imageCenter)*density + imageCenter;
          float x = rnd::uniformS();
          float size = 1.3*(x*0.3+0.7)*scale;
          fragments[i*ROWS+j] = Fragment(center,size,ratio,i,j);
          fragments[i*ROWS+j].createRecMesh(ROWS,COLUMNS);
          Array[i*ROWS+j] = 1;
      }
    }
  }

  void onDraw(Graphics& g){
      g.pushMatrix();
      g.translate(pos());
      g.scale(1);
      texture.bind();
      for(int j = 0 ; j < COLUMNS ; j++){
        for(int k = 0 ; k < ROWS ; k++){
            fragments[j*ROWS+k].onDraw(g);
        }
      }
      texture.unbind();
      g.popMatrix();
  }

  void dropOneFragment(){
    float x = rnd::uniform();
    float y = rnd::uniform();
    int i = x * (COLUMNS-1);
    int j = y * (ROWS-1);
    fragments[i*ROWS+j].moveCenter();
    Array[i*ROWS+j] = 0;
    fragments[i*ROWS+j].updateMesh();
  }

  void heartBeatMove(float visualLoudnessMeasure){
    for(int i = 0 ; i < COLUMNS ; i++){
        for(int j = 0 ; j < ROWS ; j++){
            float x = rnd::uniformS();
            if (Array[i*ROWS+j] == 1){
                float y = rnd::uniformS();
                Vec3f imageCenter = Vec3f((ROWS-1)/2*scale,(COLUMNS-1)/2*scale,0);
                fragments[i*ROWS+j].center = (Vec3f(i*ratio*scale,j*scale,y)
                  - imageCenter)*(5*visualLoudnessMeasure+density) + imageCenter;
            }
            fragments[i*ROWS+j].updateMesh();
        }
    }
  }
  void setPosition(Vec3f _v){
    pos(_v);
  }
};

struct MyApp : App, AlloSphereAudioSpatializer, InterfaceServerClient {
  cuttlebone::Maker<State, 9000> maker;

    Texture* texture;
    FragmentedImage* fragmentedImage;
    double t,t2,t3;
    float* ratio;
    float scale;
    float density;
    float visualLoudnessMeasure;
    int currentImageIndex;
    int fileCount;
    FileList file;
    SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> heartBeatSamples, fragmentSoundSamples;

    Mesh soundSourceMesh;

    SoundSource heartBeatSource, fragmentSoundSource;

    MyApp()
      : maker(Simulator::defaultBroadcastIP()),
        InterfaceServerClient(Simulator::defaultInterfaceServerIP()) {

      scale = 0.1;
      density = 1.5;
      currentImageIndex = 0;

      SearchPaths searchPaths;
      searchPaths.addSearchPath("student/jing.yan/final");
      file = searchPaths.glob("(.*)\\.(png|jpg|jpeg|bmp)");
      cout << "Found " << file.count() << " files:" << endl;
      file.print();

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
        texture[i].allocate(image.array());
        ratio[i] = (float) image.width() / image.height();
        fragmentedImage[i] = FragmentedImage(texture[i], ratio[i], density, scale, 30, 30);
        fragmentedImage[i].createImage();
      }

      heartBeatSamples.load(fullPathOrDie("heart_beat.wav").c_str());
      fragmentSoundSamples.load(fullPathOrDie("inner_1.wav").c_str());

      AlloSphereAudioSpatializer::initAudio();
      AlloSphereAudioSpatializer::initSpatialization();
      gam::Sync::master().spu(AlloSphereAudioSpatializer::audioIO().fps());
      scene()->usePerSampleProcessing(false);

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

    t += dt;
    t2 += dt;
    if(((int)t2)%4 == 0 || ((int)t2)%4 == 1) {}
    else t3 += dt;
    if (t > 2){
          t = 0;
          for(int i = 0 ; i<fileCount ; i++)
            fragmentedImage[i].dropOneFragment();
    }
    for(int i = 0 ; i<fileCount ; i++){
      fragmentedImage[i].heartBeatMove(visualLoudnessMeasure);
    }
    float a = 2.5;
    fragmentedImage[0].setPosition(Vec3f(0,0,-5*a));
    fragmentedImage[1].setPosition(Vec3f(-1,1,-15*a));
    fragmentedImage[2].setPosition(Vec3f(0.5,-0.5,-25*a));
    fragmentedImage[3].setPosition(Vec3f(-1,-1,-35*a));
    fragmentedImage[4].setPosition(Vec3f(1,0,-45*a));
    fragmentedImage[5].setPosition(Vec3f(-0.5,-1,-55*a));
    fragmentedImage[6].setPosition(Vec3f(-0.75,0.75,-65*a));
    fragmentedImage[7].setPosition(Vec3f(-0.25,0,-75*a));
  }

  virtual void onDraw(Graphics& g) {

      if (t3 < 66) g.translate(0,0,3*t3);
      else  g.translate(0,0,3*66 - (t2-132)*20);

      if(3*66 - (t2-132)*20 < 0){
          t2 = 0;
          t3 = 0;
      }
      cout<<"t3:"<<t3<<endl;
      cout<<"t2:"<<t2<<endl;
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
      //
      float x = 3.0 * sin(secondsCounter * 0.5 * 2 * M_PI);
      float z = 3.0 * cos(secondsCounter * 0.5 * 2 * M_PI);
      heartBeatSource.pos(0,0,0);
      fragmentSoundSource.pos(x,0,z);

      // Generate a test signal with decaying envelope
      //
      float sample = heartBeatSamples();
      float sample2 = fragmentSoundSamples();

      // record the "loudest" sample from this block
      //
      if (abs(sample) > visualLoudnessMeasure)
        visualLoudnessMeasure = abs(sample);


      // Write sample to the source
      //
      heartBeatSource.writeSample(sample + sample2);

      // add a second sound
      // fragmentSoundSource.writeSample(fragmentSoundSamples());

      ++sampleCount;
    }

    // render this scene buffer (renders as many frames as specified at
    // initialization)
    //
    listener()->pos(0, 0, 0);
    scene()->render(io);
  }

  virtual void onKeyDown(const ViewpointWindow&, const Keyboard& k){



    if (k.key() == ' '){ // key hold
      if(currentImageIndex == 0){
        fragmentedImage[(int)t2/17].density *= 0.3;
      }
      if(currentImageIndex == 1){
        fragmentedImage[(int)t2/17].density /= 0.3;
      }
      currentImageIndex = (currentImageIndex+1)%2;
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
