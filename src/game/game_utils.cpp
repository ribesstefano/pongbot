#include "game_utils.h"

unsigned long xorshf96(void) {
#pragma HLS INLINE
  //period 2^96-1
  unsigned long t;
  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;
  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;
  return z;
}

unsigned long xorrand() {
#pragma HLS INLINE
  return xorshf96();
}