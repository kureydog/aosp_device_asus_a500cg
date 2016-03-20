/* Copyright (c) 2010 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef VBOOT_REFERENCE_CGPTLIB_H_
#define VBOOT_REFERENCE_CGPTLIB_H_

#include "sysincludes.h"

enum {
  GPT_SUCCESS = 0,
  GPT_ERROR_NO_VALID_KERNEL,
  GPT_ERROR_INVALID_HEADERS,
  GPT_ERROR_INVALID_ENTRIES,
  GPT_ERROR_INVALID_SECTOR_SIZE,
  GPT_ERROR_INVALID_SECTOR_NUMBER,
  GPT_ERROR_INVALID_UPDATE_TYPE,
  GPT_ERROR_CRC_CORRUPTED,
  GPT_ERROR_OUT_OF_REGION,
  GPT_ERROR_START_LBA_OVERLAP,
  GPT_ERROR_END_LBA_OVERLAP,
  GPT_ERROR_DUP_GUID,
};

/* Bit masks for GptData.modified field. */
#define GPT_MODIFIED_HEADER1 0x01
#define GPT_MODIFIED_HEADER2 0x02
#define GPT_MODIFIED_ENTRIES1 0x04
#define GPT_MODIFIED_ENTRIES2 0x08

#define TOTAL_ENTRIES_SIZE 16384  /* Size of GptData.primary_entries
                                   * and secondary_entries: 128
                                   * bytes/entry * 128 entries. */

/* The 'update_type' of GptUpdateKernelEntry()
 * We expose TRY and BAD only because those are what verified boot needs.
 * For more precise control on GPT attribute bits, please refer to
 * gpt_internal.h */
enum {
  GPT_UPDATE_ENTRY_TRY = 1,
  /* System will be trying to boot the currently selected kernel partition.
   * Update its try count if necessary. */
  GPT_UPDATE_ENTRY_BAD = 2,
  /* The currently selected kernel partition failed validation.  Mark entry as
   * invalid. */
};

typedef struct {
  /* Fill in the following fields before calling GptInit() */
  uint8_t *primary_header;       /* GPT primary header, from sector 1 of disk
                                  *   (size: 512 bytes) */
  uint8_t *secondary_header;     /* GPT secondary header, from last sector of
                                  *   disk (size: 512 bytes) */
  uint8_t *primary_entries;      /* primary GPT table, follows primary header
                                  *   (size: 16 KB) */
  uint8_t *secondary_entries;    /* secondary GPT table, precedes secondary
                                  *   header (size: 16 KB) */
  uint32_t sector_bytes;         /* Size of a LBA sector, in bytes */
  uint64_t drive_sectors;        /* Size of drive in LBA sectors, in sectors */

  /* Outputs */
  uint8_t modified;              /* Which inputs have been modified?
                                  * 0x01 = header1
                                  * 0x02 = header2
                                  * 0x04 = table1
                                  * 0x08 = table2  */
  int current_kernel; /* the current chromeos kernel index in partition table.
                       * -1 means not found on drive. Note that GPT partition
                       * numbers are traditionally 1-based, but we're using
                       * a zero-based index here.
                       */

  /* Internal variables */
  uint32_t valid_headers, valid_entries;
  int current_priority;
} GptData;

int GptInit(GptData* gpt);
/* Initializes the GPT data structure's internal state.  The following fields
 * must be filled before calling this function:
 *
 *   primary_header
 *   secondary_header
 *   primary_entries
 *   secondary_entries
 *   sector_bytes
 *   drive_sectors
 *
 * On return the modified field may be set, if the GPT data has been modified
 * and should be written to disk.
 *
 * Returns GPT_SUCCESS if successful, non-zero if error:
 *   GPT_ERROR_INVALID_HEADERS, both partition table headers are invalid, enters
 *                              recovery mode,
 *   GPT_ERROR_INVALID_ENTRIES, both partition table entries are invalid, enters
 *                              recovery mode,
 *   GPT_ERROR_INVALID_SECTOR_SIZE, size of a sector is not supported,
 *   GPT_ERROR_INVALID_SECTOR_NUMBER, number of sectors in drive is invalid (too
 *                                    small) */

int GptNextKernelEntry(GptData* gpt, uint64_t* start_sector, uint64_t* size);
/* Provides the location of the next kernel partition, in order of decreasing
 * priority.  On return the start_sector parameter contains the LBA sector
 * for the start of the kernel partition, and the size parameter contains the
 * size of the kernel partition in LBA sectors.  gpt.current_kernel contains
 * the partition index of the current chromeos kernel partition.
 *
 * Returns GPT_SUCCESS if successful, else
 *   GPT_ERROR_NO_VALID_KERNEL, no avaliable kernel, enters recovery mode */

int GptUpdateKernelEntry(GptData* gpt, uint32_t update_type);
/* Updates the kernel entry with the specified index, using the specified type
 * of update (GPT_UPDATE_ENTRY_*).
 *
 * On return the modified field may be set, if the GPT data has been modified
 * and should be written to disk.
 *
 * Returns GPT_SUCCESS if successful, else
 *   GPT_ERROR_INVALID_UPDATE_TYPE, invalid 'update_type' is given.
 */

#endif  /* VBOOT_REFERENCE_CGPTLIB_H_ */
