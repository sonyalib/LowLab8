#include "bmp.h"
#include "image.h"
#include "log.h"
#include <stdio.h>
#include <sys/resource.h>

static unsigned char sat(uint64_t x) {
  if (x < 256)
    return x;
  return 255;
}

float const matrix[9] = {
/* Считаем сразу в BGR */
#if 1
    .131f, .543f,      .272f, /**/ .168f, .686f,
    .349f, /**/ .189f, .769f, .393f
#else
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f
#endif
};

static void sepia_one(struct Rgb8 *pixel) {
  struct Rgb8 old = *pixel;
  pixel->r = sat(old.r * matrix[0] + old.g * matrix[1] + old.b * matrix[2]);
  pixel->g = sat(old.r * matrix[3] + old.g * matrix[4] + old.b * matrix[5]);
  pixel->b = sat(old.r * matrix[6] + old.g * matrix[7] + old.b * matrix[8]);
}

extern void sepia_asm(float const *matrix, struct Rgb8 *pixels, size_t size);

void sepia_sse(struct ImageRgb8 *img) {
  size_t i;
  size_t full_size = img->width * img->height;
  size_t sse_num = full_size / 4;
  if (sse_num)
    sepia_asm(matrix, img->pixels, sse_num);
  for (i = 4 * sse_num; i < full_size; ++i)
    sepia_one(&img->pixels[i]);
}

void sepia_no_sse(struct ImageRgb8 *img) {
  size_t i, full_size = img->width * img->height;
  for (i = 0; i < full_size; ++i)
    sepia_one(&img->pixels[i]);
}

int main(int argc, char **argv) {
  struct ImageRgb8 img1;
  struct ImageRgb8 img2;
  int32_t maxdiff;
  size_t i, fullsize;
  struct rusage res1, res2, res3;
  long s1, s2, us1, us2;

  if (argc < 4) {
    log_err("Not enough arguments");
    return __LINE__;
  }

  if (from_bmp_path(&img1, argv[1]))
    return __LINE__;

  if (image_copy(&img1, &img2))
    return __LINE__;

  getrusage(RUSAGE_SELF, &res1);
  sepia_no_sse(&img1);
  getrusage(RUSAGE_SELF, &res2);
  sepia_sse(&img2);
  getrusage(RUSAGE_SELF, &res3);

  if (to_bmp_path(&img1, argv[2]))
    return __LINE__;

  if (to_bmp_path(&img2, argv[3]))
    return __LINE__;

  s1 = res2.ru_utime.tv_sec - res1.ru_utime.tv_sec;
  s2 = res3.ru_utime.tv_sec - res2.ru_utime.tv_sec;
  us1 = res2.ru_utime.tv_usec - res1.ru_utime.tv_usec;
  us2 = res3.ru_utime.tv_usec - res2.ru_utime.tv_usec;
  printf("Without manual SSE: %ld seconds %06ld microseconds\n", s1, us1);
  printf("With    manual SSE: %ld seconds %06ld microseconds\n", s2, us2);
  printf("(%f times faster)\n",
         (s1 * 1000000.0f + us1) / (s2 * 1000000.0f + us2));

  maxdiff = 0;
  fullsize = img1.width * img1.height;
  for (i = 0; i < fullsize; ++i) {
    int32_t b1, b2, abs;
    b1 = img1.pixels[i].r;
    b2 = img2.pixels[i].r;
    abs = b1 > b2 ? b1 - b2 : b2 - b1;
    if (abs > maxdiff)
      maxdiff = abs;
    b1 = img1.pixels[i].g;
    b2 = img2.pixels[i].g;
    abs = b1 > b2 ? b1 - b2 : b2 - b1;
    if (abs > maxdiff)
      maxdiff = abs;
    b1 = img1.pixels[i].b;
    b2 = img2.pixels[i].b;
    abs = b1 > b2 ? b1 - b2 : b2 - b1;
    if (abs > maxdiff)
      maxdiff = abs;
  }

  printf("Maximal difference between pixels is %d\n", maxdiff);

  destroy_image(&img1);
  destroy_image(&img2);
  return 0;
}
