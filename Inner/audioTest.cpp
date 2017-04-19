// MAT201B
// Fall 2015
// Author(s): Karl Yerkes
//
// Adapted from allocore/examples/audioscene.cpp by Ryan McGee
//
#include "allocore/al_Allocore.hpp"
#include "allocore/sound/al_Vbap.hpp"
#include "allocore/sound/al_Dbap.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
#include "Gamma/SamplePlayer.h"


using namespace al;
using namespace std;
using namespace gam;

#define BLOCK_SIZE (256)

string fullPathOrDie(string fileName, string whereToLook = ".") {
  SearchPaths searchPaths;
  searchPaths.addSearchPath(".");
  string filePath = searchPaths.find(fileName).filepath();
  // assert(filePath != "");
  return filePath;
}

struct MyApp : App {
  Mesh soundSourceMesh;
  float visualLoudnessMeasure;
  SamplePlayer<float, gam::ipl::Linear, phsInc::Loop> samplePlayer;

  // Create a speaker layout
  //
  SpeakerLayout speakerLayout = HeadsetSpeakerLayout();

  // Create a panner: DBAP, VBAP, Ambisonics, or Stereo
  //
  StereoPanner* panner;
  // Dbap* panner = new Dbap(speakerLayout);
  // Vbap* panner = new Vbap(speakerLayout);
  // AmbisonicsSpatializer* panner =
  //   new AmbisonicsSpatializer(speakerLayout, 2, 1); // dimension and order

  // Create listener(s) (at least one)
  //
  Listener* listener;

  // Create a Sound Source
  //
  SoundSource soundSource;

  // Create an audio scene with single argument for frames per buffer
  //
  AudioScene scene;
  MyApp() : scene(BLOCK_SIZE) {

    samplePlayer.load(fullPathOrDie("heart_beat.wav").c_str());

    panner = new StereoPanner(speakerLayout);

    // Initialize the listener(s) with their individual speaker layout and
    // panner
    //
    listener = scene.createListener(panner);

    // Add the sound source to the scene
    //
    scene.addSource(soundSource);

    // Optionally, disable per sample processing to save CPU. Recommended to
    // disable Doppler in this case as well.
    //
    scene.usePerSampleProcessing(false);
    soundSource.dopplerType(DOPPLER_NONE);

    // update the listener's speaker layout and panner call this to dynamically
    // change a listener's speaker layout and panner.
    //
    listener->compile();

    // Print out relevant panner info (ex. number of triplets found for VBAP)
    //
    panner->print();

    // Create an audio IO for the audio scene, specifying blocksize
    //
    initAudio(44100, BLOCK_SIZE);

    addSphere(soundSourceMesh);
    initWindow();
  }

  virtual void onDraw(Graphics& g) {
    g.color(HSV(visualLoudnessMeasure, 1, visualLoudnessMeasure));
    g.translate(soundSource.pos());
    g.draw(soundSourceMesh);
  }

  // Create an audio callback function for the source and scene
  //
  virtual void onSound(AudioIOData& io) {
    static unsigned int sampleCount = 0;

    int numFrames = io.framesPerBuffer();

    float maximumSampleThisBlock = 0;
    for (int i = 0; i < numFrames; i++) {
      double secondsCounter = (sampleCount / io.fps());

      // Create an oscillating trajectory for the sound source
      //
      float x = 3.0 * sin(secondsCounter * 0.5 * 2 * M_PI);
      float z = 3.0 * cos(secondsCounter * 0.5 * 2 * M_PI);
      soundSource.pos(x, 0, z);

      // Generate a test signal with decaying envelope
      //
      float sample = sin(secondsCounter * 440 * 2 * M_PI) * 0.5;
      float envelope = 1 - (secondsCounter - unsigned(secondsCounter));
      sample *= envelope * envelope;

      // record the "loudest" sample from this block
      //
      if (abs(sample) > maximumSampleThisBlock)
        maximumSampleThisBlock = abs(sample);

      // Write sample to the source
      //
      soundSource.writeSample(samplePlayer[sampleCount]);

      ++sampleCount;
    }

    visualLoudnessMeasure = maximumSampleThisBlock;

    // render this scene buffer (renders as many frames as specified at
    // initialization)
    //
    scene.render(io);
  }
};

int main() { MyApp().start(); }
