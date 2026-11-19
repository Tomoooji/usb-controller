#pragma once

#include "ControllerInput.h"

class DualSenseParser final : public ControllerParser {
public:
    static constexpr size_t MIN_USB_REPORT_SIZE = 54;
    static constexpr uint8_t USB_REPORT_ID = 0x01;
    static constexpr uint16_t SONY_VID = 0x054C;
    static constexpr uint16_t DUALSENSE_PID = 0x0CE6;
    static constexpr uint16_t DUALSENSE_EDGE_PID = 0x0DF2;

    bool matches(
        uint16_t vendor_id,
        uint16_t product_id,
        uint8_t protocol,
        const uint8_t* report_desc,
        uint16_t desc_len
    ) const override;

    bool parse(
        const uint8_t* report,
        size_t length,
        ControllerInputState& out
    ) const override;
};
