#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>

#include <stdio.h>

int
myputc(int c)
{
  putchar(c);
}

int
myvprintf(const char *fmt, va_list ap)
{
  size_t strlen(const char *);
  union { intmax_t i; uintmax_t u; long double f; void *p; } u;
  int flags, width, precision = -1, i;
  char c, buf[256], *bufp, *bufb;
  enum {
    ALT = 1,
    ZERO = 2,
    LEFT = 4,
    SIGN = 8,
    SPACE = 16,
  };

  while ((c = *fmt++)) {
    if (c != '%') {
      myputc(c);
      continue;
    } else {
      c = *fmt++;
      goto FLAGS;
    }

  FLAGS:
    flags = 0;
    do {
      switch (c) {
      case '#':
        flags |= ALT;
        break;
      case '0':
        flags |= ZERO;
        break;
      case '+':
        flags |= SIGN;
        break;
      case '-':
        flags |= LEFT;
        break;
      case ' ':
        flags |= SPACE;
        break;
      default:
        goto WIDTH;
      }
    } while ((c = *fmt++));

  WIDTH:
    width = 0;
    do {
      switch (c) {
      case '*':
        width = va_arg(ap, int);
        c = *fmt++;
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        width = width * 10 + c - '0';
        break;
      default:
        goto PRECISION;
      }
    } while ((c = *fmt++));

  PRECISION:
    if (c == '.') {
      precision = 0;
      while ((c = *fmt++)) {
        switch (c) {
        case '*':
          precision = va_arg(ap, int);
          c = *fmt++;
          break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          precision = precision * 10 + c - '0';
          break;
        default:
          goto LENGTH;
        }
      }
    }

  LENGTH:
    switch (c) {
    case 'h':
      if (fmt[1] == 'h') {
        fmt++;
        switch (c = *fmt++) {
        case 'd': case 'i':
          u.i = va_arg(ap, int);
          goto SPECIFIER;
        case 'u': case 'o': case 'x': case 'X':
          u.u = va_arg(ap, unsigned int);
          goto SPECIFIER;
        default:
          return -1;
        }
      } else {
        switch (c = *fmt++) {
        case 'd': case 'i':
          u.i = va_arg(ap, int);
          goto SPECIFIER;
        case 'u': case 'o': case 'x': case 'X':
          u.u = va_arg(ap, unsigned int);
          goto SPECIFIER;
        default:
          return -1;
        }
      }
      break;
    case 'l':
      if (fmt[1] == 'l') {
        fmt++;
        switch (c = *fmt++) {
        case 'd': case 'i':
          u.i = va_arg(ap, long int);
          goto SPECIFIER;
        case 'u': case 'o': case 'x': case 'X':
          u.u = va_arg(ap, unsigned long int);
          goto SPECIFIER;
        case 'c':
        case 's':
          /* TODO */
        default:
          return -1;
        }
      } else {
        switch (c = *fmt++) {
        case 'd': case 'i':
          u.i = va_arg(ap, long long int);
          goto SPECIFIER;
        case 'u': case 'o': case 'x': case 'X':
          u.u = va_arg(ap, unsigned long long int);
          goto SPECIFIER;
        default:
          return -1;
        }
      }
      break;
    case 'j':
      switch (c = *fmt++) {
      case 'd': case 'i':
        u.i = va_arg(ap, intmax_t);
        goto SPECIFIER;
      case 'u': case 'o': case 'x': case 'X':
        u.u = va_arg(ap, uintmax_t);
        goto SPECIFIER;
      default:
        return -1;
      }
      break;
    case 'z':
      switch (c = *fmt++) {
      case 'd': case 'i':
        u.i = va_arg(ap, size_t);
        goto SPECIFIER;
      case 'u': case 'o': case 'x': case 'X':
        u.u = va_arg(ap, size_t);
        goto SPECIFIER;
      default:
        return -1;
      }
      break;
    case 't':
      switch (c = *fmt++) {
      case 'd': case 'i':
        u.i = va_arg(ap, ptrdiff_t);
        goto SPECIFIER;
      case 'u': case 'o': case 'x': case 'X':
        u.u = va_arg(ap, ptrdiff_t);
        goto SPECIFIER;
      default:
        return -1;
      }
      break;
    case 'L':
      switch (c = *fmt++) {
      case 'f': case 'F': case 'e': case 'E':
      case 'g': case 'G': case 'a': case 'A':
        u.i = va_arg(ap, long double);
        goto SPECIFIER;
      default:
        return -1;
      }
      break;
    case 'd': case 'i':
      u.i = va_arg(ap, int);
      break;
    case 'u': case 'o': case 'x': case 'X':
      u.u = va_arg(ap, unsigned int);
      break;
    case 'f': case 'F': case 'e': case 'E':
    case 'g': case 'G': case 'a': case 'A':
      u.f = va_arg(ap, double);
      break;
    case 'c':
      u.i = va_arg(ap, int);
      break;
    case 's':
      u.p = va_arg(ap, char *);
      break;
    case 'p':
      u.p = va_arg(ap, void *);
      break;
    case '%':
      break;
    default:
      return -1;
    }

  SPECIFIER:
    bufp = bufb = buf;
    switch (c) {
      int radix, upper;

      /* signed */
    case 'd': case 'i':
      if (precision == -1) {
        precision = 0;
      }
      while (u.i != 0) {
        *bufp++ = u.i % 10 + '0';
        u.i /= 10;
      }
      while (bufp - bufb == 0 && precision > 0) {
        *bufp++ = '0';
      }
      break;

      /* unsigned */
    case 'o':
      radix = 8;
      goto UNSIGNED;
    case 'u':
      radix = 10;
      goto UNSIGNED;
    case 'x':
      radix = 16;
      upper = 0;
      goto UNSIGNED;
    case 'X':
      radix = 16;
      upper = 1;
      goto UNSIGNED;

    UNSIGNED:
      if (precision == -1) {
        precision = 0;
      }
      while (u.u != 0) {
        *bufp++ = u.u % 10 + '0';
        if (*bufp > '9') {
          *bufp = *bufp - '9' - 1 + (upper ? 'A' : 'a');
        }
        u.u /= 10;
      }
      while (bufp - bufb == 0 && precision > 0) {
        *bufp++ = '0';
      }
      break;

    case 'a': case 'A':
    case 'e': case 'E':
    case 'f': case 'F':
    case 'g': case 'G':
    case 'c':
      *bufp++ = u.i;
      break;
    case 's':
      bufb = u.p;
      bufp = bufb + strlen(bufb);
      break;
    case 'p':
    case '%':
      *bufp++ = '%';
      break;
    }

  SIGN:
    if ((flags & SIGN) != 0) {
      *bufp++ = (u.i >= 0) ? '+' : '-';
    } else if ((flags & SPACE) != 0 && u.i < 0) {
      *bufp++ = ' ';
    }

  PADDING:
    if ((flags & LEFT) != 0) {
      for (i = 0; bufp - 1 - i >= bufb; ++i) {
        myputc(*(bufp - i - 1));
      }
      while (i++ < width) {
        myputc(' ');
      }
    } else if ((flags & ZERO) != 0) {
      if (bufp - bufb < width) {
        for (i = 0; bufp + i < bufb + width; ++i) {
          myputc('0');
        }
      }
      for (i = 0; bufp - 1 - i >= bufb; ++i) {
        myputc(*(bufp - i - 1));
      }
    } else {
      if (bufp - bufb < width) {
        for (i = 0; bufp + i < bufb + width; ++i) {
          myputc(' ');
        }
      }
      for (i = 0; bufp - 1 - i >= bufb; ++i) {
        myputc(*(bufp - i - 1));
      }
    }
  }

  return 0;                     /* FIXME */
}

int
myprintf(const char *fmt, ...)
{
  va_list ap;
  int r;

  va_start(ap, fmt);

  r = myvprintf(fmt, ap);

  va_end(ap);

  return r;
}

int
main()
{
  myprintf("abcde% 4d%02% %s", 1234324, "hello");
}
