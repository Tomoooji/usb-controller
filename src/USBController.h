#pragma once

#include <USBHost.h>
#include <USBHostHID.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "ControllerInput.h"
#include "DS4Parser.h"
#include "DualSenseParser.h"

class USBController : public USBHostHIDDevice {
public:
    using ReportCallback = void (*)(const ControllerInputState&, void*);
    using DisconnectCallback = void (*)(void*);

    USBController();
    // Custom parser references must remain valid for the USBController lifetime.
    explicit USBController(ControllerParser& parser);
    ~USBController() override;

    USBController(const USBController&) = delete;
    USBController& operator=(const USBController&) = delete;

    bool begin();
    bool registerWithHost();
    void update();

    bool connected() const;
    bool stateChanged() const;
    void clearStateChanged();
    ControllerInputState state() const;
    bool takeState(ControllerInputState& out);
    bool connectionChanged() const;
    void clearConnectionChanged();
    bool takeConnectionState(bool& connected);

    void setCallback(ReportCallback callback, void* arg = nullptr);
    void setDisconnectCallback(DisconnectCallback callback, void* arg = nullptr);

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
    DualSenseParser _dualsense_parser;
    ControllerParser* _parser;
    SemaphoreHandle_t _mutex = nullptr;
    bool _registered = false;
    bool _connected = false;
    bool _claiming = false;
    bool _state_changed = false;
    bool _connection_changed = false;
    bool _callback_pending = false;
    ControllerInputState _state{};
    ReportCallback _callback = nullptr;
    void* _callback_arg = nullptr;
    DisconnectCallback _disconnect_callback = nullptr;
    void* _disconnect_callback_arg = nullptr;
    bool _disconnect_pending = false;
};
