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

#include "tusb_option.h"
#include "descriptor_test.h"

CFG_TUSB_MEM_SECTION ATTR_ALIGNED(4)
const uint8_t keyboard_report_descriptor[] = {
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
    HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ),
      HID_USAGE_MIN    ( 224                                    ),
      HID_USAGE_MAX    ( 231                                    ),
      HID_LOGICAL_MIN  ( 0                                      ),
      HID_LOGICAL_MAX  ( 1                                      ),

      HID_REPORT_COUNT ( 8                                      ), /* 8 bits */
      HID_REPORT_SIZE  ( 1                                      ),
      HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* maskable modifier key */

      HID_REPORT_COUNT ( 1                                      ),
      HID_REPORT_SIZE  ( 8                                      ),
      HID_INPUT        ( HID_CONSTANT                           ), /* reserved */

    HID_USAGE_PAGE  ( HID_USAGE_PAGE_LED                   ),
      HID_USAGE_MIN    ( 1                                       ),
      HID_USAGE_MAX    ( 5                                       ),
      HID_REPORT_COUNT ( 5                                       ),
      HID_REPORT_SIZE  ( 1                                       ),
      HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ), /* 5-bit Led report */

      HID_REPORT_COUNT ( 1                                       ),
      HID_REPORT_SIZE  ( 3                                       ), /* led padding */
      HID_OUTPUT       ( HID_CONSTANT                            ),

    HID_USAGE_PAGE (HID_USAGE_PAGE_KEYBOARD),
      HID_USAGE_MIN    ( 0                                   ),
      HID_USAGE_MAX    ( 101                                 ),
      HID_LOGICAL_MIN  ( 0                                   ),
      HID_LOGICAL_MAX  ( 101                                 ),

      HID_REPORT_COUNT ( 6                                   ),
      HID_REPORT_SIZE  ( 8                                   ),
      HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ), /* keycodes array 6 items */
  HID_COLLECTION_END
};

CFG_TUSB_MEM_SECTION ATTR_ALIGNED(4)
const uint8_t mouse_report_descriptor[] = {
  HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     ),
  HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE    ),
  HID_COLLECTION ( HID_COLLECTION_APPLICATION ),
    HID_USAGE      (HID_USAGE_DESKTOP_POINTER),

    HID_COLLECTION ( HID_COLLECTION_PHYSICAL ),
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON ),
        HID_USAGE_MIN    ( 1                                      ),
        HID_USAGE_MAX    ( 3                                      ),
        HID_LOGICAL_MIN  ( 0                                      ),
        HID_LOGICAL_MAX  ( 1                                      ),

        HID_REPORT_COUNT ( 3                                      ), /* Left, Right and Middle mouse*/
        HID_REPORT_SIZE  ( 1                                      ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ),

        HID_REPORT_COUNT ( 1                                      ),
        HID_REPORT_SIZE  ( 5                                      ),
        HID_INPUT        ( HID_CONSTANT                           ), /* reserved */

      HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP ),
        HID_USAGE        ( HID_USAGE_DESKTOP_X                    ),
        HID_USAGE        ( HID_USAGE_DESKTOP_Y                    ),
        HID_LOGICAL_MIN  ( 0x81                                   ), /* -127 */
        HID_LOGICAL_MAX  ( 0x7f                                   ), /* 127  */

        HID_REPORT_COUNT ( 2                                      ), /* X, Y position */
        HID_REPORT_SIZE  ( 8                                      ),
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* relative values */
    HID_COLLECTION_END,

  HID_COLLECTION_END
};


CFG_TUSB_MEM_SECTION ATTR_ALIGNED(4)
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_TYPE_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,

    .bMaxPacketSize0    = 64,

    .idVendor           = 0x1FC9,
    .idProduct          = 0x4000,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x02
} ;


