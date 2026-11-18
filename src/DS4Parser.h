#pragma once

#include "ControllerInput.h"

class DS4Parser final : public ControllerParser {
public:
    static constexpr size_t MIN_USB_REPORT_SIZE = 31;
    static constexpr uint8_t USB_REPORT_ID = 0x01;
    static constexpr uint16_t SONY_VID = 0x054C;
    static constexpr uint16_t DS4_PID_V1 = 0x05C4;
    static constexpr uint16_t DS4_PID_V2 = 0x09CC;

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
