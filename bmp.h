#ifndef _BMP_H_
#define _BMP_H_

#include <stdio.h>

struct ImageRgb8;

enum BmpLoadResult {
  BMP_LOAD_OK = 0,
  BMP_LOAD_FORMAT_ERROR,
  BMP_LOAD_UNSUPPORTED_FORMAT,
  BMP_LOAD_READ_ERROR,
  BMP_LOAD_MEMORY_ERROR,
  BMP_LOAD_FOPEN_ERROR
};

enum BmpSaveResult {
  BMP_SAVE_OK = 0,
  BMP_SAVE_WRITE_ERROR,
  BMP_SAVE_FOPEN_ERROR
};

enum BmpLoadResult from_bmp(struct ImageRgb8 *, FILE *);
enum BmpSaveResult to_bmp(struct ImageRgb8 const *, FILE *);

enum BmpLoadResult from_bmp_path(struct ImageRgb8 *, char const *);
enum BmpSaveResult to_bmp_path(struct ImageRgb8 const *, char const *);

#endif
