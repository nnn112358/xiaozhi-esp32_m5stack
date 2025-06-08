# カスタム開発ボードガイド

本ガイドでは、XiaoZhi AI音声チャットボットプロジェクト用の新しい開発ボード初期化プログラムをカスタマイズする方法を説明します。XiaoZhi AIは50種類以上のESP32シリーズ開発ボードをサポートしており、各開発ボードの初期化コードは対応するディレクトリに配置されています。

## 重要な注意事項

> **警告**: カスタム開発ボードで、IO設定が既存の開発ボードと異なる場合、既存の開発ボードの設定を直接上書きしてファームウェアをコンパイルしないでください。新しい開発ボードタイプを作成するか、config.jsonファイルのbuilds設定で異なるnameとsdkconfig macro定義を使用して区別する必要があります。`python scripts/release.py [開発ボードディレクトリ名]` を使用してファームウェアをコンパイル・パッケージ化してください。
>
> 既存の設定を直接上書きすると、将来のOTAアップグレード時に、カスタムファームウェアが既存開発ボードの標準ファームウェアで上書きされ、デバイスが正常に動作しなくなる可能性があります。各開発ボードには固有の識別子と対応するファームウェアアップグレードチャネルがあり、開発ボード識別子の固有性を保つことが非常に重要です。

## ディレクトリ構造

各開発ボードのディレクトリ構造には通常以下のファイルが含まれます：

- `xxx_board.cc` - メインのボードレベル初期化コード、ボード関連の初期化と機能を実装
- `config.h` - ボードレベル設定ファイル、ハードウェアピンマッピングとその他の設定項目を定義
- `config.json` - コンパイル設定、ターゲットチップと特別なコンパイルオプションを指定
- `README.md` - 開発ボード関連の説明文書

## カスタム開発ボードの手順

### 1. 新しい開発ボードディレクトリの作成

まず`boards/`ディレクトリ下に新しいディレクトリを作成します（例：`my-custom-board/`）：

```bash
mkdir main/boards/my-custom-board
```

### 2. 設定ファイルの作成

#### config.h

`config.h`ですべてのハードウェア設定を定義します：

- オーディオサンプリングレートとI2Sピン設定
- オーディオコーデックチップアドレスとI2Cピン設定
- ボタンとLEDピン設定
- ディスプレイパラメータとピン設定

参考例（lichuang-c3-devより）：

```c
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// オーディオ設定
#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_10
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_12
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_8
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_7
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_11

#define AUDIO_CODEC_PA_PIN       GPIO_NUM_13
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_0
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_1
#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR

// ボタン設定
#define BOOT_BUTTON_GPIO        GPIO_NUM_9

// ディスプレイ設定
#define DISPLAY_SPI_SCK_PIN     GPIO_NUM_3
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_5
#define DISPLAY_DC_PIN          GPIO_NUM_6
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_4

#define DISPLAY_WIDTH   320
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY true

#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_2
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

#endif // _BOARD_CONFIG_H_
```

#### config.json

`config.json`でコンパイル設定を定義します：

```json
{
    "target": "esp32s3",  // ターゲットチップモデル: esp32, esp32s3, esp32c3など
    "builds": [
        {
            "name": "my-custom-board",  // 開発ボード名
            "sdkconfig_append": [
                // 追加で必要なコンパイル設定
                "CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y",
                "CONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"partitions_8M.csv\""
            ]
        }
    ]
}
```

### 3. ボードレベル初期化コードの記述

`my_custom_board.cc`ファイルを作成し、開発ボードのすべての初期化ロジックを実装します。

基本的な開発ボードクラス定義には以下の部分が含まれます：

1. **クラス定義**：`WifiBoard`または`ML307Board`から継承
2. **初期化関数**：I2C、ディスプレイ、ボタン、IoTなどのコンポーネントの初期化を含む
3. **仮想関数のオーバーライド**：`GetAudioCodec()`、`GetDisplay()`、`GetBacklight()`など
4. **開発ボードの登録**：`DECLARE_BOARD`マクロを使用して開発ボードを登録

