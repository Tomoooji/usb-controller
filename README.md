# USB Controller Input

ESP32-S3 の USB Host ポートに接続したゲームコントローラーから入力を取得する
Arduino ライブラリです。現在は DualShock 4 の USB HID input report に対応しています。

## 対応環境

- ESP32-S2 / ESP32-S3 / ESP32-P4
- Arduino-ESP32 の USB Host 対応版
- DualShock 4 (VID `0x054C`, PID `0x05C4` または `0x09CC`)
- USB OTG host 側の VBUS 5 V 供給

## Arduino IDE

このリポジトリを ZIP としてダウンロードし、Arduino IDE の
「スケッチ」→「ライブラリをインクルード」→「.ZIP形式のライブラリをインストール」
からインストールしてください。

USB Host 対応の ESP32 ボードを選択し、USB CDC On Boot はボードの配線に合わせて設定します。

## PlatformIO

```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
lib_deps =
    https://github.com/Tomoooji/usb-controller.git
```

featureブランチを直接ビルドする場合は、`lib_deps` にブランチ名を指定します。

```ini
lib_deps =
    https://github.com/Tomoooji/usb-controller.git#feature/wired-controller-input
```

USB Host ポートを使えるよう、ボード側で OTG と VBUS を設定してください。

## 使用例

```cpp
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
            "LX=%3u LY=%3u RX=%3u RY=%3u L2=%3u R2=%3u "
            "buttons=%c%c%c%c dpad=%u\n",
            state.left_x, state.left_y,
            state.right_x, state.right_y,
            state.l2, state.r2,
            state.cross ? 'X' : '-',
            state.circle ? 'O' : '-',
            state.square ? 'S' : '-',
            state.triangle ? 'T' : '-',
            state.dpad
        );
    }
}
```

`update()` は `USBHost.task()` を呼び出すため、毎回 `loop()` から呼んでください。
登録した `USBController` インスタンスは、USB Hostが稼働する間ずっと生存させてください。
`setup()` 内のローカル変数など、寿命が短いオブジェクトは使わないでください。
入力コールバックを使う場合は `controller.setCallback()` を利用できます。
切断通知を受け取る場合は `controller.setDisconnectCallback()` を利用できます。

## 拡張

USB Host の受信処理とコントローラー固有のreport解析を分離しています。
他のコントローラーを追加する場合は `ControllerParser` を実装し、
`USBController` のコンストラクターへ渡してください。

`ControllerInputState` は複数のコントローラーで共通利用しやすい基本入力を表します。
DualShock 4用の互換名として `DS4InputState` も利用できます。

## 注意

ESP-NOWなどの通信機能はこのライブラリの入力取得機能とは分離し、現時点では含めていません。
