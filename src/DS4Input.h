#pragma once

#include <Arduino.h>
#include <stdint.h>

struct DS4InputState {
    uint8_t left_x = 128;
    uint8_t left_y = 128;
    uint8_t right_x = 128;
    uint8_t right_y = 128;
    uint8_t l2 = 0;
    uint8_t r2 = 0;

    // Face buttons / shoulders / system buttons.
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

    uint8_t battery = 0;       // DS4 status nibble, normally 0..11.
    bool cable_connected = false;

    // USB report sequence is useful for detecting dropped/repeated reports.
    uint8_t sequence = 0;
};

class DS4Parser {
public:
    static constexpr size_t USB_REPORT_SIZE = 64;
    static constexpr uint8_t USB_REPORT_ID = 0x01;

    bool parse(const uint8_t* report, size_t length, DS4InputState& out) const;

    // Optional convenience helpers for normalized signed stick values.
    static int16_t axis(uint8_t raw) {
        return static_cast<int16_t>(raw) - 128;
    }
};
