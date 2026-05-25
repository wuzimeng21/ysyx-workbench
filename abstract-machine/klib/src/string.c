#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  // NAME
  //        strlen - calculate the length of a string
  // SYNOPSIS
  //        #include <string.h>
  //        size_t strlen(const char *s);
  // DESCRIPTION
  //        The strlen() function calculates the length of the string pointed to by
  //        s, excluding the terminating null byte ('\0').
  // RETURN VALUE
  //        The strlen() function returns the number of bytes in the string pointed
  //        to by s.
  size_t l = 0;
  while(s[l] != '\0'){
    l ++;
  }
  return l;
}

char *strcpy(char *dst, const char *src) {
    char * ret = dst;
    if(dst == NULL || src == NULL) return dst;
    while (*src != '\0'){
      *dst = *src;
      dst ++, src ++;
    }
    * dst = '\0';
    return ret;
}

char *strncpy(char *dst, const char *src, size_t n) {
  // 与 (1) 相同，但函数不再将零写入目标数组以填充到 count，
  // 而是在写入终止空字符后停止（如果源中没有空字符，它会在 dest[count] 处写入一个然后停止）。
  size_t i;
  char * ret = dst;
  for (i = 0; i < n && src[i] != '\0'; i++) {
      dst[i] = src[i];
  }
  for ( ; i < n; i++)
      dst[i] = '\0';
  
  return ret;
}

char *strcat(char *dst, const char *src) {
  // DESCRIPTION
  //      The  strcat() function appends the src string to the dest string, over‐
  //      writing the terminating null byte ('\0') at the end of dest,  and  then
  //      adds  a  terminating  null  byte.  The strings may not overlap, and the
  //      dest string must have enough space for the  result.   If  dest  is  not
  //      large  enough, program behavior is unpredictable; buffer overruns are a
  //      favorite avenue for attacking secure programs.
  //      The strncat() function is similar, except that
  //      *  it will use at most n bytes from src; and
  //      *  src does not need to be null-terminated if it  contains  n  or  more
  //         bytes.
  //      As  with  strcat(),  the resulting string in dest is always null-termi‐
  //      nated.
  //      If src contains n or more bytes, strncat() writes n+1 bytes to dest  (n
  //      from  src plus the terminating null byte).  Therefore, the size of dest
  //      must be at least strlen(dest)+n+1.
// 1) 将 src 指向的以 null 结尾的字节字符串的副本附加到 dest 指向的以 null 结尾的字节字符串的末尾。
// 字符 src[0] 替换 dest 末尾的 null 终止符。结果字节字符串以 null 结尾。
//  如果目标数组不足以容纳 src 和 dest 的内容以及终止 null 字符，则行为是未定义的。
//  如果字符串重叠，则行为是未定义的。
//  如果 dest 或 src 都不是指向以 null 结尾的字节字符串的指针，则行为是未定义的。  
  char * ret = dst;
  // size_t l = strlen(dst), L = sizeof(dst) / sizeof(char), i = 0;
  // size_t l_left = L - l;
  if(dst == NULL || src == NULL) return ret;
  while(*dst != '\0') {
    dst ++;
  }

  while(*src != '\0') {
    *dst = *src;
    src ++, dst ++;
    // i ++;
  }
  *dst = '\0';

  return ret;

}

int strcmp(const char *s1, const char *s2) {

  // DESCRIPTION
  //      The  strcmp()  function compares the two strings s1 and s2.  The locale
  //      is not taken into account (for  a  locale-aware  comparison,  see  str‐
  //      coll(3)).  The comparison is done using unsigned characters.
  //      strcmp() returns an integer indicating the result of the comparison, as
  //      follows:
  //      • 0, if the s1 and s2 are equal;
  //      • a negative value if s1 is less than s2;
  //      • a positive value if s1 is greater than s2.
  //      The strncmp() function is similar, except it compares  only  the  first
  //      (at most) n bytes of s1 and s2.
  size_t l_s1, l_s2, l, i = 0;
  l_s1 = strlen(s1);
  l_s2 = strlen(s2);
  l = (l_s1 < l_s2) ? l_s1 : l_s2;
  while(i < l) {
    if(s1[i] > s2[i]) return s1[i] - s2[i];
    else if(s1[i] < s2[i]) return s1[i] - s2[i];
    i ++;
  }
  if(l_s1 > l) return 1;
  else if(l_s2 > l) return -1;

  return 0;
}



