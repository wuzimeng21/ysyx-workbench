#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define INT_TYPE 1
#define STR_TYPE 2


// ===================== stdout =========================
// TODO PA2 add: 2026.2.3
int vprintf(const char *fmt, ...) {
  // 1) 将结果写入输出流 stdout。
  va_list ap;
  // char *s = fmt;
  int t = 0, cnt = 0;
  const char* p = "";
  va_start(ap, fmt);

  while(*fmt != '\0') {
    if(*fmt ==  '%') {
      fmt ++;
      if(*fmt == 'd') {
        t = va_arg(ap, int);
        p = (const char *)(t + '0');
      }
      else if(*fmt == 's') {
        p = va_arg(ap, const char*);
      }
      // putch(p);
      while(*p ++ != '\0') putch(*p);
      // putstr(p);
    }
    else putch(*fmt);
    cnt ++;
    fmt ++;
  }
  va_end(ap);

  return cnt;
}

int printf(const char *fmt, ...) {
  // 1) 将结果写入输出流 stdout。
  int res = vprintf(fmt);
  return res;
}

// ===================== buffer =========================
int vsprintf(char *out, const char *fmt, va_list ap) {
  // 3) 将结果写入字符字符串 buffer。
  // 1-3) 如果成功，写入的字符数；如果发生错误，则为负值。
  // char *buffer = out;
  // int i, cnt = 0, t = 0, buffer_sz = sizeof(* out) / sizeof(char);
  // const char * p = "";

  // // va_start(ap) ; // va_copy???

  // while(*fmt != '\0' && cnt < buffer_sz - 1) {
  //   if(*fmt == '%') {
  //     fmt ++;
  //     if(*fmt == 'd') {
  //       t = va_arg(ap, int);
  //       // buffer[cnt] = (t + '0');
  //       *buffer = (t + '0');
  //     }
  //     else if(*fmt == 's') {
  //       p = va_arg(ap, const char*);
  //       // buffer[cnt] = p;
  //       *buffer = *p;
  //     } 
  //   }
  //   buffer ++;
  //   cnt ++;
  //   fmt ++;
  // }

  // // va_end(ap);
  // if(cnt == buffer_sz - 1) buffer[buffer_sz - 1] = '\0';

  // for(i = 0; i < cnt; i ++) {
  //   // putch(buffer[i]);  // output buffer
  //   while(*buffer[i] ++ != '\0') putch(*buffer[i]);
  // }

  return 0;
}

int sprintf(char *out, const char *fmt, ...) {
  // 3) 将结果写入字符字符串 buffer。
  // 如果待写入的字符串（加上终止空字符）超出 buffer 指向的数组大小，则行为未定义。
  // 3) 写入 buffer 的字符数（不包括终止空字符），
  // 如果发生编码错误（对于字符串和字符转换说明符），则为负值。
  va_list ap;
  va_start(ap, fmt);
  int res = vsprintf(out, fmt, ap);
  va_end(ap);

  return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  // 将结果写入字符字符串 buffer。
  // 最多写入 bufsz - 1 个字符。
  // 除非 bufsz 为零，否则生成的字符字符串将以空字符终止。
  // 如果 bufsz 为零，则不写入任何内容，
  // buffer 可以是空指针，但仍会计算并返回返回值（不包括空终止符的写入字节数）。
  panic("Not implemented");
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  // 4) 将结果写入字符字符串 out
  // 最多写入 n - 1 个字符。除非 n 为零，否则生成的字符串将以空字符终止。
  // 如果 n 为零，则不写入任何内容，
  // buffer 可以是空指针，但仍会计算并返回返回值（不包括空终止符的写入字节数）。
  // 4) 如果成功，写入的字符数；如果发生错误，则为负值。
  // 如果由于 buf_size 限制导致生成的字符串被截断，函数将返回将被写入的总字符数（不包括终止空字节）
  // 如果未施加限制的话。
  int cnt = 0;
  
  return cnt;
}

#endif
