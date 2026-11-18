#pragma once

#include "DS4Input.h"
#include "DS4USBHost.h"
#include "ESPNowTransport.h"

class DS4ESPNow {
public:
    struct Config {
        ESPNowTransport::Config esp_now;
        bool accept_any_usb_hid = false;
    };

    explicit DS4ESPNow(const Config& config)
        : _config(config), _usb(config.accept_any_usb_hid) {}

    bool begin() {
        if (!_usb.begin()) {
            return false;
        }

        if (!_transport.begin(_config.esp_now)) {
            return false;
        }

        return true;
    }

    // Must be called from loop().
    void update() {
        USBHost.task();
    }

    bool connected() const {
        return _usb.connected();
    }

    const DS4InputState& state() const {
        return _usb.state();
    }

    bool sendCurrentState() {
        return _transport.send(_usb.state());
    }

    bool send(const DS4InputState& state) {
        return _transport.send(state);
    }

    DS4USBHost& usb() {
        return _usb;
    }

    ESPNowTransport& transport() {
        return _transport;
    }

private:
    Config _config;
    DS4USBHost _usb;
    ESPNowTransport _transport;
};
