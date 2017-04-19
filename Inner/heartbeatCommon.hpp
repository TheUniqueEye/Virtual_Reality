#ifndef __COMMON_STUFF__
#define __COMMON_STUFF__

#define NumFragment 6

struct State {
  al::Pose pose;
  Vec3f destination[NumFragment*NumFragment];
  HSV hsv[NumFragment*NumFragment];
};

#endif
