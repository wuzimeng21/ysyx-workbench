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
  // NAME
  //        strcpy, strncpy - copy a string

  // SYNOPSIS
  //        #include <string.h>

  //        char *strcpy(char *dest, const char *src);

  //        char *strncpy(char *dest, const char *src, size_t n);

  // DESCRIPTION
  //        The  strcpy()  function  copies the string pointed to by src, including
  //        the terminating null byte ('\0'), to the buffer  pointed  to  by  dest.
  //        The  strings  may  not overlap, and the destination string dest must be
  //        large enough to receive the copy.  Beware  of  buffer  overruns!   (See
  //        BUGS.)

  //        The  strncpy()  function is similar, except that at most n bytes of src
  //        are copied.  Warning: If there is no null byte among the first n  bytes
  //        of src, the string placed in dest will not be null-terminated.
  // If  the  length of src is less than n, strncpy() writes additional null
  // bytes to dest to ensure that a total of n bytes are written.
  //  如果 dest 数组不够大，则行为未定义。
  //  如果字符串重叠，则行为未定义。
  //  如果 dest 不是指向字符数组的指针，或者 src 不是指向以空字符结尾的字节字符串的指针，则行为未定义。
    char * ret = dst;
    if(dst == NULL || src == NULL) return dst;

    while (*src != '\0'){
      *ret ++ = *src ++;
    }
    * ret = '\0';
    return ret;

}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i, src_len, dst_len;

  src_len = strlen(src);
  dst_len = strlen(src);
  if (src_len >= n) {}
  else if(src_len < n) {}

  for (i = 0; i < n && src[i] != '\0'; i++)
      dst[i] = src[i];
  for ( ; i < n; i++)
      dst[i] = '\0';

  return dst;
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
  // 将最多 count 个字符从 src 指向的字符数组追加到 dest 指向的以空字符结尾的字节字符串的末尾，
  // 如果在 src 中遇到空字符则停止。字符 src[0] 替换 dest 末尾的空终止符。
  // 终止空字符始终附加在末尾（因此函数最多可以写入 count+1 个字节）。
  // 如果目标数组没有足够的空间容纳 dest 的内容和 src 的前 count 个字符，以及终止空字符，则行为是未定义的。
  // 如果源对象和目标对象重叠，则行为是未定义的。
  // 如果 dest 不是指向以空字符结尾的字节字符串的指针，或者 src 不是指向字符数组的指针，则行为是未定义的。
  char * ret = dst;
  size_t i = 0;
  while(*ret != '\0') {
    ret ++;
  }

  while(*src != '\0') {
    *ret = *src;
    src ++;
  }

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
  l = min(l_s1, l_s2);
  while(i < l) {
    if(s1[i] > s2[i]) return 1;
    else iff(s1[i] < s2[i]) return -1;
    i ++;
  }
  if(l_s1 > l) return 1;
  else iff(l_s2 > l) return -1;

  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  // NAME
  //        memset - fill memory with a constant byte
  // SYNOPSIS
  //        #include <string.h>
  //        void *memset(void *s, int c, size_t n);
  // DESCRIPTION
  //        The  memset()  function  fills  the  first  n  bytes of the memory area
  //        pointed to by s with the constant byte c.
  // RETURN VALUE
  //        The memset() function returns a pointer to the memory area s.
  size_t i;
  if(s == NULL || n == 0) return s;
  for(i = 0; i < n; i ++) {
    s[i] = (char)c;
  }
  s[i] = '\0';

  return s;

}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
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
  if(in == out) memmove(out, in, n);

  for(i = 0; i < n; i ++) {
    out[i] = in[i];
  }


  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  // DESCRIPTION
  //        The  memcmp()  function compares the first n bytes (each interpreted as
  //        unsigned char) of the memory areas s1 and s2.

  // RETURN VALUE
  //        The memcmp() function returns  an  integer  less  than,  equal  to,  or
  //        greater than zero if the first n bytes of s1 is found, respectively, to
  //        be less than, to match, or be greater than the first n bytes of s2.

  //        For a nonzero return value, the sign is determined by the sign  of  the
  //        difference  between  the  first  pair of bytes (interpreted as unsigned
  //        char) that differ in s1 and s2.

  //        If n is zero, the return value is zero.
  // 比较内存区域：比较 s1 和 s2 的前 n 个字节
  // 按无符号字符解释：每个字节都当作 unsigned char（0-255）
  // 比较规则：逐字节比较，直到找到不同或比较完 n 个字节
  // 返回值：
  // < 0：s1 的前 n 字节 小于 s2
  // = 0：s1 的前 n 字节 等于 s2
  // > 0：s1 的前 n 字节 大于 s2
  // 关键特性：
  // 按字节比较，不是按其他类型（如 int）
  // 无符号比较：(unsigned char)s1[i] - (unsigned char)s2[i]
  // n=0 时返回 0：零长度区域被认为相等
  size_t i = 0;
  if(n == 0) return 0;
  while(i < n) {
    if(*s1 < *s2) return -1;
    else if(*s1 > *s2) return 1;
  }
  
  return 0;

}

#endif
