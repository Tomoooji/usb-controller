#include "USBController.h"

#include <tusb.h>

USBController::USBController()
    : _parser(&_default_parser) {}

USBController::USBController(ControllerParser& parser)
    : _parser(&parser) {}

bool USBController::registerWithHost() {
    portENTER_CRITICAL(&_mux);
    if (_registered) {
        portEXIT_CRITICAL(&_mux);
        return true;
    }

    if (!USBHostHID.addDevice(this)) {
        portEXIT_CRITICAL(&_mux);
        return false;
    }

    _registered = true;
    portEXIT_CRITICAL(&_mux);
    return true;
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

bool USBController::connected() const {
    portENTER_CRITICAL(&_mux);
    const bool connected = _connected;
    portEXIT_CRITICAL(&_mux);
    return connected;
}

bool USBController::stateChanged() const {
    portENTER_CRITICAL(&_mux);
    const bool changed = _state_changed;
    portEXIT_CRITICAL(&_mux);
    return changed;
}

void USBController::clearStateChanged() {
    portENTER_CRITICAL(&_mux);
    _state_changed = false;
    portEXIT_CRITICAL(&_mux);
}

ControllerInputState USBController::state() const {
    portENTER_CRITICAL(&_mux);
    ControllerInputState snapshot = _state;
    portEXIT_CRITICAL(&_mux);
    return snapshot;
}

void USBController::setCallback(ReportCallback callback, void* arg) {
    portENTER_CRITICAL(&_mux);
    _callback = callback;
    _callback_arg = arg;
    portEXIT_CRITICAL(&_mux);
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

    portENTER_CRITICAL(&_mux);
    _dev_addr = dev_addr;
    _idx = idx;
    _connected = true;
    _state_changed = false;
    _callback_pending = false;
    _state = ControllerInputState{};
    portEXIT_CRITICAL(&_mux);
    return true;
}

void USBController::onUnmount(uint8_t dev_addr, uint8_t idx) {
    portENTER_CRITICAL(&_mux);
    if (dev_addr != _dev_addr || idx != _idx) {
        portEXIT_CRITICAL(&_mux);
        return;
    }

    _connected = false;
    _state = ControllerInputState{};
    _state_changed = true;
    _callback_pending = _callback != nullptr;
    _dev_addr = 0;
    _idx = 0;
    portEXIT_CRITICAL(&_mux);
    clearHidInterfaceBinding();
}

void USBController::onReport(
    uint8_t dev_addr,
    uint8_t idx,
    const uint8_t* report,
    uint16_t len
) {
    portENTER_CRITICAL(&_mux);
    if (!_connected || dev_addr != _dev_addr || idx != _idx) {
        portEXIT_CRITICAL(&_mux);
        return;
    }
    ControllerInputState next = _state;
    portEXIT_CRITICAL(&_mux);

    if (!_parser->parse(report, len, next)) return;

    portENTER_CRITICAL(&_mux);
    if (!_connected || dev_addr != _dev_addr || idx != _idx) {
        portEXIT_CRITICAL(&_mux);
        return;
    }
    _state = next;
    _state_changed = true;
    _callback_pending = _callback != nullptr;
    portEXIT_CRITICAL(&_mux);
}

void USBController::dispatchReportCallback() {
    portENTER_CRITICAL(&_mux);
    if (!_callback_pending || _callback == nullptr) {
        _callback_pending = false;
        portEXIT_CRITICAL(&_mux);
        return;
    }

    _callback_pending = false;
    ControllerInputState snapshot = _state;
    ReportCallback callback = _callback;
    void* callback_arg = _callback_arg;
    portEXIT_CRITICAL(&_mux);

    callback(snapshot, callback_arg);
}
