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

#include <ff.h>
#include <bcm2835.h>
#include <hdmi.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#undef errno
extern int errno;

int _write(int file, char *ptr, int len) {
  // No support for STDIN
  if(file == 0) {
    errno = EBADF;
    return -1;
  } else if (file < 3) {  // STDOUT and STDERR
    for (uint32_t i  = 0; i < len; i++) {
      hdmi_write_char(*ptr++);
    }
    return len;
  }
  // Write FAT32 file at handle
  if(openfiles[file-3] == NULL) {
    errno = EBADF;
    return -1;
  }
  UINT *blockswrote = NULL;
  if (f_write(openfiles[file-3], ptr, len, blockswrote) == FR_OK) {
    return *blockswrote;
  } else {
    errno = EBADF;
    return -1;
  }  
  return 1;    
}

int _translateflags(int flags) {
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

// File controls
int _open(const char *name, int flags, int perms) {
  // Find a handle
  flags = _translateflags(flags);
  uint8_t fd;
  for(fd = 0; fd < MAX_OPEN_FILES; fd++) {
    if(openfiles[fd] == NULL)
      break;
  }
  if(openfiles[fd] != NULL) {
    errno = EMFILE;
    return -1;
  }
  FIL *fp = malloc(sizeof(FIL));
  int res = f_open(fp, name, flags);
  if(res == FR_OK) {
    openfiles[fd] = fp;
    return fd+3;
  }
  return -1;
}

int _close(int file) {
  // No way to close STDIN, STDOUT, or STDERROR
  if (file < 3) {
    errno = EBADF;
    return -1;    
  }
  // Close FAT32 file at handle
  if(openfiles[file-3] == NULL) {
    errno = EBADF;
    return -1;
  }
  if (f_close(openfiles[file-3]) == FR_OK) {
    openfiles[file-3] = NULL;
    return 0;
  } else {
    errno = EBADF;
    return -1;
  }
}

int _read(int file, char *ptr, int len) {
  // No way to read STDIN, STDOUT, or STDERROR
  if (file < 3) {
    errno = EBADF;
    return -1;    
  }
  if(openfiles[file-3] == NULL) {
    errno = EBADF;
    return -1;
  }
  // Read FAT32 file at handle
  UINT blockcount = 0;
  UINT *blocksread = &blockcount;
  bcm2835_delay(1000);
  if (f_read(openfiles[file-3], ptr, len, blocksread) == FR_OK) {
    return blockcount;
  }
  errno = EBADF;
  return -1;
}

// Needs some work.
int _lseek(int file, int ptr, int dir) {
  // No way to seek STDIN, STDOUT, or STDERROR
  if (file < 3) {
    errno = EBADF;
    return -1;    
  }
  if(openfiles[file-3] == NULL) {
    errno = EBADF;
    return -1;
  }
  // Seek FAT32 file at handle
  FSIZE_t fpointer = openfiles[file-3]->fptr;
  if(dir == 0) fpointer = ptr;
  else if (dir == 1) fpointer = fpointer + ptr;
  // End of file not supported in FatFs library  
  else {
    errno = EINVAL;
    return -1;
  }  
  while(fpointer > FF_MAX_SS * SDFS.csize) {
    fpointer -= FF_MAX_SS * SDFS.csize;
    openfiles[file-1]->clust++;
  }
  if (f_lseek(openfiles[file-3], fpointer) == FR_OK) {
    return fpointer;
  } else {
    errno = EBADF;
    return -1;
  }
}

int _fstat(int file, struct stat *st) {
  if(!file) {
    errno = ENOENT;
    return -1;
  } else if (file < 3) {
    st->st_dev = 0;       /* ID of device containing file */
    st->st_ino = file;    /* inode number */
    st->st_mode = 0222;   /* protection */
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

int _unlink(char *name) {
  if(f_unlink(name) == FR_OK) {
    return 0;
  } else {
    errno = ENOENT;
    return -1;
  }
}

int _link(char *name_old, char *name_new) {
  if(f_rename(name_old, name_new) == FR_OK) {
    return 0;
  } else {
    errno = ENOENT;
    return -1;
  }
}

//  Start at the end of BSS segment and will increase until it reaches MEMMAX.

uint32_t _sbrk(int incr) {
  highest_addr += incr;
  
  if((uint32_t)highest_addr > 0x20000000) {
    errno = ENOMEM;    
    return -1;    
  }
  
  return (uint32_t)highest_addr;
}

int _gettimeofday(struct timeval *tv, struct timezone *tz) {
  tv->tv_sec = 0;
  tv->tv_usec = 0;
  tz->tz_minuteswest = 0;
  tz->tz_dsttime = 0;
  return 0;
}

int _isatty(int file) {
  return 1;
}

// Unused syscalls (CirnOS provides no facilities for threading)
char *__env[1] = { 0 };
char **environ = __env;

int _fini(int file) {
  return 1;
}

int _times(struct tms *buf) {
  return -1;
}

int _exit(int status) {
  return -1;  
}

int _execve(char *name, char **argv, char **env) {
  errno = ENOMEM;
  return -1;
}

int _fork(void) {
  errno = EAGAIN;
  return -1;
}

int _getpid(void) {
  return 1;
}

int _kill(int pid, int sig) {
  errno = EINVAL;
  return -1;
}

int _wait(int *status) {
  errno = ECHILD;
  return -1;
}
