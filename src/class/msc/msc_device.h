/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#ifndef _TUSB_MSC_DEVICE_H_
#define _TUSB_MSC_DEVICE_H_

#include "common/tusb_common.h"
#include "device/usbd.h"
#include "msc.h"

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// Class Driver Configuration
//--------------------------------------------------------------------+
TU_VERIFY_STATIC(CFG_TUD_MSC_BUFSIZE < UINT16_MAX, "Size is not correct");

#ifndef CFG_TUD_MSC_BUFSIZE
  #error CFG_TUD_MSC_BUFSIZE must be defined, value of a block size should work well, the more the better
#endif

/** \addtogroup ClassDriver_MSC
 *  @{
 * \defgroup MSC_Device Device
 *  @{ */

bool tud_msc_set_sense(uint8_t lun, uint8_t sense_key, uint8_t add_sense_code, uint8_t add_sense_qualifier);

//--------------------------------------------------------------------+
// Application Callbacks (WEAK is optional)
//--------------------------------------------------------------------+

/**
 * Invoked when received \ref SCSI_CMD_READ_10 command
 * \param[in]   lun         Logical unit number
 * \param[in]   lba         Logical Block Address to be read
 * \param[in]   offset      Byte offset from LBA
 * \param[out]  buffer      Buffer which application need to update with the response data.
 * \param[in]   bufsize     Requested bytes
 *
 * \return      Number of byte read, if it is less than requested bytes by \a \b bufsize. Tinyusb will transfer
 *              this amount first and invoked this again for remaining data.
 *
 * \retval      zero        Indicate application is not ready yet to response e.g disk I/O is not complete.
 *                          tinyusb will invoke this callback with the same parameters again some time later.
 *
 * \retval      negative    Indicate error e.g reading disk I/O. tinyusb will \b STALL the corresponding
 *                          endpoint and return failed status in command status wrapper phase.
 */
int32_t tud_msc_read10_cb (uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);

/**
 * Invoked when received \ref SCSI_CMD_WRITE_10 command
 * \param[in]   lun         Logical unit number
 * \param[in]   lba         Logical Block Address to be write
 * \param[in]   offset      Byte offset from LBA
 * \param[out]  buffer      Buffer which holds written data.
 * \param[in]   bufsize     Requested bytes
 *
 * \return      Number of byte written, if it is less than requested bytes by \a \b bufsize. Tinyusb will proceed with
 *              other work and invoked this again with adjusted parameters.
 *
 * \retval      zero        Indicate application is not ready yet e.g disk I/O is not complete.
 *                          Tinyusb will invoke this callback with the same parameters again some time later.
 *
 * \retval      negative    Indicate error writing disk I/O. Tinyusb will \b STALL the corresponding
 *                          endpoint and return failed status in command status wrapper phase.
 */
int32_t tud_msc_write10_cb (uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]);

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun);

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size);

/**
 * Invoked when received an SCSI command not in built-in list below.
 * - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, TEST_UNIT_READY, START_STOP_UNIT, MODE_SENSE6, REQUEST_SENSE
 * - READ10 and WRITE10 has their own callbacks
 *
 * \param[in]   lun         Logical unit number
 * \param[in]   scsi_cmd    SCSI command contents which application must examine to response accordingly
 * \param[out]  buffer      Buffer for SCSI Data Stage.
 *                            - For INPUT: application must fill this with response.
 *                            - For OUTPUT it holds the Data from host
 * \param[in]   bufsize     Buffer's length.
 *
 * \return      Actual bytes processed, can be zero for no-data command.
 * \retval      negative    Indicate error e.g unsupported command, tinyusb will \b STALL the corresponding
 *                          endpoint and return failed status in command status wrapper phase.
 */
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize);

/*------------- Optional callbacks -------------*/

// Invoked when received GET_MAX_LUN request, required for multiple LUNs implementation
ATTR_WEAK uint8_t tud_msc_get_maxlun_cb(void);

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
ATTR_WEAK void tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject);

// Invoked when Read10 command is complete
ATTR_WEAK void tud_msc_read10_complete_cb(uint8_t lun);

// Invoke when Write10 command is complete, can be used to flush flash caching
ATTR_WEAK void tud_msc_write10_complete_cb(uint8_t lun);

// Invoked when command in tud_msc_scsi_cb is complete
ATTR_WEAK void tud_msc_scsi_complete_cb(uint8_t lun, uint8_t const scsi_cmd[16]);

// Hook to make a mass storage device read-only. TODO remove
ATTR_WEAK bool tud_msc_is_writable_cb(uint8_t lun);

/** @} */
/** @} */

//--------------------------------------------------------------------+
// Internal Class Driver API
//--------------------------------------------------------------------+
void mscd_init(void);
bool mscd_open(uint8_t rhport, tusb_desc_interface_t const * itf_desc, uint16_t *p_length);
bool mscd_control_request(uint8_t rhport, tusb_control_request_t const * p_request);
bool mscd_control_request_complete (uint8_t rhport, tusb_control_request_t const * p_request);
bool mscd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes);
void mscd_reset(uint8_t rhport);

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_MSC_DEVICE_H_ */
