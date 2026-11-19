#include "DualSenseParser.h"

bool DualSenseParser::matches(
    uint16_t vendor_id,
    uint16_t product_id,
    uint8_t protocol,
    const uint8_t* report_desc,
    uint16_t desc_len
) const {
    (void)protocol;
    (void)report_desc;
    (void)desc_len;

    return vendor_id == SONY_VID &&
           (product_id == DUALSENSE_PID ||
            product_id == DUALSENSE_EDGE_PID);
}

bool DualSenseParser::parse(
    const uint8_t* report,
    size_t length,
    ControllerInputState& out
) const {
    if (report == nullptr || length < MIN_USB_REPORT_SIZE ||
        report[0] != USB_REPORT_ID) {
        return false;
    }

    out.left_x = report[1];
    out.left_y = report[2];
    out.right_x = report[3];
    out.right_y = report[4];
    out.l2 = report[5];
    out.r2 = report[6];

    const uint8_t buttons0 = report[8];
    const uint8_t buttons1 = report[9];
    const uint8_t buttons2 = report[10];
    const uint8_t raw_dpad = buttons0 & 0x0F;

    out.dpad = raw_dpad <= 7 ? raw_dpad : 8;
    out.square = (buttons0 & 0x10) != 0;
    out.cross = (buttons0 & 0x20) != 0;
    out.circle = (buttons0 & 0x40) != 0;
    out.triangle = (buttons0 & 0x80) != 0;

    out.l1 = (buttons1 & 0x01) != 0;
    out.r1 = (buttons1 & 0x02) != 0;
    out.l2_button = (buttons1 & 0x04) != 0;
    out.r2_button = (buttons1 & 0x08) != 0;
    out.create = (buttons1 & 0x10) != 0;
    out.options = (buttons1 & 0x20) != 0;
    out.l3 = (buttons1 & 0x40) != 0;
    out.r3 = (buttons1 & 0x80) != 0;

    out.ps = (buttons2 & 0x01) != 0;
    out.touchpad = (buttons2 & 0x02) != 0;
    out.sequence = report[7];

    // The USB status block starts at byte 53; its low nibble is battery level.
    out.battery = report[53] & 0x0F;
    out.cable_connected = false;
    return true;
}
