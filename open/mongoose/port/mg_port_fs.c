#include <vfs.h>
#include "src/fs.h"
#include "src/printf.h"
#include "src/str.h"

#if MG_ENABLE_FILE

int mg_fgetc(FILE *fp)
{
    char c;
    if (VFS_ReadFile(&c, 1, 1, fp) == 1)
        return c;
    return -1;
}

int mg_stat(const char *path, size_t *size, time_t *mtime)
{
    VFS_FileStat st;

    if (VFS_Stat(path, &st) != VFS_EOK) {
       return 0;
    }

    if (size)
        *size = (size_t) st.st_size;
    if (mtime)
        *mtime = 0;

    return MG_FS_READ | MG_FS_WRITE | (VFS_S_ISDIR(st.st_mode) ? MG_FS_DIR : 0);
}

#endif

#if MG_ENABLE_PACKED_FS

OS_WEAK const char *mg_unpack(const char *path, size_t *size, time_t *mtime)
{
  *size = 0, *mtime = 0;
  (void) path;
  return NULL;
}

OS_WEAK const char *mg_unlist(size_t no)
{
  (void) no;
  return NULL;
}

#endif

