#ifndef __COMMON_STUFF__
#define __COMMON_STUFF__

#define NumOfRow 30
#define NumOfCol 30
#define BLOCK_SIZE 256

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
    quat(Quatf());
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


struct State {
  al::Pose pose;
  al::Pose fragmentPose[NumOfCol][NumOfRow];
};

#endif
