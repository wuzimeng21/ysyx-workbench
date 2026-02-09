#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define INT_TYPE 1
#define STR_TYPE 2


// ===================== stdout =========================
// TODO PA2 add: 2026.2.3
// stdlib.c has a atoi()
static char * itoa_klib(int num) {
    int i = 0;
    int is_negative = 0;
    static char str[32];
    // // char *ret = str;
    if (num == 0) {
      str[i ++] = '0';
      str[i] = '\0';
      return str;
    }
    
    // 处理负数（只支持十进制）
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    // 转换数字
    while (num != 0) {
        int remainder = num % 10;
        str[i] = (remainder + '0');
        i ++;
        num = num / 10;
    }
    if (is_negative) str[i ++] = '-';

    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
    // putstr(str);
    // putstr("\n");
    return str;
}

int vprintf(const char *fmt, va_list ap) {
  // 1) 将结果写入输出流 stdout。
  int t = 0, cnt = 0;
  char* p = "";

  while(*fmt != '\0') {
    if(*fmt == '%') {
      fmt ++;
      if(*fmt == 'd') {
        t = va_arg(ap, int);
        p = itoa_klib(t);
      }
      else if(*fmt == 's') {
        p = va_arg(ap, char*);
      }

      while(*p != '\0') {
        putch(*p);
        cnt ++;
        p ++;
      }
      p = "";
      fmt ++;
    }
    else {
      putch(*fmt);
      fmt ++, cnt ++;
    }
  }
  return cnt;
}

int printf(const char *fmt, ...) {
  // 1) 将结果写入输出流 stdout。
  va_list ap;
  int cnt;
  
  va_start(ap, fmt);
  cnt = vprintf(fmt, ap);
  va_end(ap);
  
  return cnt;
}

// ===================== buffer =========================
int vsprintf(char *out, const char *fmt, va_list ap) {
  // 3) 将结果写入字符字符串 buffer。
  // 1-3) 如果成功，写入的字符数；如果发生错误，则为负值。
  char *buffer = out;
  int i, cnt = 0, t = 0;

  while(*fmt != '\0') {
    if(*fmt == '%') {
      fmt ++;
      if(*fmt == 'd') {
        t = va_arg(ap, int);
        char * num_str = itoa_klib(t);
        while (*num_str != '\0') {
            *buffer = *num_str;
            cnt ++, buffer ++, num_str ++;
        }
      }
      else if(*fmt == 's') {
        char *p = "";
        p = va_arg(ap, char *);
        // putstr(p);
        while (*p != '\0') {
            *buffer = *p;
            cnt ++, buffer ++, p ++;
        }     
      } 
      fmt ++;
    }
    else {
      *buffer = *fmt;
      cnt ++, buffer ++, fmt ++;
    }
  }

  for(i = 0; i < cnt; i ++) putch(out[i]);

  return cnt;
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
  va_list ap;
  int cnt;
  
  va_start(ap, fmt);
  cnt = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  
  return cnt;
}


int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  // 4) 将结果写入字符字符串 out
  // 最多写入 n - 1 个字符。除非 n 为零，否则生成的字符串将以空字符终止。
  // 如果 n 为零，则不写入任何内容，
  // buffer 可以是空指针，但仍会计算并返回返回值（不包括空终止符的写入字节数）。
  // 4) 如果成功，写入的字符数；如果发生错误，则为负值。
  // 如果由于 buf_size 限制导致生成的字符串被截断，函数将返回将被写入的总字符数（不包括终止空字节）
  // 如果未施加限制的话。
    char *buffer = out;
    int cnt = 0;
    int num;
    
    if (n == 0 || buffer == NULL) return 0;
    
    size_t remaining = n - 1; // 为'\0'保留位置
    
    while (*fmt != '\0' && remaining > 0) {
        if (*fmt == '%') {
            fmt++;
            if(*fmt == 'd'){ // 整数
                num = va_arg(ap, int);
                char * num_str = itoa_klib(num);
                while (*num_str != '\0' && remaining > 0) {
                    *buffer ++ = *num_str ++;
                    cnt ++;
                    remaining --;
                }
            }
            else if(*fmt == 's'){ // 字符串
                char * p = va_arg(ap, char*);
                while (*p != '\0' && remaining > 0) {
                    *buffer ++ = *p ++;
                    cnt ++;
                    remaining --;
                }

            }
            else {
                *buffer ++ = *fmt;
                cnt ++;
                remaining --;
            }
            fmt++; 
        } else {
            *buffer ++ = *fmt ++;
            remaining --;
            cnt ++;
        }
    }
    
    *buffer = '\0';
        
    return cnt;
}

#endif
