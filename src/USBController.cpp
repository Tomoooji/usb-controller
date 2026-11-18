#include "USBController.h"

#include <tusb.h>

USBController::USBController()
    : _parser(&_default_parser) {}

USBController::USBController(ControllerParser& parser)
    : _parser(&parser) {}

bool USBController::registerWithHost() {
    return USBHostHID.addDevice(this);
}

bool USBController::begin() {
    if (!registerWithHost()) {
        return false;
    }
    return USBHost.begin();
}

void USBController::update() {
    USBHost.task();
}

bool USBController::claim(
    uint8_t dev_addr,
    uint8_t idx,
    uint8_t protocol,
    const uint8_t* report_desc,
    uint16_t desc_len
) {
    uint16_t vendor_id = 0;
    uint16_t product_id = 0;
    if (!tuh_vid_pid_get(dev_addr, &vendor_id, &product_id)) {
        return false;
    }

    if (!_parser->matches(
            vendor_id, product_id, protocol, report_desc, desc_len)) {
        return false;
    }

    if (!bindHidInterface(dev_addr, idx)) {
        return false;
    }

    _dev_addr = dev_addr;
    _idx = idx;
    _connected = true;
    _state_changed = false;
    _callback_pending = false;
    _state = ControllerInputState{};
    return true;
}

void USBController::onUnmount(uint8_t dev_addr, uint8_t idx) {
    if (dev_addr != _dev_addr || idx != _idx) {
        return;
    }

    _connected = false;
    _state_changed = false;
    _callback_pending = false;
    clearHidInterfaceBinding();
    _dev_addr = 0;
    _idx = 0;
}

void USBController::onReport(
    uint8_t dev_addr,
    uint8_t idx,
    const uint8_t* report,
    uint16_t len
) {
    if (!_connected || dev_addr != _dev_addr || idx != _idx) {
        return;
    }

    ControllerInputState next = _state;
    if (!_parser->parse(report, len, next)) {
        return;
    }

    _state = next;
    _state_changed = true;
    _callback_pending = _callback != nullptr;
}

void USBController::dispatchReportCallback() {
    if (!_callback_pending || _callback == nullptr) {
        _callback_pending = false;
        return;
    }

    _callback_pending = false;
    _callback(_state, _callback_arg);
}
