#include <assert.h>
#include <stdlib.h>
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

SDL_Surface* IMG_Load(const char *filename) {
  int img_fd = open(filename, 0, 0);
  if (img_fd < 0) return NULL;

  struct stat st;
  int size = 0;
  if (fstat(img_fd, &st) == 0) {
    size = st.st_size;
  }
  void *buf = malloc(size);
  lseek(img_fd, 0, SEEK_SET);
  read(img_fd, buf, size);
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
