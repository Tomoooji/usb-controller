#include <DS4ESPNow.h>

DS4USBHost ds4;

void onDS4(const DS4InputState& state, void*) {
    Serial.printf(
        "sequence=%u LX=%u LY=%u RX=%u RY=%u\n",
        state.sequence,
        state.left_x, state.left_y,
        state.right_x, state.right_y
    );
}

void setup() {
    Serial.begin(115200);

    ds4.setCallback(onDS4);

    if (!ds4.begin()) {
        Serial.println("USB Host start failed");
        while (true) {
            delay(1000);
        }
    }
}

void loop() {
    USBHost.task();
}
