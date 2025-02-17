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

/** \ingroup TBD
 *  \defgroup TBD
 *  \brief TBD
 *
 *  @{
 */

#ifndef _TUSB_CDC_CALLBACK_H_
#define _TUSB_CDC_CALLBACK_H_

#include "common/common.h"
#include "cdc_host.h"

#ifdef __cplusplus
 extern "C" {
#endif

void tusbh_cdc_mounted_cb(uint8_t dev_addr);
void tusbh_cdc_unmounted_cb(uint8_t dev_addr);
void tusbh_cdc_xfer_isr(uint8_t dev_addr, xfer_result_t event, cdc_pipeid_t pipe_id, uint32_t xferred_bytes);

void tusbh_cdc_rndis_mounted_cb(uint8_t dev_addr);
void tusbh_cdc_rndis_unmounted_isr(uint8_t dev_addr);


#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CDC_CALLBACK_H_ */

/** @} */
