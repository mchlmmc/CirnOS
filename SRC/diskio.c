// The following code was edited from
// InterimOS, created by Lukas F. Hartmann.
// That project is licensed under GPL,
// found at <https://github.com/mntmn/interim>

#include <stdlib.h>

#include "ff.h"
#include "diskio.h"
#include "emmc.h"

uint8_t disk_current_status = STA_NOINIT;

/**
 * disk_initialize - FatFs wrapper for sd init
 *
 * @drv: The drive to init.
 *
 * Calls sd_card_init if drive is not 0.
 */
DSTATUS disk_initialize (uint8_t drv) {
  if(drv) return RES_ERROR;    
  if (sd_card_init() == 0) {
    disk_current_status &= ~STA_NOINIT;
  }

  return disk_current_status;
}

/**
 * disk_read - Read sectors of SD Card.
 *
 * @drv: The drive to read from
 * @buf: Memory to read to.
 * @sector: Place on card to read.
 * @count: Amount of sectors to read.
 *
 * Starting from sector, reads count
 * sectors into buf.
 */
DRESULT disk_read (uint8_t drv, uint8_t *buf, uint32_t sector, uint32_t count) {
  if(drv) return RES_ERROR;  
  if (sd_read(buf, sector, count) > 0)
    return RES_OK;
  else
    return RES_ERROR;
}

/**
 * disk_write - Write sectors to SD Card
 *
 * @drv: The drive to write to
 * @buf: Memory to copy to card
 * @sector: Place on card to write to
 * @count: Amount of sectors to write
 *
 * Starting from sector, writes buf
 * to card.
 */
DRESULT disk_write (uint8_t drv, uint8_t *buf, uint32_t sector, uint32_t count) {
  if(drv) return RES_ERROR;
  if (sd_write(buf, sector, count) > 0)
    return RES_OK;
  else
    return RES_ERROR;
}

/**
 * disk_status - Returns status of drive
 *
 * @drv: The drive to check
 *
 * Returns the current status of SD Card.
 */
DSTATUS disk_status (uint8_t drv) {
  if(drv) return RES_ERROR;  
  return disk_current_status;
}

/**
 * disk_ioctl - Only used by FatFs
 * 
 * Always returns true if the drive
 * can be accessed.
 */
DRESULT disk_ioctl (uint8_t drv, uint8_t ctrl, void *buf) {
  if(drv) return RES_ERROR;  
  return RES_OK;
}

/**
 * get-fattime - Returns current time in FAT format.
 *
 * Returns the date 1999-9-9 in FAT format because
 * CirnOS.
 */
uint32_t get_fattime (void)
{
  return  ((uint32_t)(1999 - 1980) << 25)/* Year = 1999 */
    | ((uint32_t)9 << 21)/* Month = 9 */
    | ((uint32_t)9 << 16)/* Day_m = 9*/
    | ((uint32_t)0 << 11)/* Hour = 0 */
    | ((uint32_t)0 << 5)/* Min = 0 */
    | ((uint32_t)0 >> 1);/* Sec = 0 */
}
