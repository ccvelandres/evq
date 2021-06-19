#pragma once

#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>

/**
 * USB Configuration is based from libopencm3 example for
 * waveshare-open103r usbserial
 */

#define CFG_USB_SERIAL_ENDPOINT     (1)
// #define CFG_USB_SERIAL_ENDPOINT_RX  (CFG_USB_SERIAL_ENDPOINT)
// #define CFG_USB_SERIAL_ENDPOINT_TX  ((CFG_USB_SERIAL_ENDPOINT + 1) | USB_REQ_TYPE_IN)
#define CFG_USB_SERIAL_ENDPOINT_ISR (CFG_USB_SERIAL_ENDPOINT + 1)
#define CFG_USB_CONTROL_BUFFER_SIZE (256)
#define CFG_USB_EP_PACKET_SIZE      (64)

static const struct usb_device_descriptor dev_desc = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = USB_CLASS_CDC,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x0483,
    .idProduct          = 0x5740,
    .bcdDevice          = 0x0200,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .iSerialNumber      = 3,
    .bNumConfigurations = 1,
};

static const struct usb_endpoint_descriptor comm_endp[] = {
    {
     .bLength          = USB_DT_ENDPOINT_SIZE,
     .bDescriptorType  = USB_DT_ENDPOINT,
     .bEndpointAddress = CFG_USB_SERIAL_ENDPOINT_ISR,
     .bmAttributes     = USB_ENDPOINT_ATTR_INTERRUPT,
     .wMaxPacketSize   = 16,
     .bInterval        = 255,
     }
};

static const struct usb_endpoint_descriptor data_endp[] = {
    {
     .bLength          = USB_DT_ENDPOINT_SIZE,
     .bDescriptorType  = USB_DT_ENDPOINT,
     .bEndpointAddress = CFG_USB_SERIAL_ENDPOINT,
     .bmAttributes     = USB_ENDPOINT_ATTR_BULK,
     .wMaxPacketSize   = CFG_USB_EP_PACKET_SIZE,
     .bInterval        = 1,
     },
    {
     .bLength          = USB_DT_ENDPOINT_SIZE,
     .bDescriptorType  = USB_DT_ENDPOINT,
     .bEndpointAddress = CFG_USB_SERIAL_ENDPOINT | USB_REQ_TYPE_IN,
     .bmAttributes     = USB_ENDPOINT_ATTR_BULK,
     .wMaxPacketSize   = CFG_USB_EP_PACKET_SIZE,
     .bInterval        = 1,
     }
};

static const struct
{
    struct usb_cdc_header_descriptor          header;
    struct usb_cdc_call_management_descriptor call_mgmt;
    struct usb_cdc_acm_descriptor             acm;
    struct usb_cdc_union_descriptor           cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
  .header = {
    .bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_HEADER,
    .bcdCDC = 0x0110,
  },
  .call_mgmt = {
    .bFunctionLength = 
    sizeof(struct usb_cdc_call_management_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
    .bmCapabilities = 0,
    .bDataInterface = 1,
  },
  .acm = {
    .bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_ACM,
    .bmCapabilities = 0,
  },
  .cdc_union = {
    .bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
    .bDescriptorType = CS_INTERFACE,
    .bDescriptorSubtype = USB_CDC_TYPE_UNION,
    .bControlInterface = 0,
    .bSubordinateInterface0 = 1, 
  }
};

static const struct usb_interface_descriptor comm_iface[] = {
    {.bLength            = USB_DT_INTERFACE_SIZE,
     .bDescriptorType    = USB_DT_INTERFACE,
     .bInterfaceNumber   = 0,
     .bAlternateSetting  = 0,
     .bNumEndpoints      = 1,
     .bInterfaceClass    = USB_CLASS_CDC,
     .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
     .bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
     .iInterface         = 0,
     .endpoint           = comm_endp,
     .extra              = &cdcacm_functional_descriptors,
     .extralen           = sizeof(cdcacm_functional_descriptors)}
};

static const struct usb_interface_descriptor data_iface[] = {
    {
     .bLength            = USB_DT_INTERFACE_SIZE,
     .bDescriptorType    = USB_DT_INTERFACE,
     .bInterfaceNumber   = 1,
     .bAlternateSetting  = 0,
     .bNumEndpoints      = 2,
     .bInterfaceClass    = USB_CLASS_DATA,
     .bInterfaceSubClass = 0,
     .bInterfaceProtocol = 0,
     .iInterface         = 0,
     .endpoint           = data_endp,
     }
};

static const struct usb_interface ifaces[] = {
    {
     .num_altsetting = 1,
     .altsetting     = comm_iface,
     },
    {
     .num_altsetting = 1,
     .altsetting     = data_iface,
     }
};

static const struct usb_config_descriptor cfg_desc = {
    .bLength             = USB_DT_CONFIGURATION_SIZE,
    .bDescriptorType     = USB_DT_CONFIGURATION,
    .wTotalLength        = 0,
    .bNumInterfaces      = 2,
    .bConfigurationValue = 1,
    .iConfiguration      = 0,
    .bmAttributes        = 0x80,
    .bMaxPower           = 0x32,
    .interface           = ifaces,
};

static const __attribute__((__used__)) char *usb_strings[] = {
    "DEMO_1",
    "DEMO_2",
    "DEMO_3",
};

/**
 * Write data to usb serial endpoint
*/
void usb_serial_write(const void *data, const uint16_t len);

/** Returns whether serial console is connected */
bool usb_serial_ready(void);