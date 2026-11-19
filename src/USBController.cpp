#include "USBController.h"

#include <tusb.h>

USBController::USBController()
    : _parser(&_default_parser) {
    _mutex = xSemaphoreCreateMutex();
}

USBController::USBController(ControllerParser& parser)
    : _parser(&parser) {
    _mutex = xSemaphoreCreateMutex();
}

USBController::~USBController() {
    if (_mutex != nullptr) {
        vSemaphoreDelete(_mutex);
    }
}

bool USBController::registerWithHost() {
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    if (_registered) {
        xSemaphoreGive(_mutex);
        return true;
    }
    _registered = true;
    xSemaphoreGive(_mutex);

    if (!USBHostHID.addDevice(this)) {
        if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
            _registered = false;
            xSemaphoreGive(_mutex);
        }
        return false;
    }
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
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    const bool connected = _connected;
    xSemaphoreGive(_mutex);
    return connected;
}

bool USBController::stateChanged() const {
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    const bool changed = _state_changed;
    xSemaphoreGive(_mutex);
    return changed;
}

void USBController::clearStateChanged() {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    _state_changed = false;
    xSemaphoreGive(_mutex);
}

ControllerInputState USBController::state() const {
    ControllerInputState snapshot{};
    if (_mutex == nullptr) return snapshot;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return snapshot;
    snapshot = _state;
    xSemaphoreGive(_mutex);
    return snapshot;
}

bool USBController::takeState(ControllerInputState& out) {
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    if (!_state_changed) {
        xSemaphoreGive(_mutex);
        return false;
    }
    out = _state;
    _state_changed = false;
    xSemaphoreGive(_mutex);
    return true;
}

bool USBController::connectionChanged() const {
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    const bool changed = _connection_changed;
    xSemaphoreGive(_mutex);
    return changed;
}

void USBController::clearConnectionChanged() {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    _connection_changed = false;
    xSemaphoreGive(_mutex);
}

bool USBController::takeConnectionState(bool& connected) {
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    if (!_connection_changed) {
        xSemaphoreGive(_mutex);
        return false;
    }
    connected = _connected;
    _connection_changed = false;
    xSemaphoreGive(_mutex);
    return true;
}

void USBController::setCallback(ReportCallback callback, void* arg) {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    _callback = callback;
    _callback_arg = arg;
    xSemaphoreGive(_mutex);
}

void USBController::setDisconnectCallback(DisconnectCallback callback, void* arg) {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    _disconnect_callback = callback;
    _disconnect_callback_arg = arg;
    xSemaphoreGive(_mutex);
}

bool USBController::claim(
    uint8_t dev_addr,
    uint8_t idx,
    uint8_t protocol,
    const uint8_t* report_desc,
    uint16_t desc_len
) {
    if (_mutex == nullptr) return false;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    if (_connected || _claiming) {
        xSemaphoreGive(_mutex);
        return false;
    }
    _claiming = true;
    xSemaphoreGive(_mutex);

    uint16_t vendor_id = 0;
    uint16_t product_id = 0;
    if (!tuh_vid_pid_get(dev_addr, &vendor_id, &product_id)) {
        if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
            _claiming = false;
            xSemaphoreGive(_mutex);
        }
        return false;
    }

    if (_parser == &_default_parser) {
        if (_default_parser.matches(
                vendor_id, product_id, protocol, report_desc, desc_len)) {
            _parser = &_default_parser;
        } else if (_dualsense_parser.matches(
                       vendor_id, product_id, protocol, report_desc, desc_len)) {
            _parser = &_dualsense_parser;
        } else {
            if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
                _claiming = false;
                xSemaphoreGive(_mutex);
            }
            return false;
        }
    } else if (!_parser->matches(
                   vendor_id, product_id, protocol, report_desc, desc_len)) {
        if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
            _claiming = false;
            xSemaphoreGive(_mutex);
        }
        return false;
    }

    if (!bindHidInterface(dev_addr, idx)) {
        if (xSemaphoreTake(_mutex, portMAX_DELAY) == pdTRUE) {
            _claiming = false;
            xSemaphoreGive(_mutex);
        }
        return false;
    }

    if (_mutex == nullptr) return true;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return false;
    _connected = true;
    _state_changed = false;
    _connection_changed = true;
    _callback_pending = false;
    _state = ControllerInputState{};
    _claiming = false;
    xSemaphoreGive(_mutex);
    return true;
}

void USBController::onUnmount(uint8_t dev_addr, uint8_t idx) {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    if (dev_addr != _dev_addr || idx != _idx) {
        xSemaphoreGive(_mutex);
        return;
    }

    _connected = false;
    _state = ControllerInputState{};
    _state_changed = false;
    _connection_changed = true;
    _callback_pending = false;
    _disconnect_pending = _disconnect_callback != nullptr;
    if (_parser == &_dualsense_parser) {
        _parser = &_default_parser;
    }
    xSemaphoreGive(_mutex);
    clearHidInterfaceBinding();
}

void USBController::onReport(
    uint8_t dev_addr,
    uint8_t idx,
    const uint8_t* report,
    uint16_t len
) {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    if (!_connected || dev_addr != _dev_addr || idx != _idx) {
        xSemaphoreGive(_mutex);
        return;
    }
    xSemaphoreGive(_mutex);

    ControllerInputState next{};
    if (!_parser->parse(report, len, next)) return;

    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    if (!_connected || dev_addr != _dev_addr || idx != _idx) {
        xSemaphoreGive(_mutex);
        return;
    }
    const bool meaningful_change = _state.hasMeaningfulChange(next);
    _state = next;
    if (meaningful_change) {
        _state_changed = true;
        _callback_pending = _callback != nullptr;
    }
    xSemaphoreGive(_mutex);
}

void USBController::dispatchReportCallback() {
    if (_mutex == nullptr) return;
    if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    if (!_callback_pending || _callback == nullptr) {
        _callback_pending = false;
    } else {
        _callback_pending = false;
        ControllerInputState snapshot = _state;
        ReportCallback callback = _callback;
        void* callback_arg = _callback_arg;
        xSemaphoreGive(_mutex);
        callback(snapshot, callback_arg);
        if (xSemaphoreTake(_mutex, portMAX_DELAY) != pdTRUE) return;
    }

    if (_disconnect_pending) {
        _disconnect_pending = false;
        DisconnectCallback disconnect_callback = _disconnect_callback;
        void* disconnect_callback_arg = _disconnect_callback_arg;
        xSemaphoreGive(_mutex);
        if (disconnect_callback != nullptr) {
            disconnect_callback(disconnect_callback_arg);
        }
        return;
    }
    xSemaphoreGive(_mutex);
}
