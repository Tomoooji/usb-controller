#include "DS4Input.h"

bool DS4Parser::parse(const uint8_t* report, size_t length, DS4InputState& out) const {
    if (report == nullptr || length < USB_REPORT_SIZE) {
        return false;
    }

    if (report[0] != USB_REPORT_ID) {
        return false;
    }

    // USB DS4 input report:
    //  0  report ID
    //  1  LX
    //  2  LY
    //  3  RX
    //  4  RY
    //  5  L2
    //  6  R2
    //  7  sequence
    //  8  face buttons + D-pad
    //  9  shoulders / options / sticks
    // 10  PS / touchpad
    // 30  battery/cable status (common DS4 USB layout)
    //
    // The parser intentionally only consumes fields needed by the library's
    // public state. The remaining bytes remain available through raw reports
    // if future versions expose them.

    out.left_x = report[1];
    out.left_y = report[2];
    out.right_x = report[3];
    out.right_y = report[4];
    out.l2 = report[5];
    out.r2 = report[6];
    out.sequence = report[7];

    const uint8_t buttons0 = report[8];
    const uint8_t buttons1 = report[9];
    const uint8_t buttons2 = report[10];

    // Byte 8: low nibble is the HID hat switch.
    out.dpad = buttons0 & 0x0F;

    out.square   = buttons0 & 0x10;
    out.cross    = buttons0 & 0x20;
    out.circle   = buttons0 & 0x40;
    out.triangle = buttons0 & 0x80;

    out.l1        = buttons1 & 0x01;
    out.r1        = buttons1 & 0x02;
    out.l2_button = buttons1 & 0x04;
    out.r2_button = buttons1 & 0x08;
    out.create    = buttons1 & 0x10;
    out.options   = buttons1 & 0x20;
    out.l3        = buttons1 & 0x40;
    out.r3        = buttons1 & 0x80;

    out.ps        = buttons2 & 0x01;
    out.touchpad  = buttons2 & 0x02;

    // Linux's DS4 parser defines the USB report as:
    // common[32] + touch data + reserved. In the common portion,
    // status[0] is at offset 29 from the report after the report ID,
    // i.e. absolute byte 30.
    out.battery = report[30] & 0x0F;
    out.cable_connected = report[30] & 0x10;

    return true;
}
