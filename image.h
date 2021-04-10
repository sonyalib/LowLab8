#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stddef.h>
#include <stdint.h>

struct Rgb8 {
  #define E(t, n) t n;
  #include "Rgb8.inc"
  #undef E
};

typedef struct Rgb8 *PRgb8;

struct ImageRgb8 {
  #define E(t, n) t n;
  #include "ImageRgb8.inc"
  #undef E
};

int create_image(struct ImageRgb8 *);
void destroy_image(struct ImageRgb8 *);

int image_copy(struct ImageRgb8 const *src, struct ImageRgb8 *dst);

void image_rgb_to_bgr(struct ImageRgb8 *);
void image_bgr_to_rgb(struct ImageRgb8 *);

int image_transpose(struct ImageRgb8 const *src, struct ImageRgb8 *dst);
void image_mirror_h(struct ImageRgb8 *);
void image_mirror_v(struct ImageRgb8 *);

int image_rotate_clockwise(struct ImageRgb8 const *src, struct ImageRgb8 *dst);
int image_rotate_counter_clockwise(struct ImageRgb8 const *src, struct ImageRgb8 *dst);
void image_rotate_180(struct ImageRgb8 *);

#endif
