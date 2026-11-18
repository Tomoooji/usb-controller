#include <USBController.h>

USBController controller;

void setup() {
    Serial.begin(115200);

    if (!controller.begin()) {
        Serial.println("USB controller initialization failed");
        while (true) {
            delay(1000);
        }
    }
}

void loop() {
    controller.update();

    ControllerInputState state;
    if (controller.takeState(state)) {
        Serial.printf(
            "LX=%u LY=%u RX=%u RY=%u cross=%d circle=%d\n",
            state.left_x, state.left_y,
            state.right_x, state.right_y,
            state.cross, state.circle
        );
    }
}
