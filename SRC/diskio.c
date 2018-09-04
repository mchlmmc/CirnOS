// The following code was edited from
// InterimOS, created by Lukas F. Hartmann.
// That project is licensed under GPL,
// found at <https://github.com/mntmn/interim>

/*
 * Based on code from NXP app note AN10916.
 */
#include <stdint.h>
#include <stdlib.h>
#include "ff.h"
#include "diskio.h"
#include "malloc.h"
#include "emmc.h"

BYTE disk_current_status = STA_NOINIT;

void* sd_aligned_malloc(size_t required_bytes, size_t alignment) {
  void* p1; // original block
  void** p2; // aligned block
  int offset = alignment - 1 + sizeof(void*);
  if ((p1 = (void*)malloc(required_bytes + offset)) == NULL) {
    return NULL;
  }
  p2 = (void**)(((size_t)(p1) + offset) & ~(alignment - 1));
  p2[-1] = p1;
  return p2;
}

void sd_aligned_free(void *p) {
  free(((void**)p)[-1]);
}

/* disk_initialize
 *
 * Set up the disk.
 */
DSTATUS disk_initialize (BYTE drv) {
  if(drv) return RES_ERROR;    
  if (sd_card_init() == 0) {
    disk_current_status &= ~STA_NOINIT;
  }

  return disk_current_status;
}

/* disk_read
 *
 * Read some sectors.
 */
DRESULT disk_read (BYTE drv, BYTE *buf, DWORD sector, UINT count) {
  if(drv) return RES_ERROR;  
  if (sd_read(buf, sector, count) > 0)
    return RES_OK;
  else
    return RES_ERROR;
}

/* disk_write
 *
 * Write some sectors.
 */
DRESULT disk_write (BYTE drv, BYTE *buf, DWORD sector, UINT count) {
  if(drv) return RES_ERROR;
  if (sd_write(buf, sector, count) > 0)
    return RES_OK;
  else
    return RES_ERROR;
}

/* disk_status
 *
 * Check the status of this drive. All we know how to say is "initialized"
 * vs "uninitialized".
 */
DSTATUS disk_status (BYTE drv) {
  if(drv) return RES_ERROR;  
  return disk_current_status;
}

/* disk_ioctl
 *
 * Everything else.
 */
DRESULT disk_ioctl (BYTE drv, BYTE ctrl, void *buf) {
  if(drv) return RES_ERROR;  
  return RES_OK;
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
DWORD get_fattime (void)
{
  return  ((DWORD)(1999 - 1980) << 25)/* Year = 1999 */
    | ((DWORD)9 << 21)/* Month = 9 */
    | ((DWORD)9 << 16)/* Day_m = 9*/
    | ((DWORD)0 << 11)/* Hour = 0 */
    | ((DWORD)0 << 5)/* Min = 0 */
    | ((DWORD)0 >> 1);/* Sec = 0 */
}