CFG_TUSB_MEM_SECTION ATTR_ALIGNED(4)
const app_configuration_desc_t desc_configuration =
{
    .configuration =
    {
        .bLength             = sizeof(tusb_desc_configuration_t),
        .bDescriptorType     = TUSB_DESC_TYPE_CONFIGURATION,

        .wTotalLength        = sizeof(app_configuration_desc_t) - 1, // exclude termination
        .bNumInterfaces      = 5,

        .bConfigurationValue = 1,
        .iConfiguration      = 0x00,
        .bmAttributes        = TUSB_DESC_CONFIG_ATT_BUS_POWER,
        .bMaxPower           = TUSB_DESC_CONFIG_POWER_MA(100)
    },

    //------------- HID Keyboard -------------//
    .keyboard_interface =
    {
        .bLength            = sizeof(tusb_desc_interface_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 1,
        .bAlternateSetting  = 0x00,
        .bNumEndpoints      = 1,
        .bInterfaceClass    = TUSB_CLASS_HID,
        .bInterfaceSubClass = HID_SUBCLASS_BOOT,
        .bInterfaceProtocol = HID_PROTOCOL_KEYBOARD,
        .iInterface         = 0x00
    },

    .keyboard_hid =
    {
        .bLength           = sizeof(tusb_hid_descriptor_hid_t),
        .bDescriptorType   = HID_DESC_TYPE_HID,
        .bcdHID            = 0x0111,
        .bCountryCode      = HID_LOCAL_NotSupported,
        .bNumDescriptors   = 1,
        .bReportType       = HID_DESC_TYPE_REPORT,
        .wReportLength     = sizeof(keyboard_report_descriptor)
    },

    .keyboard_endpoint =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 0x81,
        .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
        .wMaxPacketSize   = 0x08,
        .bInterval        = 0x0A
    },

    //------------- HID Mouse -------------//
    .mouse_interface =
    {
        .bLength            = sizeof(tusb_desc_interface_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 2,
        .bAlternateSetting  = 0x00,
        .bNumEndpoints      = 1,
        .bInterfaceClass    = TUSB_CLASS_HID,
        .bInterfaceSubClass = HID_SUBCLASS_BOOT,
        .bInterfaceProtocol = HID_PROTOCOL_MOUSE,
        .iInterface         = 0x00
    },

    .mouse_hid =
    {
        .bLength           = sizeof(tusb_hid_descriptor_hid_t),
        .bDescriptorType   = HID_DESC_TYPE_HID,
        .bcdHID            = 0x0111,
        .bCountryCode      = HID_LOCAL_NotSupported,
        .bNumDescriptors   = 1,
        .bReportType       = HID_DESC_TYPE_REPORT,
        .wReportLength     = sizeof(mouse_report_descriptor)
    },

    .mouse_endpoint =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 0x82,
        .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
        .wMaxPacketSize   = 0x08,
        .bInterval        = 0x0A
    },

    //------------- Mass Storage -------------//
    .msc_interface =
    {
        .bLength            = sizeof(tusb_desc_interface_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 3,
        .bAlternateSetting  = 0x00,
        .bNumEndpoints      = 2,
        .bInterfaceClass    = TUSB_CLASS_MSC,
        .bInterfaceSubClass = MSC_SUBCLASS_SCSI,
        .bInterfaceProtocol = MSC_PROTOCOL_BOT,
        .iInterface         = 0x00
    },

    .msc_endpoint_in =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 0x83,
        .bmAttributes     = { .xfer = TUSB_XFER_BULK },
        .wMaxPacketSize   = 512,
        .bInterval        = 1
    },

    .msc_endpoint_out =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 0x03,
        .bmAttributes     = { .xfer = TUSB_XFER_BULK },
        .wMaxPacketSize   = 512,
        .bInterval        = 1
    },

    //------------- CDC Serial -------------//
    .cdc_comm_interface =
    {
        .bLength            = sizeof(tusb_desc_interface_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 4,
        .bAlternateSetting  = 0,
        .bNumEndpoints      = 1,
        .bInterfaceClass    = TUSB_CLASS_CDC,
        .bInterfaceSubClass = CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL,
        .bInterfaceProtocol = CDC_COMM_PROTOCOL_ATCOMMAND,
        .iInterface         = 0x00
    },

    .cdc_header =
    {
        .bLength            = sizeof(cdc_desc_func_header_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE_CLASS_SPECIFIC,
        .bDescriptorSubType = CDC_FUNC_DESC_HEADER,
        .bcdCDC             = 0x0120
    },

    .cdc_acm =
    {
        .bLength            = sizeof(cdc_desc_func_acm_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE_CLASS_SPECIFIC,
        .bDescriptorSubType = CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT,
        .bmCapabilities     = { // 0x06
            .support_line_request = 1,
            .support_send_break   = 1
        }
    },

    .cdc_union =
    {
        .bLength                  = sizeof(cdc_desc_func_union_t), // plus number of
        .bDescriptorType          = TUSB_DESC_TYPE_INTERFACE_CLASS_SPECIFIC,
        .bDescriptorSubType       = CDC_FUNC_DESC_UNION,
        .bControlInterface        = 1,
        .bSubordinateInterface    = 2,
    },

    .cdc_endpoint_notification =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 0x84,
        .bmAttributes     = { .xfer = TUSB_XFER_INTERRUPT },
        .wMaxPacketSize   = 8,
        .bInterval        = 0x0a // lowest polling rate
    },

    //------------- CDC Data Interface -------------//
    .cdc_data_interface =
    {
        .bLength            = sizeof(tusb_desc_interface_t),
        .bDescriptorType    = TUSB_DESC_TYPE_INTERFACE,
        .bInterfaceNumber   = 5,
        .bAlternateSetting  = 0x00,
        .bNumEndpoints      = 2,
        .bInterfaceClass    = TUSB_CLASS_CDC_DATA,
        .bInterfaceSubClass = 0,
        .bInterfaceProtocol = 0,
        .iInterface         = 0x00
    },

    .cdc_endpoint_out =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 5,
        .bmAttributes     = { .xfer = TUSB_XFER_BULK },
        .wMaxPacketSize   = 64,
        .bInterval        = 0
    },

    .cdc_endpoint_in =
    {
        .bLength          = sizeof(tusb_desc_endpoint_t),
        .bDescriptorType  = TUSB_DESC_TYPE_ENDPOINT,
        .bEndpointAddress = 0x85,
        .bmAttributes     = { .xfer = TUSB_XFER_BULK },
        .wMaxPacketSize   = 64,
        .bInterval        = 0
    },

    // TODO CDC & RNDIS
    .ConfigDescTermination = 0,
};
