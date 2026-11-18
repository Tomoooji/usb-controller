#include <DS4ESPNow.h>

// Replace this with the receiver's STA MAC address.
DS4ESPNow::Config config = {
    .esp_now = {
        .peer = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56},
        .channel = 1,
        .encrypt = false,
    },
    .accept_any_usb_hid = false,
};

DS4ESPNow ds4(config);

void setup() {
    Serial.begin(115200);

    if (!ds4.begin()) {
        Serial.println("DS4ESPNow begin failed");
        while (true) {
            delay(1000);
        }
    }

    Serial.println("DS4ESPNow started");
}

void loop() {
    ds4.update();

    static uint8_t last_sequence = 0xFF;

    if (ds4.connected() && ds4.state().sequence != last_sequence) {
        last_sequence = ds4.state().sequence;

        // Send one packet per new USB input report.
        if (!ds4.sendCurrentState()) {
            Serial.println("ESP-NOW send failed");
        }

        const auto& s = ds4.state();

        Serial.printf(
            "LX=%3u LY=%3u RX=%3u RY=%3u L2=%3u R2=%3u "
            "buttons=%c%c%c%c\n",
            s.left_x, s.left_y,
            s.right_x, s.right_y,
            s.l2, s.r2,
            s.cross ? 'X' : '-',
            s.circle ? 'O' : '-',
            s.square ? 'S' : '-',
            s.triangle ? 'T' : '-'
        );
    }
}
