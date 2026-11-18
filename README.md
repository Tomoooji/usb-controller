# DS4ESPNow

ESP32-S3 の USB Host で Sony DualShock 4 (DS4) を読み取り、
入力状態を ESP-NOW で指定した MAC アドレスへ転送する Arduino-ESP32 用ライブラリ。

## 構成

```text
DualShock 4
    │ USB HID
    ▼
DS4USBHost
    │ 64-byte USB report
    ▼
DS4Parser
    │
    ▼
DS4InputState
    │
    ├── application can read directly
    │
    ▼
ESPNowTransport
    │ ESP-NOW
    ▼
specified peer MAC
```

USB Host と ESP-NOW の処理を分離しているため、後から通信方式を変更しやすい構成。

## 必要環境

- ESP32-S3
- Arduino-ESP32 の USB Host 対応版
- DualShock 4 を USB 接続
- USB OTG host 側で VBUS 5 V を供給できるハードウェア

Arduino-ESP32 の USB Host API は ESP32-S2/S3/P4 の OTG peripheral を対象としている。
S3 では USB-OTG を使う場合、USB CDC On Boot の設定にも注意が必要。

## 注意

DS4 の USB input report は Report ID `0x01`、64 bytes。
このライブラリは DS4 の USB report を直接パースする。

対応 PID:

- VID `0x054C`
- PID `0x05C4` (DS4 v1)
- PID `0x09CC` (DS4 v2)

`accept_any_usb_hid = true` にすると VID/PID のチェックだけを外せる。

## 基本使用例

```cpp
#include <DS4ESPNow.h>

DS4ESPNow::Config config = {
    .esp_now = {
        .peer = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56},
        .channel = 1,
        .encrypt = false,
    },
};

DS4ESPNow ds4(config);

void setup() {
    Serial.begin(115200);
    ds4.begin();
}

void loop() {
    ds4.update();

    static uint8_t last_sequence = 0xFF;

    if (ds4.connected() && ds4.state().sequence != last_sequence) {
        last_sequence = ds4.state().sequence;
        ds4.sendCurrentState();
    }
}
```

## ESP-NOW payload

現在は `DS4InputState` をそのまま送信している。

これは「まず動かす」ための実装で、今後の通信プロトコルとしては以下を検討できる。

- protocol version
- packet type
- sequence number
- timestamp
- CRC
- packed struct
- receiver acknowledgement

特に ESP-NOW の受信側を既存プロジェクトへ組み込む場合は、
`DS4InputState` をそのまま外部通信仕様に固定せず、通信packet用structを別にするのがおすすめ。

## USB Host の注意

`DS4USBHost::onReport()` は USB Host 側の report callback から呼ばれるため、
重い処理を入れないこと。

標準構成では `DS4ESPNow::update()` が `USBHost.task()` を呼ぶ。

また、USB Host ポートには VBUS 5 V の供給が必要。
S3 の開発ボードによっては USB-A コネクタを直接ホストとして使えないため、
OTG配線・VBUS制御はボード側で確認すること。

## 今後の候補

- DS4 touchpad
- gyro / accelerometer
- battery status
- light bar / rumble output
- DS4の複数PID/派生モデル対応
- ESP-NOW packet protocol の独立化
- 送信周期制御
- send callback / success tracking
- 複数peer
- 暗号化設定


## 実装方針

`DS4USBHost` は `USBHostHIDDevice` のカスタムハンドラとして実装している。
そのため、標準 `USBHostHIDGamepad` の「汎用ゲームパッド解釈」に依存しない。

アプリケーションが USB Host のライフサイクルを管理したい場合は:

```cpp
DS4USBHost ds4;

void setup() {
    ds4.registerWithHost();
    USBHost.begin();
}

void loop() {
    USBHost.task();
}
```

通常は単に:

```cpp
DS4USBHost ds4;

void setup() {
    ds4.begin();
}

void loop() {
    USBHost.task();
}
```

でよい。

`DS4ESPNow` は後者の構成を内部で使用する。
