#include "ESPNowTransport.h"

#include <WiFi.h>
#include <esp_wifi.h>

namespace {
bool sameMac(const uint8_t* a, const uint8_t* b) {
    for (size_t i = 0; i < 6; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}
}

bool ESPNowTransport::begin(const Config& config) {
    _config = config;

    WiFi.mode(WIFI_STA);

    if (_config.channel != 0) {
        // ESP-NOW peers must use the same Wi-Fi channel.
        if (esp_wifi_set_channel(_config.channel, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
            return false;
        }
    }

    if (esp_now_init() != ESP_OK) {
        return false;
    }

    esp_now_peer_info_t peer{};
    memcpy(peer.peer_addr, _config.peer, 6);
    peer.channel = _config.channel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = _config.encrypt;

    if (_config.encrypt) {
        memcpy(peer.lmk, _config.lmk, 16);
    }

    // Make repeated begin() calls harmless when the peer already exists.
    if (esp_now_is_peer_exist(_config.peer)) {
        _initialized = true;
        return true;
    }

    if (esp_now_add_peer(&peer) != ESP_OK) {
        return false;
    }

    _initialized = true;
    return true;
}

bool ESPNowTransport::send(const DS4InputState& state) {
    return sendRaw(&state, sizeof(state));
}

bool ESPNowTransport::sendRaw(const void* data, size_t length) {
    if (!_initialized || data == nullptr || length == 0) {
        return false;
    }

    if (length > ESP_NOW_MAX_DATA_LEN) {
        return false;
    }

    return esp_now_send(
        _config.peer,
        static_cast<const uint8_t*>(data),
        length
    ) == ESP_OK;
}
