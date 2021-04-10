#include "image.h"

#include <stdlib.h>

int create_image(struct ImageRgb8 *image) {
  image->pixels = malloc(image->width * image->height * sizeof(struct Rgb8));
  return image->pixels ? 0 : 1;
}

void destroy_image(struct ImageRgb8 *image) { free(image->pixels); }

int image_copy(struct ImageRgb8 const *src, struct ImageRgb8 *dst) {
  int err;
  size_t i, total = src->width * src->height;
  struct Rgb8 const *psrc;
  struct Rgb8 *pdst;
  dst->width = src->width;
  dst->height = src->height;
  err = create_image(dst);
  if (err)
    return err;
  psrc = src->pixels;
  pdst = dst->pixels;
  for (i = 0; i < total; ++i)
    *pdst++ = *psrc++;
  return 0;
}

static void swap_rb(struct ImageRgb8 *img) {
  struct Rgb8 *ptr = img->pixels;
  size_t total = img->width * img->height;
  size_t i;
  for (i = 0; i < total; ++i) {
    uint8_t t = ptr->r;
    ptr->r = ptr->b;
    ptr->b = t;
  }
}

void image_rgb_to_bgr(struct ImageRgb8 *img) { swap_rb(img); }
void image_bgr_to_rgb(struct ImageRgb8 *img) { swap_rb(img); }

int image_transpose(struct ImageRgb8 const *src, struct ImageRgb8 *dst) {
  int err;
  size_t x, y;
  dst->width = src->height;
  dst->height = src->width;
  err = create_image(dst);
  if (err)
    return err;
  for (y = 0; y < src->height; ++y) {
    for (x = 0; x < src->width; ++x) {
      dst->pixels[x * src->height + y] = src->pixels[y * src->width + x];
    }
  }
  return 0;
}

void image_mirror_h(struct ImageRgb8 *img) {
  size_t x, y;
  for (y = 0; y < img->height; ++y) {
    for (x = 0; x + x + 1 < img->width; ++x) {
      size_t i = y * img->width + x;
      size_t j = y * img->width + img->width - x - 1;
      struct Rgb8 t = img->pixels[i];
      img->pixels[i] = img->pixels[j];
      img->pixels[j] = t;
    }
  }
}

void image_mirror_v(struct ImageRgb8 *img) {
  size_t x, y;
  for (y = 0; y + y + 1 < img->height; ++y) {
    for (x = 0; x < img->width; ++x) {
      size_t i = y * img->width + x;
      size_t j = (img->height - y - 1) * img->width + x;
      struct Rgb8 t = img->pixels[i];
      img->pixels[i] = img->pixels[j];
      img->pixels[j] = t;
    }
  }
}

int image_rotate_clockwise(struct ImageRgb8 const *src, struct ImageRgb8 *dst) {
  int err;
  err = image_transpose(src, dst);
  if (err)
    return err;
  image_mirror_h(dst);
  return 0;
}

int image_rotate_counter_clockwise(struct ImageRgb8 const *src,
                                   struct ImageRgb8 *dst) {
  int err;
  err = image_transpose(src, dst);
  if (err)
    return err;
  image_mirror_v(dst);
  return 0;
}

void image_rotate_180(struct ImageRgb8 *img) {
  /* Оптимизировать можно, но асимптотика не поменяется */
  image_mirror_h(img);
  image_mirror_v(img);
}
