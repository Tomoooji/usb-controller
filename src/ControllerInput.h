#pragma once

#include <stddef.h>
#include <stdint.h>

struct ControllerInputState {
    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
    uint8_t l2 = 0;
    uint8_t r2 = 0;

    bool square = false;
    bool cross = false;
    bool circle = false;
    bool triangle = false;
    bool l1 = false;
    bool r1 = false;
    bool l2_button = false;
    bool r2_button = false;
    bool create = false;
    bool options = false;
    bool l3 = false;
    bool r3 = false;
    bool ps = false;
    bool touchpad = false;

    // 0..8: up, up-right, right, down-right, down,
    // down-left, left, up-left, neutral.
    uint8_t dpad = 8;

    uint8_t battery = 0;
    bool cable_connected = false;
    uint8_t sequence = 0;
};

using DS4InputState = ControllerInputState;

class ControllerParser {
public:
    virtual ~ControllerParser() = default;

    virtual bool matches(
        uint16_t vendor_id,
        uint16_t product_id,
        uint8_t protocol,
        const uint8_t* report_desc,
        uint16_t desc_len
    ) const = 0;

    // On success, parse() replaces every field in out.
    virtual bool parse(
        const uint8_t* report,
        size_t length,
        ControllerInputState& out
    ) const = 0;
};
