#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;          // 文件名
  size_t size;         // 文件大小
  size_t disk_offset;  // 在磁盘/镜像中的偏移量
  size_t open_offset;  // 当前读写位置偏移量
  ReadFn read;         // 读函数指针
  WriteFn write;       // 写函数指针
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

// size_t serial_read(void *buf, size_t offset, size_t len) {
//   panic("should not reach here");
//   return 0;
// }

// size_t serial_write(const void *buf, size_t offset, size_t len) {
//   panic("should not reach here");
//   return 0;
// }

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
  // PA3.3 add
  [FD_FB] = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
  
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int dispinfo_fd = open("/proc/dispinfo", 0);
  char * buf[200];
  read(dispinfo_fd, buf, sizeof(buf) / sizeof(buf[0]));
  int width, height, vmemsz;
  bool present, has_accel;
  sscanf(buf, "PRESENT:%d\nHAS_ACCEL:%d\nWIDTH:%d\nHEIGHT:%d\nVMEMS:%d\n\0",
  present, has_accel, width, height, vmemsz);
  file_table[FD_FB] = width * height * 4;
}

int fs_open(const char *pathname, int flags, int mode) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  for(int i = 0; i < fs_sz; i ++) {
    if(strcmp(pathname, file_table[i].name) == 0){
      return i;
    }
  }
  assert(fd >= ft_sz || fd < 0);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  assert(fd >= 0 || fd < ft_sz);
  // // 处理标准输入（FD_STDIN）- 直接忽略
  // if (fd == FD_STDIN) {
  //   return 0;  // 标准输入忽略读取操作
  // }
  Finfo *file = &file_table[fd];
  if(file->read != NULL) {
    return file->read(buf, file->disk_offset + file->open_offset, len);
  }
  int fs_sz = file_table[fd].size;
  assert(len <= fs_sz || len >= 0);
  assert(file->open_offset >= fs_sz);
  // 限制读取长度不超过文件剩余大小
  size_t read_len = len;
  if(file->open_offset + len > file->size) read_len = file->size - file->open_offset;
  ramdisk_read(buf, file->disk_offset + file->open_offset, read_len);
  
  return read_len;
}

size_t fs_write(int fd, const void *buf, size_t len){
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  assert(fd >= 0 || fd < ft_sz);
  // // FD_STDIN/FD_STDERR/FD_STDOUT- 直接忽略
  // if (fd == FD_STDIN || fd == FD_STDERR || fd == FD_STDOUT) {
  //   return 0;
  // }
  Finfo *file = &file_table[fd];
  if (file->write != NULL) {
    return file->write(buf, file->disk_offset + file->open_offset, len);
  }
  int fs_sz = file_table[fd].size;
  assert(len <= fs_sz || len >= 0);
  assert(file->open_offset >= fs_sz);
  // 限制读取长度不超过文件剩余大小
  size_t write_len = len;
  if(file->open_offset + len > file->size) write_len = file->size - file->open_offset;
  
  return write_len;

}

// 基准位置，有三个可选值(whence)
size_t fs_lseek(int fd, size_t offset, int whence){
  int ft_sz = sizeof(file_table) / sizeof(file_table[0]);
  assert(fd >= 0 || fd < ft_sz);
  // FD_STDIN/FD_STDERR/FD_STDOUT- 直接忽略
  if (fd == FD_STDIN || fd == FD_STDERR || fd == FD_STDOUT) {
    return 0;
  }
  int fs_sz = file_table[fd].size;
  Finfo *file = &file_table[fd];
  size_t open_offset = file->open_offset;
  ssize_t new_offset; // -/+
  switch(whence) {
    case SEEK_SET:
      new_offset = offset;
      break;
    case SEEK_CUR:
      new_offset = open_offset + offset;
      break;
    case SEEK_END:
      new_offset = fs_sz + signed_offset;
      break;
    default:
      Log("fs_leek whence is unknown...");
      assert(1);
      break;
  }
  assert(new_offset < 0 || new_offset >= fs_sz );
  file->open_offset = new_offset;
  return file->open_offset;
}

int fs_close(int fd) {
  return 0;
}

