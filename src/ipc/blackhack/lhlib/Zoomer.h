#ifndef BLACKHACK_ZOOMER_H
#define BLACKHACK_ZOOMER_H

#include "LHPoint.h"

struct Zoomer {
  float current_value;
  float destination;
  float destination_speed;
  float current_speed;
  float time_m2;
  float current_time;
  float duration;
  float start_value;
  float start_speed;
  struct LHPoint non_linear_acceleration;
};
static_assert(sizeof(struct Zoomer) == 0x30, "Struct is of wrong size");

struct Zoomer3d {
  struct Zoomer x;
  struct Zoomer y;
  struct Zoomer z;
};

#endif //BLACKHACK_ZOOMER_H
