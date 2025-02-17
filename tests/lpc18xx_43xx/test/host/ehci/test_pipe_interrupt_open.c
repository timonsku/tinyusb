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

#include "unity.h"
#include "tusb_option.h"
#include "tusb_errors.h"
#include "binary.h"
#include "type_helper.h"

#include "hal.h"
#include "mock_osal.h"
#include "hcd.h"
#include "mock_usbh_hcd.h"
#include "ehci.h"
#include "ehci_controller_fake.h"
#include "host_helper.h"

usbh_device_t _usbh_devices[CFG_TUSB_HOST_DEVICE_MAX+1];

static uint8_t const hub_addr = 2;
static uint8_t const hub_port = 2;
static uint8_t dev_addr;
static uint8_t hostid;

static ehci_qhd_t *period_head_arr;

static ehci_qhd_t *p_int_qhd;
static pipe_handle_t pipe_hdl;

//--------------------------------------------------------------------+
// Setup/Teardown + helper declare
//--------------------------------------------------------------------+
void setUp(void)
{
  tu_memclr(_usbh_devices, sizeof(usbh_device_t)*(CFG_TUSB_HOST_DEVICE_MAX+1));

  hcd_init();

  dev_addr = 1;
  hostid = RANDOM(CONTROLLER_HOST_NUMBER) + TEST_CONTROLLER_HOST_START_INDEX;

  helper_usbh_device_emulate(dev_addr , hub_addr, hub_port, hostid, TUSB_SPEED_HIGH);

  period_head_arr = get_period_head( hostid, 1 );
  p_int_qhd = NULL;
  tu_memclr(&pipe_hdl, sizeof(pipe_handle_t));
}

void tearDown(void)
{
}

void verify_open_qhd(ehci_qhd_t *p_qhd, uint8_t endpoint_addr, uint16_t max_packet_size)
{
  TEST_ASSERT_EQUAL(dev_addr, p_qhd->device_address);
  TEST_ASSERT_FALSE(p_qhd->non_hs_period_inactive_next_xact);
  TEST_ASSERT_EQUAL(endpoint_addr & 0x0F, p_qhd->endpoint_number);
  TEST_ASSERT_EQUAL(_usbh_devices[dev_addr].speed, p_qhd->endpoint_speed);
  TEST_ASSERT_EQUAL(max_packet_size, p_qhd->max_package_size);
  TEST_ASSERT_EQUAL(0, p_qhd->nak_count_reload); // TDD NAK Reload disable

  TEST_ASSERT_EQUAL(hub_addr, p_qhd->hub_address);
  TEST_ASSERT_EQUAL(hub_port, p_qhd->hub_port);
  TEST_ASSERT_EQUAL(1, p_qhd->mult); // TDD operation model for mult

  TEST_ASSERT_FALSE(p_qhd->qtd_overlay.halted);
  TEST_ASSERT(p_qhd->qtd_overlay.next.terminate);
  TEST_ASSERT(p_qhd->qtd_overlay.alternate.terminate);

  //------------- HCD -------------//
  TEST_ASSERT(p_qhd->used);
  TEST_ASSERT_FALSE(p_qhd->is_removing);
  TEST_ASSERT_NULL(p_qhd->p_qtd_list_head);
  TEST_ASSERT_NULL(p_qhd->p_qtd_list_tail);
}


//--------------------------------------------------------------------+
// PIPE OPEN
//--------------------------------------------------------------------+
tusb_desc_endpoint_t const desc_ept_interrupt_out =
{
    .bLength          = sizeof(tusb_desc_endpoint_t),
    .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
    .bEndpointAddress = 0x02,
    .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
    .wMaxPacketSize   = 16,
    .bInterval        = 4
};
void verify_int_qhd(ehci_qhd_t *p_qhd, tusb_desc_endpoint_t const * desc_endpoint, uint8_t class_code)
{
  verify_open_qhd(p_qhd, desc_endpoint->bEndpointAddress, desc_endpoint->wMaxPacketSize.size);

  TEST_ASSERT_FALSE(p_qhd->head_list_flag);
  TEST_ASSERT_FALSE(p_qhd->data_toggle_control);
  TEST_ASSERT_FALSE(p_qhd->non_hs_control_endpoint);

  TEST_ASSERT_EQUAL(desc_endpoint->bEndpointAddress & 0x80 ? EHCI_PID_IN : EHCI_PID_OUT, p_qhd->pid_non_control);
}

void check_int_endpoint_link(ehci_qhd_t *p_prev, ehci_qhd_t *p_qhd)
{
  //------------- period list check -------------//
  TEST_ASSERT_EQUAL_HEX((uint32_t) p_qhd, tu_align32(p_prev->next.address));
  TEST_ASSERT_FALSE(p_prev->next.terminate);
  TEST_ASSERT_EQUAL(EHCI_QUEUE_ELEMENT_QHD, p_prev->next.type);
}

void test_open_interrupt_qhd_hs(void)
{
  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &desc_ept_interrupt_out, TUSB_CLASS_HID);

  TEST_ASSERT_EQUAL(dev_addr, pipe_hdl.dev_addr);
  TEST_ASSERT_EQUAL(TUSB_XFER_INTERRUPT, pipe_hdl.xfer_type);

  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  verify_int_qhd(p_int_qhd, &desc_ept_interrupt_out, TUSB_CLASS_HID);

  TEST_ASSERT_EQUAL(0, p_int_qhd->non_hs_interrupt_cmask);
}

