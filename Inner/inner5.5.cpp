// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// [INNER] Version 5.5: backup
//
// press key 1 to reconstruct the image;
// press key 2 to expand and loosen the image

#include "allocore/io/al_App.hpp"
#include "Gamma/SamplePlayer.h"
#include "allocore/sound/al_Vbap.hpp" //
#include "allocore/sound/al_Dbap.hpp" //
#include "allocore/sound/al_Ambisonics.hpp"
#include <cassert>
// #include "alloutil/al_AlloSphereAudioSpatializer.hpp" // XXX

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
    g.rotate(quat());
    g.draw(*this);
    g.popMatrix();
  }
};

class FragmentedImage : Pose {
public:
  Texture texture;
  Fragment* fragments;
  float ratio; //
  float density;
  float scale;
  int ROWS, COLUMNS;
  int* Array; // int Array[][]
  FragmentedImage(){

  }
  FragmentedImage(Texture& _texture , float _ratio,  /// XXX Texture& _texture failed
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
          // int indexBorder;
          // if(min(i,NumOfCol-i) < min(j,NumOfRow-j))
          //   indexBorder = min(i,NumOfCol-i);
          // else
          //   indexBorder = min(j,NumOfRow-j);
          // float Probability = min(i,NumOfCol-i) * min(j,NumOfRow-j) / (NumOfRow*NumOfCol/4);
          // // if (x > Probability)
          // //   size = 0;
          // // else

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
      // g.rotate(quat());
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
    // cout<<i<<" "<<j<<endl;
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

struct MyApp : App {
    Texture* texture;
    FragmentedImage* fragmentedImage;
    double t,t2,t3;
    float* ratio;
    float scale;
    float density;
    float visualLoudnessMeasure;
    int fileCount;
    FileList file;
    SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> heartBeatSamples, fragmentSoundSamples;

    Mesh soundSourceMesh;
    SpeakerLayout speakerLayout = HeadsetSpeakerLayout();
    StereoPanner* panner;
    // Dbap* panner = new Dbap(speakerLayout);
    // Vbap* panner = new Vbap(speakerLayout);
    // AmbisonicsSpatializer* panner =
    //   new AmbisonicsSpatializer(speakerLayout, 2, 1); // dimension and order

    Listener* listener;
    SoundSource heartBeatSource, fragmentSoundSource;  // XXX
    AudioScene scene;

    MyApp() :scene(BLOCK_SIZE){

      scale = 0.1;
      density = 1.5;

      // const char* fileName = "7.jpg";
      // if (!image.load(fullPathOrDie(fileName))) {
      //   cerr << "failed to load " << fileName << endl;
      //   exit(1);
      // }

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
        // Texture tempTexture = texture[i];
        fragmentedImage[i] = FragmentedImage(texture[i], ratio[i], density, scale, 30, 30); // XXX texture[i] failed
        fragmentedImage[i].createImage();
      }

      heartBeatSamples.load(fullPathOrDie("heart_beat.wav").c_str());
      fragmentSoundSamples.load(fullPathOrDie("fragment.wav").c_str()); // XXX



      panner = new StereoPanner(speakerLayout);
      listener = scene.createListener(panner);
      // listener->pos(0,0,0);
      scene.addSource(heartBeatSource); // XXX
      // scene.addSource(fragmentSoundSource); // XXX
      // scene.addSource(nearDoorSoundSource); // XXX
      // scene.usePerSampleProcessing(false);
      // heartBeatSource.dopplerType(DOPPLER_NONE);
      listener->compile();
      panner->print();
      initAudio(44100, BLOCK_SIZE);
      // gam::Sync::master().spu(audioIO().fps()); ///
      initWindow();
      nav().pos(0,0.5,10);
      background(0.3);
      t=0;
      t2=0;
      t3=0;
      // cout<<"size:  "<<heartBeatSamples.size()<<endl;
  }

  virtual void onAnimate(double dt) {
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
      fragmentedImage[i].heartBeatMove(visualLoudnessMeasure); //visualLoudnessMeasure
    }
    float a = 2.5;
    fragmentedImage[0].setPosition(Vec3f(0,0,-5*a)); // XXX set the position
    fragmentedImage[1].setPosition(Vec3f(-1,1,-15*a));
    fragmentedImage[2].setPosition(Vec3f(0.5,-0.5,-25*a));
    fragmentedImage[3].setPosition(Vec3f(-1,-1,-35*a));
    fragmentedImage[4].setPosition(Vec3f(1,0,-45*a));
    fragmentedImage[5].setPosition(Vec3f(-0.5,-1,-55*a));
    fragmentedImage[6].setPosition(Vec3f(-0.75,0.75,-65*a));
    fragmentedImage[7].setPosition(Vec3f(-0.25,0,-75*a));
    // fragmentedImage[5].rotate();
  }

