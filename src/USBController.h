#pragma once

#include <USBHost.h>
#include <USBHostHID.h>

#include "ControllerInput.h"
#include "DS4Parser.h"

class USBController : public USBHostHIDDevice {
public:
    using ReportCallback = void (*)(const ControllerInputState&, void*);

    USBController();
    explicit USBController(ControllerParser& parser);

    bool begin();
    bool registerWithHost();
    void update();

    bool connected() const { return _connected; }
    bool stateChanged() const { return _state_changed; }
    void clearStateChanged() { _state_changed = false; }
    const ControllerInputState& state() const { return _state; }

    void setCallback(ReportCallback callback, void* arg = nullptr) {
        _callback = callback;
        _callback_arg = arg;
    }

protected:
    bool claim(
        uint8_t dev_addr,
        uint8_t idx,
        uint8_t protocol,
        const uint8_t* report_desc,
        uint16_t desc_len
    ) override;

    void onUnmount(uint8_t dev_addr, uint8_t idx) override;

    void onReport(
        uint8_t dev_addr,
        uint8_t idx,
        const uint8_t* report,
        uint16_t len
    ) override;

private:
    void dispatchReportCallback() override;

    DS4Parser _default_parser;
    ControllerParser* _parser;
    bool _connected = false;
    bool _state_changed = false;
    bool _callback_pending = false;
    ControllerInputState _state{};
    ReportCallback _callback = nullptr;
    void* _callback_arg = nullptr;
    uint8_t _dev_addr = 0;
    uint8_t _idx = 0;
};
