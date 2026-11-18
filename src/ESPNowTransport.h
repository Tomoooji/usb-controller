#pragma once

#include <Arduino.h>
#include <esp_now.h>
#include "DS4Input.h"

class ESPNowTransport {
public:
    struct Config {
        uint8_t peer[6] = {0};
        uint8_t channel = 0;       // 0 = use current Wi-Fi channel.
        bool encrypt = false;
        uint8_t lmk[16] = {0};
    };

    bool begin(const Config& config);

    bool send(const DS4InputState& state);

    bool sendRaw(const void* data, size_t length);

    bool initialized() const { return _initialized; }

private:
    Config _config{};
    bool _initialized = false;
};