  virtual void onDraw(Graphics& g) {

      // g.pushMatrix();
      // float velocity = 0;
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
      // g.popMatrix();
  }

  virtual void onSound(AudioIOData& io) {
    visualLoudnessMeasure = 0;

    // static unsigned int sampleCount = 0;
    // if(sampleCount > (std::min(fragmentSoundSamples.size(),heartBeatSamples.size())-10)) sampleCount = 0;
    //
    // int numFrames = io.framesPerBuffer();
    //
    // float maximumSampleThisBlock = 0;
    // for (int i = 0; i < numFrames; i++) {
    //   double secondsCounter = (sampleCount / io.fps());
    //
    //   // Vec3f position = pos();///
    //   // heartBeatSource.pos(nav().pos().x,nav().pos().y,nav().pos().z);
    //   Vec3f imageCenter = Vec3f((NumOfRow-1)/2*scale,(NumOfCol-1)/2*scale,0);
    //   heartBeatSource.pos(imageCenter.x,imageCenter.y,imageCenter.z); //XXX
    //   fragmentSoundSource.pos(nav().pos().x,nav().pos().y,nav().pos().z); //XXX
    //   // envelope
    //
    //   float sample = heartBeatSamples[sampleCount];
    //   if (abs(sample) > maximumSampleThisBlock)
    //     maximumSampleThisBlock = abs(sample);
    //
    //   heartBeatSource.writeSample(heartBeatSamples[sampleCount]);
    //   fragmentSoundSource.writeSample(fragmentSoundSamples[sampleCount]);
    //   ++sampleCount;
    // }
    //
    // visualLoudnessMeasure = maximumSampleThisBlock;
    // cout<<visualLoudnessMeasure<<endl;
    // visualLoudnessMeasure /= 5;
    // scene.render(io);

    static unsigned int sampleCount = 0;

    int numFrames = io.framesPerBuffer();

    float maximumSampleThisBlock = 0;
    for (int i = 0; i < numFrames; i++) {
      double secondsCounter = (sampleCount / io.fps());

      // Create an oscillating trajectory for the sound source
      //
      float x = 3.0 * sin(secondsCounter * 0.5 * 2 * M_PI);
      float z = 3.0 * cos(secondsCounter * 0.5 * 2 * M_PI);
      heartBeatSource.pos(0,0,0);
      // fragmentSoundSource.pos(0,0,0); //XXX

      // Generate a test signal with decaying envelope
      //
      float sample = heartBeatSamples[sampleCount];
      // float envelope = 1 - (secondsCounter - unsigned(secondsCounter));
      // sample *= envelope * envelope;

      // record the "loudest" sample from this block
      //
      if (abs(sample) > maximumSampleThisBlock)
        maximumSampleThisBlock = abs(sample);

      // Write sample to the source
      //
      heartBeatSource.writeSample(sample+fragmentSoundSamples[sampleCount]); // XXX
      // heartBeatSamples + fragmentSoundSamples ; 2 SoundSource? XXX

      // sample = fragmentSoundSamples[sampleCount];
      // fragmentSoundSource.writeSample(sample);

      ++sampleCount;
      if(sampleCount == heartBeatSamples.size()-2)  sampleCount = 0;
    }
    // cout<<sampleCount<<endl;
    visualLoudnessMeasure = maximumSampleThisBlock;

    // render this scene buffer (renders as many frames as specified at
    // initialization)
    //
    scene.render(io);
  }

  virtual void onKeyDown(const ViewpointWindow&, const Keyboard& k){
    float tempT;
    if (k.key() == '1'){
      tempT= t2;
      fragmentedImage[3].density *= 0.6;
      // usleep(2*1000*1000);
      // fragmentedImage[3].density *= -0.8;
    }
    // if(t2-tempT > 0.05)
    //   fragmentedImage[0].density *= -0.8;

    if (k.key() == '2'){
      fragmentedImage[3].density /= 0.8;
    }
    if (k.key() == '3'){
        // fragmentData[1][1].center -= Vec3f(0.1,0.1,0.1);
    }
  }
};


int main() { MyApp().start(); }
