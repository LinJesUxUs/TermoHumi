#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef SEGMENT_NUM
#define SEGMENT_NUM 4
#endif

#include "includes.h"

struct display_buf {
  uint8_t temp_segments[SEGMENT_NUM-1];
  uint8_t humi_segments[SEGMENT_NUM-1];
};
#endif