int strncmp(const char *s1, const char *s2, size_t n) {
    // 比较前 n 个字符
    unsigned char c1;
    unsigned char c2;

    for (size_t i = 0; i < n; i++) {
        c1 = (unsigned char)s1[i];
        c2 = (unsigned char)s2[i];        
        if (c1 != c2) {
            return (int)c1 - (int)c2;
        }
        // 如果到达字符串结尾，它们相等
        if (c1 == '\0') {
            return 0;
        }
    }
    // 前 n 个字符都相等
    return 0;
}

void *memset(void *s, int c, size_t n) {
  size_t i;
  unsigned char *p1 = s;
  if(p1 == NULL) return s;
  for(i = 0; i < n; i ++) {
    *p1 =  (unsigned char)c;
    p1 ++;
  }
  return s;
}


void *memmove(void *dst, const void *src, size_t n) {
  // memmove 干了三件事：
  // 复制内存块：从 src 复制 n 个字节到 dst
  // 检查重叠：比较 dst 和 src 的地址
  // 选择方向：
  // 如果 dst < src：从前往后复制
  // 如果 dst > src：从后往前复制（避免覆盖）
  // 如果 dst == src：什么都不做
    if (dst == NULL || src == NULL || n == 0) {
        return dst;
    }
    
    unsigned char *d = dst;
    const unsigned char *s = src;
    
    // 相同地址或长度为0，直接返回
    if (d == s) {
        return dst;
    }
    
    // 检查重叠并决定复制方向
    if (d < s) {
        // 目标在源前面，从前往后复制
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        // 目标在源后面，从后往前复制（避免覆盖）
        for (size_t i = n; i > 0; ) {
          i --;
          d[i] = s[i];
        }
    }
    return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  // NAME
  //        memcpy - copy memory area
  // SYNOPSIS
  //        #include <string.h>
  //        void *memcpy(void *dest, const void *src, size_t n);
  // DESCRIPTION
  //        The  memcpy()  function  copies  n bytes from memory area src to memory
  //        area dest.  The memory areas must not overlap.  Use memmove(3)  if  the
  //        memory areas do overlap.
  // RETURN VALUE
  //        The memcpy() function returns a pointer to dest.

  // 未能遵守内存区域不重叠的要求已导致重大bug。
  // （POSIX和C标准明确规定，对重叠区域使用memcpy()会产生未定义行为。）
  // 最值得注意的是，在glibc 2.13中，
  // 某些平台（包括x86-64）对memcpy()的性能优化包含了改变从src到dest复制字节的顺序。
  // 这一改动揭示了许多在重叠区域进行复制的应用程序中的缺陷。
  // 在之前的实现中，字节复制的顺序恰好隐藏了这些bug，
  // 而当复制顺序被反转时，这些bug就暴露出来了。
  // 在glibc 2.14中，添加了一个版本化符号，使得旧的二进制文件
  // （即链接到glibc 2.14之前版本的）使用能够安全处理重叠缓冲区情况的memcpy()实现（
  // 通过提供一个"较旧"的memcpy()实现，该实现被别名为memmove(3)）。
  size_t i;
  unsigned char *p1 = out;
  const unsigned char *p2 = in;

  if(p1 == p2) return out;
  for(i = 0; i < n; i ++) {
    *p1 ++ = *p2 ++;
  }

  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i = 0;
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;
  if(n == 0) return 0;

  while(i < n) {
    if(*p1 < *p2) return -1;
    else if(*p1 > *p2) return 1;
    p1 ++, p2 ++, i ++;
  }
  return 0;
}

#endif