```cpp
#include "wifi_board.h"
#include "audio_codecs/es8311_audio_codec.h"
#include "display/lcd_display.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "iot/thing_manager.h"

#include <esp_log.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>

#define TAG "MyCustomBoard"

// フォント宣言
LV_FONT_DECLARE(font_puhui_16_4);
LV_FONT_DECLARE(font_awesome_16_4);

class MyCustomBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t codec_i2c_bus_;
    Button boot_button_;
    LcdDisplay* display_;

    // I2C初期化
    void InitializeI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = I2C_NUM_0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &codec_i2c_bus_));
    }

    // SPI初期化（ディスプレイ用）
    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SPI_MOSI_PIN;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SPI_SCK_PIN;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    // ボタン初期化
    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });
    }

    // ディスプレイ初期化（ST7789の例）
    void InitializeDisplay() {
        esp_lcd_panel_io_handle_t panel_io = nullptr;
        esp_lcd_panel_handle_t panel = nullptr;
        
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_SPI_CS_PIN;
        io_config.dc_gpio_num = DISPLAY_DC_PIN;
        io_config.spi_mode = 2;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &panel_io));

        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = GPIO_NUM_NC;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));
        
        esp_lcd_panel_reset(panel);
        esp_lcd_panel_init(panel);
        esp_lcd_panel_invert_color(panel, true);
        esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
        esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);
        
        // ディスプレイオブジェクトの作成
        display_ = new SpiLcdDisplay(panel_io, panel,
                                    DISPLAY_WIDTH, DISPLAY_HEIGHT, 
                                    DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
                                    DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
                                    {
                                        .text_font = &font_puhui_16_4,
                                        .icon_font = &font_awesome_16_4,
                                        .emoji_font = font_emoji_32_init(),
                                    });
    }

    // IoTデバイス初期化
    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        thing_manager.AddThing(iot::CreateThing("Screen"));
        // 追加のIoTデバイスを追加可能
    }

public:
    // コンストラクタ
    MyCustomBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeI2c();
        InitializeSpi();
        InitializeDisplay();
        InitializeButtons();
        InitializeIot();
        GetBacklight()->SetBrightness(100);
    }

    // オーディオコーデックの取得
    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(
            codec_i2c_bus_, 
            I2C_NUM_0, 
            AUDIO_INPUT_SAMPLE_RATE, 
            AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, 
            AUDIO_I2S_GPIO_BCLK, 
            AUDIO_I2S_GPIO_WS, 
            AUDIO_I2S_GPIO_DOUT, 
            AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, 
            AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }

    // ディスプレイの取得
    virtual Display* GetDisplay() override {
        return display_;
    }
    
    // バックライト制御の取得
    virtual Backlight* GetBacklight() override {
        static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }
};

// 開発ボードの登録
DECLARE_BOARD(MyCustomBoard);
```

### 4. README.mdの作成

README.mdで開発ボードの特徴、ハードウェア要件、コンパイルとフラッシュ手順を説明します：


## 一般的な開発ボードコンポーネント

### 1. ディスプレイ

プロジェクトは複数のディスプレイドライバをサポートしています：
- ST7789 (SPI)
- ILI9341 (SPI)
- SH8601 (QSPI)
- など...

### 2. オーディオコーデック

サポートされているコーデック：
- ES8311 (一般的)
- ES7210 (マイクアレイ)
- AW88298 (アンプ)
- など...

### 3. 電源管理

一部の開発ボードは電源管理チップを使用：
- AXP2101
- その他利用可能なPMIC

### 4. IoTデバイス

さまざまなIoTデバイスを追加でき、AIが「見る」ことと制御を可能にします：
- Speaker (スピーカー)
- Screen (スクリーン)
- Battery (バッテリー)
- Light (ライト)
- など...

## 開発ボードクラス継承関係

- `Board` - 基本ボードレベルクラス
  - `WifiBoard` - WiFi接続の開発ボード
  - `ML307Board` - 4Gモジュールを使用する開発ボード

## 開発のコツ

1. **類似の開発ボードを参照**：新しい開発ボードが既存の開発ボードと類似点がある場合、既存の実装を参照できます
2. **段階的デバッグ**：まず基本機能（ディスプレイなど）を実装してから、より複雑な機能（オーディオなど）を追加します
3. **ピンマッピング**：config.hですべてのピンマッピングを正しく設定することを確認してください
4. **ハードウェア互換性の確認**：すべてのチップとドライバの互換性を確認してください

## 発生する可能性のある問題

1. **ディスプレイが正常でない**：SPI設定、ミラー設定、色反転設定を確認してください
2. **オーディオ出力がない**：I2S設定、PA有効ピン、コーデックアドレスを確認してください
3. **ネットワークに接続できない**：WiFi資格情報とネットワーク設定を確認してください
4. **サーバーと通信できない**：MQTTまたはWebSocket設定を確認してください

## 参考資料

- ESP-IDFドキュメント: https://docs.espressif.com/projects/esp-idf/
- LVGLドキュメント: https://docs.lvgl.io/
- ESP-SRドキュメント: https://github.com/espressif/esp-sr 