// MAT201B Fall 2015
// Author: Jing Yan / EYE (2015.12.13)
//
// The INNER
//
//  < This is the Common, which contains the common stuff used by both simulator
// and render. In the state class, there is the pose of images, fragments and camera.
// Also t2, t3 is contained to transfer the movement of the images.
// Besides that, there are some commonly used defines, a class of fragment,
// and a class of fragmented image. >

#ifndef __COMMON_STUFF__
#define __COMMON_STUFF__

#define NumOfRow 50
#define NumOfCol 50
#define NumOfImage 10
#define BLOCK_SIZE 256

// Class of fragment is inherited from Mesh and Pose.
// It has functions to create a fragment, update a fragment, move a fragment
// and draw a fragment.

struct Fragment : Mesh, Pose{

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

// Class of fragmented image is inherited from pose.
// It has functions to create an image, draw an image, drop a fragment from the image,
// visualize the heart beat amplitude, and move the image.

struct FragmentedImage : Pose {

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

  struct State {
    al::Pose pose;
    al::Pose fragmentPose[NumOfImage][NumOfCol][NumOfRow];
    al::Pose fragmentedImagePose[NumOfImage];
    double t2,t3;
  };

  #endif
