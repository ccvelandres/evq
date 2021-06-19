#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "usb.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/dwc/otg_fs.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/stm32/gpio.h>

#include <FreeRTOS.h>
#include <task.h>

typedef enum usbd_request_return_codes usbd_request_return_codes_e;

static usbd_device  *usb_dev      = NULL;
static volatile bool usb_ready_tx = false;
static uint8_t       usb_control_buffer[CFG_USB_CONTROL_BUFFER_SIZE];

static void usb_serial_notify_state(usbd_device *usbd_dev, const uint8_t ep, const uint16_t iface)
{
    char buf[10];
    memset(buf, 0, sizeof(buf));
    struct usb_cdc_notification *nf = (void *)buf;
    nf->bmRequestType               = 0xA1;
    nf->bNotification               = USB_CDC_NOTIFY_SERIAL_STATE;
    nf->wValue                      = 0;
    nf->wIndex                      = iface;
    nf->wLength                     = 2;
    buf[8]                          = 3;
    buf[9]                          = 0;
    usbd_ep_write_packet(usbd_dev, ep, buf, sizeof(buf));
}

static usbd_request_return_codes_e usb_serial_control_cb(usbd_device                    *usbd_dev,
                                                         struct usb_setup_data          *req,
                                                         uint8_t                       **buf,
                                                         uint16_t                       *len,
                                                         usbd_control_complete_callback *complete)
{
    switch (req->bRequest)
    {
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE: {
        usb_serial_notify_state(usbd_dev, CFG_USB_SERIAL_ENDPOINT_ISR, req->wIndex);
        // extract dte state
        usb_ready_tx = req->wValue & 0b1;
        return USBD_REQ_HANDLED;
    }
    break;
    case USB_CDC_REQ_SET_LINE_CODING:
        if (*len < sizeof(struct usb_cdc_line_coding))
        {
            return USBD_REQ_NOTSUPP;
        }
        return USBD_REQ_HANDLED;
    default:
        break;
    }
    return USBD_REQ_NOTSUPP;
}

static void usb_serial_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
    (void)ep;
    char buf[64];
    int  len = usbd_ep_read_packet(usbd_dev, ep, buf, 64);
    if (len)
    {
        usbd_ep_write_packet(usbd_dev, CFG_USB_SERIAL_ENDPOINT, buf, len);
    }
}

static void usb_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
    // Rx endpoint
    usbd_ep_setup(usbd_dev,
                  CFG_USB_SERIAL_ENDPOINT,
                  USB_ENDPOINT_ATTR_BULK,
                  CFG_USB_EP_PACKET_SIZE,
                  usb_serial_rx_cb);
    // Tx endpoint
    usbd_ep_setup(usbd_dev,
                  CFG_USB_SERIAL_ENDPOINT | USB_REQ_TYPE_IN,
                  USB_ENDPOINT_ATTR_BULK,
                  CFG_USB_EP_PACKET_SIZE,
                  NULL);
    // Interrupt endpoint
    usbd_ep_setup(usbd_dev,
                  CFG_USB_SERIAL_ENDPOINT_ISR | USB_REQ_TYPE_IN,
                  USB_ENDPOINT_ATTR_INTERRUPT,
                  16,
                  NULL);

    usbd_register_control_callback(usbd_dev,
                                   USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
                                   USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
                                   usb_serial_control_cb);

    usb_serial_notify_state(usbd_dev, CFG_USB_SERIAL_ENDPOINT, 0);
}

void setupUsb(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO11 | GPIO12);
    gpio_set_af(GPIOA, GPIO_AF10, GPIO11 | GPIO12);

    GPIOA_OSPEEDR &= 0x3c00000cU;
    GPIOA_OSPEEDR |= 0x28000008U;

    rcc_periph_clock_enable(RCC_OTGFS);

    usb_dev = usbd_init(&stm32f107_usb_driver,
                        &dev_desc,
                        &cfg_desc,
                        usb_strings,
                        3,
                        usb_control_buffer,
                        CFG_USB_CONTROL_BUFFER_SIZE);

    usbd_register_set_config_callback(usb_dev, usb_set_config_cb);

    nvic_set_priority(NVIC_OTG_FS_IRQ, (1 << 4));
    nvic_enable_irq(NVIC_OTG_FS_IRQ);

    OTG_FS_GCCFG |= OTG_GCCFG_NOVBUSSENS | OTG_GCCFG_PWRDWN;
    OTG_FS_GCCFG &= ~(OTG_GCCFG_VBUSBSEN | OTG_GCCFG_VBUSASEN);
}

void usb_serial_write(char *data, uint32_t len)
{
    uint32_t ret = 0;
    if (usb_ready_tx)
    {
        if ((CFG_USB_EP_PACKET_SIZE / 2) < len)
        {
            uint32_t remainingBytes = len;
            while (remainingBytes > 0)
            {
                uint32_t packetSize = remainingBytes > (CFG_USB_EP_PACKET_SIZE / 2)
                                        ? (CFG_USB_EP_PACKET_SIZE / 2) - 1
                                        : remainingBytes;

                ret = usbd_ep_write_packet(usb_dev,
                                           CFG_USB_SERIAL_ENDPOINT,
                                           &data[len - remainingBytes],
                                           packetSize);
                remainingBytes -= ret;
            }
        }
        else
        {
            ret = usbd_ep_write_packet(usb_dev, CFG_USB_SERIAL_ENDPOINT, data, len);
        }
    }
}

void otg_fs_isr(void) { usbd_poll(usb_dev); }

bool usb_serial_ready(void) { return usb_ready_tx; }