#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

#define DBG(msg) write(2, msg, strlen(msg))

SDL_Surface* IMG_Load(const char *filename) {
  int img_fd = open(filename, 0, 0);
  if (img_fd < 0) {
    DBG("[IMG_Load] open FAILED: "); DBG(filename); DBG("\n");
    return NULL;
  }

  struct stat st;
  int size = 0;
  if (fstat(img_fd, &st) == 0) {
    size = st.st_size;
  }
  void *buf = malloc(size);
  if (buf == NULL) {
    DBG("[IMG_Load] malloc FAILED: "); DBG(filename); DBG("\n");
    close(img_fd);
    return NULL;
  }
  lseek(img_fd, 0, SEEK_SET);
  int nread = read(img_fd, buf, size);
  DBG("[IMG_Load] "); DBG(filename); DBG(" size=");
  static char numbuf[32];
  int i = 0, n = size;
  if (n == 0) { numbuf[i++] = '0'; }
  else { while (n) { numbuf[i++] = '0' + n % 10; n /= 10; } }
  for (int j = i - 1; j >= 0; j--) write(2, &numbuf[j], 1);
  DBG(" read=");
  i = 0, n = nread;
  if (n == 0) { numbuf[i++] = '0'; }
  else { while (n) { numbuf[i++] = '0' + n % 10; n /= 10; } }
  for (int j = i - 1; j >= 0; j--) write(2, &numbuf[j], 1);
  DBG("\n");
  SDL_Surface *surface = STBIMG_LoadFromMemory(buf, size);
  close(img_fd);
  free(buf);
  return surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
