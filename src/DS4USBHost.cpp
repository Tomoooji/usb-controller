#include "DS4USBHost.h"

#include <tusb.h>

DS4USBHost::DS4USBHost(bool accept_any_device)
    : _accept_any_device(accept_any_device) {}

bool DS4USBHost::registerWithHost() {
    // Custom USBHostHIDDevice handlers register directly with the dispatcher.
    return USBHostHID.addDevice(this);
}

bool DS4USBHost::begin() {
    if (!registerWithHost()) {
        return false;
    }

    return USBHost.begin();
}

void DS4USBHost::update() {
    // USBHost.task() owns dispatching globally. Kept as a no-op so an
    // application can use the same "begin/update" style for this object.
}

bool DS4USBHost::claim(uint8_t dev_addr,
                       uint8_t idx,
                       uint8_t protocol,
                       const uint8_t* report_desc,
                       uint16_t desc_len) {
    (void)protocol;
    (void)report_desc;
    (void)desc_len;

    uint16_t vid = 0;
    uint16_t pid = 0;
    if (!tuh_vid_pid_get(dev_addr, &vid, &pid)) {
        return false;
    }

    const bool is_ds4 =
        vid == SONY_VID &&
        (pid == DS4_PID_V1 || pid == DS4_PID_V2);

    if (!_accept_any_device && !is_ds4) {
        return false;
    }

    // A custom handler should only claim the first HID interface of the DS4.
    // This prevents accidental claims if a future device exposes extra HID
    // interfaces.
    if (idx != 0) {
        return false;
    }

    _dev_addr = dev_addr;
    _idx = idx;
    _connected = true;
    _state = DS4InputState{};
    return true;
}

void DS4USBHost::onUnmount(uint8_t dev_addr, uint8_t idx) {
    if (dev_addr == _dev_addr && idx == _idx) {
        _connected = false;
        _dev_addr = 0;
        _idx = 0;
    }
}

void DS4USBHost::onReport(uint8_t dev_addr,
                          uint8_t idx,
                          const uint8_t* report,
                          uint16_t len) {
    if (!_connected || dev_addr != _dev_addr || idx != _idx) {
        return;
    }

    DS4InputState next = _state;
    if (!_parser.parse(report, len, next)) {
        return;
    }

    _state = next;

    if (_callback != nullptr) {
        _callback(_state, _callback_arg);
    }
}
