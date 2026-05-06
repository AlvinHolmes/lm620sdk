#include <os.h>
#include "src/log.h"
#include "src/printf.h"
#include "src/str.h"
#include "src/util.h"

#if MG_ENABLE_CUSTOM_LOG

int mg_log_level = MG_LL_INFO;

void mg_log_set_fn(mg_pfn_t fn, void *param) {
}

static void logc(unsigned char c) {
  osPuts((char[2]){c, 0});
}

static void logs(const char *buf, size_t len) {
  (void) len;
  osPuts(buf);
}

void mg_log_prefix(int level, const char *file, int line, const char *fname) {
  const char *p = strrchr(file, '/');
  char buf[42];
  size_t n;
  if (p == NULL) p = strrchr(file, '\\');
  n = mg_snprintf(buf, sizeof(buf), "%-6llx %d %s:%d:%s", mg_millis(), level,
                  p == NULL ? file : p + 1, line, fname);
  if (n > sizeof(buf) - 2) n = sizeof(buf) - 2;
  while (n < sizeof(buf)) buf[n++] = ' ';
  buf[n-1] = '\0';
  logs(buf, n-1);
}

struct LogBuf {
    char    *buf;
    uint32_t len;
    uint32_t off;
};

static void customLogOut(char c, void *param) {
  struct LogBuf *logBuf = (struct LogBuf *)param;
  if (logBuf->off < logBuf->len) {
    logBuf->buf[logBuf->off++] = c;
  }
}

void mg_log(const char *fmt, ...) {
    char            buf[256];
    struct LogBuf   logBuf;

    logBuf.buf = buf;
    logBuf.len = sizeof(buf)-3;
    logBuf.off = 0;

    va_list ap;
    va_start(ap, fmt);
    mg_vxprintf(customLogOut, &logBuf, fmt, &ap);
    va_end(ap);
    buf[logBuf.off++] = '\r';
    buf[logBuf.off++] = '\n';
    buf[logBuf.off]   = '\0';
    logs(buf, logBuf.off);
}

static unsigned char nibble(unsigned c) {
  return (unsigned char) (c < 10 ? c + '0' : c + 'W');
}

#define ISPRINT(x) ((x) >= ' ' && (x) <= '~')
void mg_hexdump(const void *buf, size_t len) {
  const unsigned char *p = (const unsigned char *) buf;
  unsigned char ascii[17], alen = 0;
  size_t i;

  ascii[16] = 0;
  for (i = 0; i < len; i++) {
    if ((i % 16) == 0) {
      // Print buffered ascii chars
      if (i > 0) logs("  ", 2), logs((char *) ascii, 16), logc('\n'), alen = 0;
      // Print hex address, then \t
      logc(nibble((i >> 12) & 15)), logc(nibble((i >> 8) & 15)),
          logc(nibble((i >> 4) & 15)), logc('0'), logs("   ", 3);
    }
    logc(nibble(p[i] >> 4)), logc(nibble(p[i] & 15));  // Two nibbles, e.g. c5
    logc(' ');                                         // Space after hex number
    ascii[alen++] = ISPRINT(p[i]) ? p[i] : '.';        // Add to the ascii buf
  }
  while (alen < 16) logs("   ", 3), ascii[alen++] = ' ';
  logs("  ", 2), logs((char *) ascii, 16), logc('\n');
}

#endif

