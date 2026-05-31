#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]   = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT]  = {"stdout", 0, 0, 0, invalid_read, serial_write},
  [FD_STDERR]  = {"stderr", 0, 0, 0, invalid_read, serial_write},
  [FD_FB]      = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  [FD_EVENTS]  = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  AM_GPU_CONFIG_T g = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = g.width * g.height * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  for (int i = 0; i < ft_sz; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;  // 重置文件偏移
      return i;
    }
  }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  assert(fd >= 0 && fd < ft_sz);
  Finfo *file = &file_table[fd];
  if (file->read != NULL) {
    return file->read(buf, file->disk_offset + file->open_offset, len);
  }
  size_t read_len = len;
  if (file->open_offset + len > file->size)
    read_len = file->size - file->open_offset;
  ramdisk_read(buf, file->disk_offset + file->open_offset, read_len);
  file->open_offset += read_len;
  return read_len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  assert(fd >= 0 && fd < ft_sz);
  Finfo *file = &file_table[fd];
  if (file->write != NULL) {
    return file->write(buf, file->disk_offset + file->open_offset, len);
  }
  size_t write_len = len;
  if (file->open_offset + len > file->size)
    write_len = file->size - file->open_offset;
  ramdisk_write(buf, file->disk_offset + file->open_offset, write_len);
  file->open_offset += write_len;
  return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  assert(fd >= 0 && fd < ft_sz);
  Finfo *file = &file_table[fd];
  switch (whence) {
    case SEEK_SET: file->open_offset = offset; break;
    case SEEK_CUR: file->open_offset += offset; break;
    case SEEK_END: file->open_offset = file->size + offset; break;
    default: panic("fs_lseek: unknown whence %d", whence);
  }
  return file->open_offset;
}

int fs_close(int fd) {
  return 0;
}

int fs_fstat(int fd, void *buf) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  if (fd < 0 || fd >= ft_sz) return -1;
  memset(buf, 0, 64);
  *(uint32_t *)((char *)buf + 4) = 0x8000; // st_mode = S_IFREG
  *(uint32_t *)((char *)buf + 16) = file_table[fd].size; // st_size
  Log("fs_fstat: fd=%d name=%s size=%d", fd, file_table[fd].name, file_table[fd].size);
  return 0;
}
