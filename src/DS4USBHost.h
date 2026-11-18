#pragma once

#include <Arduino.h>
#include <USBHostHID.h>
#include "DS4Input.h"

class DS4USBHost : public USBHostHIDDevice {
public:
    using ReportCallback = void (*)(const DS4InputState&, void*);

    explicit DS4USBHost(bool accept_any_device = false);

    // Registers this handler and starts the global USB Host stack.
    bool begin();

    // Registers only. Useful when the application owns USBHost.begin().
    bool registerWithHost();

    void update();

    bool connected() const { return _connected; }
    const DS4InputState& state() const { return _state; }

    void setCallback(ReportCallback callback, void* arg = nullptr) {
        _callback = callback;
        _callback_arg = arg;
    }

protected:
    bool claim(uint8_t dev_addr,
               uint8_t idx,
               uint8_t protocol,
               const uint8_t* report_desc,
               uint16_t desc_len) override;

    void onUnmount(uint8_t dev_addr, uint8_t idx) override;

    void onReport(uint8_t dev_addr,
                  uint8_t idx,
                  const uint8_t* report,
                  uint16_t len) override;

private:
    static constexpr uint16_t SONY_VID = 0x054C;
    static constexpr uint16_t DS4_PID_V1 = 0x05C4;
    static constexpr uint16_t DS4_PID_V2 = 0x09CC;

    bool _accept_any_device;
    bool _connected = false;

    DS4Parser _parser;
    DS4InputState _state{};

    ReportCallback _callback = nullptr;
    void* _callback_arg = nullptr;

    uint8_t _dev_addr = 0;
    uint8_t _idx = 0;
};
