// CirnOS -- Minimalistic scripting environment for the Raspberry Pi
// Copyright (C) 2018 Michael Mamic
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>

#include "ff.h"
#include "hdmi.h"

#undef errno
extern int errno;

#define MAX_OPEN_FILES 20
FIL *openfiles[MAX_OPEN_FILES];

/**
 * _write - Writes bytes to a file
 * 
 * @file: File handle to write to
 * @ptr: Pointer to bytes to write
 * @len: Length of bytes to write
 * 
 * Prints to screen if the file handle
 * is STDOUT or STDERR. Writes to file
 * open at handle if the file is using
 * FatFs.
 */
int _write(int file, char *ptr, int len)
{
    static UINT blocksval;
    UINT *blockswrote = &blocksval;
    if (file >= 0 && file < 3) {
        for (uint32_t i = 0; i < len; i++) {
            hdmi_write_char(*ptr++);
        }
        return len;
    } else if (openfiles[file - 3] && f_write(openfiles[file - 3], ptr, len, blockswrote) == FR_OK) {
        return blocksval;
    }
    errno = EBADF;
    return -1;
}

/**
 * _translateflags - Converts flags
 * 
 * @flags: Flags to convert
 * 
 * Newlib and FatFs use different schemes
 * for flags. This function converts
 * a flag value requested from newlib
 * so FatFs can read it.
 */
int _translateflags(int flags)
{
    // Read
    switch (flags) {
        // a+
        // ab+
    case 522:
    case 66058:
        flags = FA_READ | FA_WRITE | FA_OPEN_APPEND;
        break;
        // r+
        // rb+
        // w+
        // wb+    
    case 2:
    case 65538:
    case 1538:
    case 67074:
        flags = FA_READ | FA_WRITE;
        break;
        // a
        // ab
    case 512:
    case 66057:
        flags = FA_WRITE | FA_CREATE_NEW | FA_OPEN_APPEND;
        break;
        // w
        // wb
    case 1537:
    case 67073:
        flags = FA_WRITE | FA_CREATE_NEW | FA_OPEN_ALWAYS;
        break;
        // r
        // rb
    case 65536:
    case 0:
        flags = FA_READ;
        break;
    default:
        break;
    }
    return flags;
}

/**
 * _open - Opens a file.
 * 
 * @name: Path of file to open.
 * @flags: Mode to parse file.
 * @perms: Permissions of file (unused).
 * 
 * Looks for a file handle available.
 * If there are already twenty files open
 * then it returns an error.
 * After, it tries to open the file at name.
 * If it exists then it returns the file
 * handle that it is assigned to.
 */
int _open(const char *name, int flags, int perms)
{
    flags = _translateflags(flags);
    uint8_t fd;
    for (fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (openfiles[fd] == NULL)
            break;
    }
    if (fd == MAX_OPEN_FILES) {
        errno = EMFILE;
        return -1;
    }

    FIL *fp = malloc(sizeof(FIL));
    if (!fp) {
        return -1;
    }
    if (f_open(fp, name, flags) == FR_OK) {
        openfiles[fd] = fp;
        return fd + 3;
    }
    return -1;
}

/**
 * _close - Closes a file.
 * 
 * @file: Handle of file to close.
 * 
 * Closes the file at handle. If
 * the file is not open or is 
 * STDIN, STDOUT or STDERROR,
 * then it returns an error.
 */
int _close(int file)
{
    if (file >= 3 && openfiles[file - 3]) {
        if (f_close(openfiles[file - 3]) == FR_OK) {
            openfiles[file - 3] = NULL;
            return 0;
        }
    }
    errno = EBADF;
    return -1;
}

/**
 * _lseek - Changes position in a file.
 * 
 * @file: Handle of file to change
 * @offset: Relative position in file
 * @origin: Where offset is measured
 * from in the file.
 * 
 * Does not work for STDIN, STDOUT, or
 * STDERR.
 * Changes position in file used by
 * getc, putc etc...
 */
int _read(int file, char *ptr, int len)
{

    if (file >= 3 && openfiles[file - 3]) {
        UINT blockcount = 0;
        UINT *blocksread = &blockcount;
        if (f_read(openfiles[file - 3], ptr, len, blocksread) == FR_OK) {
            return blockcount;
        }
    }
    errno = EBADF;
    return -1;
}

