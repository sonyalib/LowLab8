#include "bmp.h"
#include "image.h"
#include "log.h"
#include <stdint.h>
#include <stdio.h>

#define _BITMAP_HEADER                                                         \
  E(uint16_t, bfType)                                                          \
  E(uint32_t, bfSize)                                                          \
  E(uint16_t, bfReserved1)                                                     \
  E(uint16_t, bfReserved2)                                                     \
  E(uint32_t, bfOffBits)                                                       \
  E(uint32_t, biSize)                                                          \
  E(int32_t, biWidth)                                                          \
  E(int32_t, biHeight)                                                         \
  E(uint16_t, biPlanes)                                                        \
  E(uint16_t, biBitCount)                                                      \
  E(uint32_t, biCompression)                                                   \
  E(uint32_t, biSizeImage)                                                     \
  E(int32_t, biXPelsPerMeter)                                                  \
  E(int32_t, biYPelsPerMeter)                                                  \
  E(uint32_t, biClrUsed)                                                       \
  E(uint32_t, biClrImportant)

struct BitmapHeader {
#define E(t, n) t n;
  _BITMAP_HEADER
#undef E
};

#define _RGB_QUAD                                                              \
  E(uint8_t, rgbBlue)                                                          \
  E(uint8_t, rgbGreen)                                                         \
  E(uint8_t, rgbRed)                                                           \
  E(uint8_t, rgbReserved)

enum BmpLoadResult from_bmp(struct ImageRgb8 *img, FILE *f) {
  struct BitmapHeader header;
  char buf[256];
  size_t col, additional_bytes_per_row;
#define E(t, n)                                                                \
  fread(&header.n, sizeof(t), 1, f);                                           \
  sprintf(buf, #n ": %d", header.n);                                           \
  log_info(buf);
  _BITMAP_HEADER
#undef E

  if ((header.bfType & 0xFF) != 'B' || ((header.bfType >> 8) & 0xFF) != 'M') {
    log_err("bfType should be 'BM'");
    return BMP_LOAD_FORMAT_ERROR;
  }

  if (header.bfReserved1 || header.bfReserved2) {
    log_err("bfReserved1 and bfReserved2 should be zero");
    return BMP_LOAD_FORMAT_ERROR;
  }

  if (header.biPlanes != 1) {
    log_err("biPlanes should be 1");
    return BMP_LOAD_FORMAT_ERROR;
  }

  if (header.biBitCount != 24) {
    log_err("Only 24 bits per pixel are supported");
    return BMP_LOAD_UNSUPPORTED_FORMAT;
  }

  if (fseek(f, header.bfOffBits, SEEK_SET)) {
    log_err("Fseek error");
    return BMP_LOAD_READ_ERROR;
  }

  img->width = header.biWidth;
  img->height = header.biHeight;
  if (create_image(img)) {
    log_err("Could not allocate memory for the image");
    return BMP_LOAD_MEMORY_ERROR;
  }

  additional_bytes_per_row = (img->width * header.biBitCount + 31) / 32 * 4 -
                             img->width * sizeof(struct Rgb8);
  for (col = 0; col < img->height; ++col) {
    if (fread(img->pixels + col * img->width, sizeof(struct Rgb8), img->width,
              f) != img->width) {
      log_err("Could not read enough data");
      return BMP_LOAD_READ_ERROR;
    }
    if (fseek(f, additional_bytes_per_row, SEEK_CUR)) {
      log_err("Fseek error");
      return BMP_LOAD_READ_ERROR;
    }
  }

  return BMP_LOAD_OK;
}

enum BmpSaveResult to_bmp(struct ImageRgb8 const *img, FILE *f) {
  struct BitmapHeader header;
  size_t additional_bytes_per_row, bytes_per_row, col, header_size;
  char garbage[4] = "abc";
  char buf[256];

  bytes_per_row = img->width * sizeof(struct Rgb8);
  if (bytes_per_row % 4) {
    additional_bytes_per_row = 4 - bytes_per_row % 4;
    bytes_per_row += additional_bytes_per_row;
  } else {
    additional_bytes_per_row = 0;
  }

  header_size = 0;
#define E(t, n) header_size += sizeof(t);
  _BITMAP_HEADER
#undef E

  header.bfType = 'B' | ('M' << 8);
  header.bfSize = img->height * bytes_per_row + header_size;
  header.bfReserved1 = 0;
  header.bfReserved2 = 0;
  header.bfOffBits = header_size;
  header.biSize = 40;
  header.biWidth = img->width;
  header.biHeight = img->height;
  header.biPlanes = 1;
  header.biBitCount = 24;
  header.biCompression = 0;
  header.biSizeImage = img->width * img->height * sizeof(struct Rgb8);
  header.biXPelsPerMeter = 3780;
  header.biYPelsPerMeter = 3780;
  header.biClrUsed = 0;
  header.biClrImportant = 0;

#define E(t, n)                                                                \
  sprintf(buf, #n ": %d", header.n);                                           \
  log_info(buf);
  _BITMAP_HEADER
#undef E

#define E(t, n)                                                                \
  if (fwrite(&header.n, sizeof(t), 1, f) != 1) {                               \
    log_err("Could not write enough data");                                    \
    return BMP_SAVE_WRITE_ERROR;                                               \
  };
  _BITMAP_HEADER
#undef E

  for (col = 0; col < img->height; ++col) {
    size_t successful_write = fwrite(img->pixels + col * img->width,
                                     sizeof(struct Rgb8), img->width, f);
    if (successful_write != img->width) {
      log_err("Could not write enough data");
      fprintf(stderr, "%ld %ld\n", successful_write, img->width);
      return BMP_SAVE_WRITE_ERROR;
    }
    if (fwrite(garbage, 1, additional_bytes_per_row, f) !=
        additional_bytes_per_row) {
      log_err("Could not write enough data");
      return BMP_SAVE_WRITE_ERROR;
    }
  }

  return BMP_SAVE_OK;
}

enum BmpLoadResult from_bmp_path(struct ImageRgb8 *img, char const *path) {
  FILE *f;
  enum BmpLoadResult r;

  log_info("Loading BMP:");
  log_info(path);

  f = fopen(path, "rb");
  if (!f) {
    log_err("Could not open file");
    return BMP_LOAD_FOPEN_ERROR;
  }
  r = from_bmp(img, f);
  fclose(f);
  return r;
}

enum BmpSaveResult to_bmp_path(struct ImageRgb8 const *img, char const *path) {
  FILE *f = fopen(path, "wb");
  enum BmpSaveResult r;

  log_info("Saving BMP:");
  log_info(path);

  if (!f) {
    log_err("Could not open file");
    return BMP_SAVE_FOPEN_ERROR;
  }
  r = to_bmp(img, f);
  fclose(f);
  return r;
}
