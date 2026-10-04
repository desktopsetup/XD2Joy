#ifndef JC_USB_DEV_H
#define JC_USB_DEV_H

#include <stdbool.h>
#include <stdint.h>

bool jc_usb_kbd_ready(void);

void jc_usb_kbd_send(uint8_t mods, const uint8_t keys[6]);

void jc_usb_kbd_release_all(void);

#endif