void test_open_interrupt_hs_interval_1(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 1;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(0              , p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(TU_BIN8(11111111) , p_int_qhd->interrupt_smask);

  check_int_endpoint_link(period_head_arr, p_int_qhd);
}

void test_open_interrupt_hs_interval_2(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 2;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(0 , p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(4 , tu_cardof(p_int_qhd->interrupt_smask)); // either 10101010 or 01010101
  check_int_endpoint_link(period_head_arr, p_int_qhd);
}

void test_open_interrupt_hs_interval_3(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 3;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(0, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(2, tu_cardof(p_int_qhd->interrupt_smask) );
  check_int_endpoint_link(period_head_arr, p_int_qhd);
}

void test_open_interrupt_hs_interval_4(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 4;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(1, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(1, tu_cardof(p_int_qhd->interrupt_smask) );
  check_int_endpoint_link(period_head_arr, p_int_qhd);
}

void test_open_interrupt_hs_interval_5(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 5;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(2, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(1, tu_cardof(p_int_qhd->interrupt_smask) );
  check_int_endpoint_link( get_period_head(hostid, 2), p_int_qhd );
}

void test_open_interrupt_hs_interval_6(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 6;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(4, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(1, tu_cardof(p_int_qhd->interrupt_smask) );
  check_int_endpoint_link( get_period_head(hostid, 4), p_int_qhd);
}

void test_open_interrupt_hs_interval_7(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 7;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(8, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(1, tu_cardof(p_int_qhd->interrupt_smask) );
  check_int_endpoint_link( get_period_head(hostid, 8), p_int_qhd);
}

void test_open_interrupt_hs_interval_8(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 16;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(255, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(1, tu_cardof(p_int_qhd->interrupt_smask) );
  check_int_endpoint_link( get_period_head(hostid, 255), p_int_qhd);
  check_int_endpoint_link( get_period_head(hostid, 8) , p_int_qhd);
}

void test_open_interrupt_qhd_non_hs(void)
{
  _usbh_devices[dev_addr].speed = TUSB_SPEED_FULL;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &desc_ept_interrupt_out, TUSB_CLASS_HID);

  TEST_ASSERT_EQUAL(dev_addr, pipe_hdl.dev_addr);
  TEST_ASSERT_EQUAL(TUSB_XFER_INTERRUPT, pipe_hdl.xfer_type);

  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  verify_int_qhd(p_int_qhd, &desc_ept_interrupt_out, TUSB_CLASS_HID);

  TEST_ASSERT_EQUAL(desc_ept_interrupt_out.bInterval, p_int_qhd->interval_ms);
  TEST_ASSERT_EQUAL(1, p_int_qhd->interrupt_smask);
  TEST_ASSERT_EQUAL(0x1c, p_int_qhd->non_hs_interrupt_cmask);
}

void test_open_interrupt_qhd_non_hs_9(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 32;

  _usbh_devices[dev_addr].speed = TUSB_SPEED_FULL;

  //------------- Code Under TEST -------------//
  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = &ehci_data.device[ pipe_hdl.dev_addr-1].qhd[ pipe_hdl.index ];

  TEST_ASSERT_EQUAL(int_edp_interval.bInterval, p_int_qhd->interval_ms);
  check_int_endpoint_link( get_period_head(hostid, 32), p_int_qhd);
  check_int_endpoint_link( get_period_head(hostid, 8) , p_int_qhd);
}

//--------------------------------------------------------------------+
// PIPE CLOSE
//--------------------------------------------------------------------+
void test_interrupt_close(void)
{
  pipe_hdl = hcd_edpt_open(dev_addr, &desc_ept_interrupt_out, TUSB_CLASS_HID);
  p_int_qhd = qhd_get_from_pipe_handle(pipe_hdl);

  //------------- Code Under TEST -------------//
  TEST_ASSERT_EQUAL(TUSB_ERROR_NONE,
                    hcd_pipe_close(pipe_hdl) );

  TEST_ASSERT(p_int_qhd->is_removing);
  TEST_ASSERT( tu_align32(period_head_arr->next.address) != (uint32_t) p_int_qhd );
  TEST_ASSERT_EQUAL_HEX( (uint32_t) period_head_arr, tu_align32(p_int_qhd->next.address ) );
  TEST_ASSERT_EQUAL(EHCI_QUEUE_ELEMENT_QHD, p_int_qhd->next.type);
}

void test_interrupt_256ms_close(void)
{
  tusb_desc_endpoint_t int_edp_interval = desc_ept_interrupt_out;
  int_edp_interval.bInterval = 9;

  pipe_hdl = hcd_edpt_open(dev_addr, &int_edp_interval, TUSB_CLASS_HID);
  p_int_qhd = qhd_get_from_pipe_handle(pipe_hdl);

  //------------- Code Under TEST -------------//
  TEST_ASSERT_EQUAL(TUSB_ERROR_NONE,
                    hcd_pipe_close(pipe_hdl) );

  TEST_ASSERT(p_int_qhd->is_removing);
  TEST_ASSERT( tu_align32(get_period_head(hostid, 8)->address) != (uint32_t) p_int_qhd );
  TEST_ASSERT_EQUAL_HEX( (uint32_t) get_period_head(hostid, 8), tu_align32(p_int_qhd->next.address ) );
  TEST_ASSERT_EQUAL(EHCI_QUEUE_ELEMENT_QHD, p_int_qhd->next.type);
}