/**
 * _lseek - Changes position in a file.
 * 
 * @file: Handle of file to change
 * @offset: Relative position in file
 * @origin: Where offset is measured
 * from in the file.
 * 
 * Does not work for STDIN, STDOUT, or
 * STDERR.
 * Changes position in file used by
 * getc, putc etc...
 */
int _lseek(int file, int offset, int dir)
{
    if (file < 3 || !openfiles[file - 3]) {
        errno = EBADF;
        return -1;
    }

    FSIZE_t fpointer = openfiles[file - 3]->fptr;
    if (dir == 0)
        fpointer = offset;
    else if (dir == 1)
        fpointer = fpointer + offset;
    // End of file not supported in FatFs library  
    else {
        errno = EINVAL;
        return -1;
    }
    while (fpointer > FF_MAX_SS * SDFS.csize) {
        fpointer -= FF_MAX_SS * SDFS.csize;
        openfiles[file - 1]->clust++;
    }
    if (f_lseek(openfiles[file - 3], fpointer) == FR_OK) {
        return fpointer;
    } else {
        errno = EBADF;
        return -1;
    }
}

/**
 * _fstat - Gets information on a file
 * 
 * @file: Handle of file to inspect.
 * @st: Where the information will be
 * returned to.
 * 
 * Most fields in st are not used by
 * CirnOS, so these are left as zeros.
 * Information is only provided for
 * STDERR and STDOUT.
 */
int _fstat(int file, struct stat *st)
{
    if (!file) {
        errno = ENOENT;
        return -1;
    } else if (file < 3) {
        st->st_dev = 0;
        st->st_ino = file;
        st->st_mode = S_IFCHR;
        st->st_nlink = 0;
        st->st_uid = 0;
        st->st_gid = 0;
        st->st_rdev = 0;
        st->st_size = SCREEN_WIDTH * SCREEN_HEIGHT * (BIT_DEPTH / 2);
        st->st_blksize = 512;
        st->st_blocks = ((SCREEN_WIDTH * SCREEN_HEIGHT) * (BIT_DEPTH * 2)) / 512;
        st->st_atime = 0;
        st->st_mtime = 0;
        st->st_ctime = 0;
        return 0;
    }
    errno = ENOENT;
    return -1;
}

/**
 * _link - Deletes a file or directory
 * 
 * @name: Name of file to delete.
 */
int _unlink(char *name)
{
    if (f_unlink(name) == FR_OK) {
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

/**
 * _link - Renames a file
 * 
 * @name_old: Name of existing file.
 * @name_new: New name for file.
 *
 * Uses FatFs library to rename file
 * at path name_old to name_new.
 */
int _link(char *name_old, char *name_new)
{
    if (f_rename(name_old, name_new) == FR_OK) {
        return 0;
    } else {
        errno = ENOENT;
        return -1;
    }
}

/**
 * _sbrk - Requests memory for malloc
 *
 * @incr: Amount of memory to allocate.
 *
 * Gives chunk of memory starting at
 * end of program. If the memory
 * request exceeds the total rPi
 * memory then throw an error.
 */
char *_sbrk(size_t incr)
{
    extern char RAM_AMOUNT;
    static char *max = &RAM_AMOUNT;
    extern char _end;
    static char *highest_addr = &_end;
    char *prev_highest_addr;

    prev_highest_addr = highest_addr;
    if (highest_addr + incr > max) {
        errno = ENOMEM;
        return (char *)-1;
    }

    highest_addr += incr;
    return prev_highest_addr;
}

/**
 * _gettimeofday - Gets time of day.
 *
 * @tv: Pointer to timeval for seconds
 * to be returned in.
 * @tz: Pointer to timezone for timezone
 * data to be returned in.
 * 
 * Returns all zeros since the rPi
 * has no built in RTC.
 */
int _gettimeofday(struct timeval *tv, struct timezone *tz)
{
    tv->tv_sec = 0;
    tv->tv_usec = 0;
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
    return 0;
}

/**
 * _isatty - Check if file handle is a tty screen.
 *
 * @file: File handle to check
 * 
 * Always return true to enable maximum
 * compatibility.
 */
int _isatty(int file)
{
    return 1;
}

// Unused syscalls (CirnOS provides no facilities for threading)
char *__env[1] = { 0 };

char **environ = __env;

int _fini(int file)
{
    return 1;
}

int _times(struct tms *buf)
{
    return -1;
}

int _exit(int status)
{
    return -1;
}

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _wait(int *status)
{
    errno = ECHILD;
    return -1;
}
