#include <os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "nr_micro_shell.h"

static const char test_data_pattern1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
static const char test_data_pattern2[] = {0x01, 0x02, 0x03, 0xA3, 0xBE, 0x06, 0x07, 0x08};

static void demo_fs_all(char argc, char **argv)
{
    int retcode;
    char buf[16];
    int size = 0;
    int fd;

    const char *test_file = "/case1.ts";

    size = sizeof(test_data_pattern1);

    // 1.read-write test

    fd = open(test_file, O_WRONLY | O_CREAT | O_TRUNC);
    osPrintf("[FS]: open fd = %d\r\n", fd);

    retcode = write(fd, test_data_pattern1, size);
    osPrintf("[FS]: write = %d\r\n", retcode);

    retcode = close(fd);
    osPrintf("[FS]: close = %d\r\n", retcode);

    fd = open(test_file, O_RDONLY);
    osPrintf("[FS]: open fd = %d\r\n", fd);

    retcode = read(fd, buf, size);
    osPrintf("[FS]: read = %d\r\n", retcode);

    retcode = memcmp(test_data_pattern1, buf, size);
    if (retcode == 0) {
        osPrintf("[FS]: read-write test OK.\r\n");
    } else {
        osPrintf("[FS]: read-write test failed!\r\n");
    }

    retcode = close(fd);
    osPrintf("[FS]: close = %d\r\n", retcode);

    // 2.EOF test

    fd = open(test_file, O_RDWR);
    osPrintf("[FS]: open fd = %d\r\n", fd);

    retcode = read(fd, buf, size + 300);
    osPrintf("[FS]: read = %d\r\n", retcode);

    retcode = memcmp(test_data_pattern1, buf, size);
    if (retcode == 0) {
        osPrintf("[FS]: read-write test EOF OK.\r\n");
    } else {
        osPrintf("[FS]: read-write test EOF failed!\r\n");
    }

    retcode = lseek(fd, 3, SEEK_SET);
    osPrintf("[FS]: lseek = %d\r\n", retcode);
    buf[0] = 0xA3;
    buf[1] = 0xBE;

    retcode = write(fd, buf, 2);
    osPrintf("[FS]: write = %d\r\n", retcode);

    retcode = lseek(fd, 0, SEEK_SET);
    osPrintf("[FS]: lseek = %d\r\n", retcode);

    retcode = read(fd, buf, 8);
    osPrintf("[FS]: read = %d\r\n", retcode);

    retcode = memcmp(test_data_pattern2, buf, 8);
    if (retcode == 0) {
        osPrintf("[FS]: read-write test EOF OK.\r\n");
    } else {
        osPrintf("[FS]: read-write test EOF failed!\r\n");
    }

    retcode = close(fd);
    osPrintf("[FS]: close = %d\r\n", retcode);

    memset(buf, 0, sizeof(buf));
    fd = open(test_file, O_RDWR | O_TRUNC);
    buf[0] = 'H';
    buf[1] = 'e';
    buf[2] = 'l';
    buf[3] = 'l';
    buf[4] = 'o';
    retcode = write(fd, buf, 5);
    retcode = lseek(fd, 0, SEEK_SET);
    retcode = read(fd, buf, 100);
    osPrintf("[FS]: test O_TRUNC, read = %d, %s\r\n", retcode, buf);
    retcode = close(fd);

    memset(buf, 0, sizeof(buf));
    fd = open(test_file, O_RDWR | O_APPEND);
    buf[0] = 'W';
    buf[1] = 'o';
    buf[2] = 'r';
    buf[3] = 'l';
    buf[4] = 'd';
    retcode = write(fd, buf, 5);
    retcode = lseek(fd, 0, SEEK_SET);
    retcode = read(fd, buf, 100);
    osPrintf("[FS]: test O_APPEND, read = %d, %s\r\n", retcode, buf);
    retcode = close(fd);

    char test_file_new_name[] = "/case1.new";
    retcode = rename(test_file, test_file_new_name);
    osPrintf("[FS]: %s rename to %s = %d\r\n", test_file, test_file_new_name, retcode);

    osPrintf("[FS]: %s %sexist\r\n", test_file, access(test_file, F_OK) != 0 ? "not " : "");

    osPrintf("[FS]: %s %sexist\r\n", test_file_new_name, access(test_file_new_name, F_OK) == 0 ? "" : "not ");

    osPrintf("[FS]: %s %s be RW\r\n", test_file_new_name, access(test_file_new_name, R_OK | W_OK) == 0 ? "can" : "can not");

    osPrintf("[FS]: %s %s be executed\r\n", test_file_new_name, access(test_file_new_name, X_OK) == 0 ? "can" : "can not");

    retcode = unlink(test_file);
    osPrintf("[FS]: %s unlink = %d\r\n", test_file, retcode);

    retcode = unlink(test_file_new_name);
    osPrintf("[FS]: %s unlink = %d\r\n", test_file_new_name, retcode);

    retcode = open(test_file_new_name, O_RDONLY);
    osPrintf("[FS]: open %s after unlink fd = %d\r\n", test_file_new_name, retcode);
    return;
}

NR_SHELL_CMD_EXPORT(posix_fs, demo_fs_all);

#if defined(_CPU_AP)
#include "psm_wakelock.h"
WakeLock lock = {0};
int WAKELOCK_LOCK(void)
{
    PSM_WakelockInit(&lock, PSM_DEEP_SLEEP);
    PSM_WakeLock(&lock);
    return 0;
}
INIT_APP_EXPORT(WAKELOCK_LOCK, OS_INIT_SUBLEVEL_LOW);
#endif
