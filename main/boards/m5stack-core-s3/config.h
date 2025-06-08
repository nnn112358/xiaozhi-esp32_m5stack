/**
 * @file config.h
 * @brief M5Stack CoreS3 ボード設定ファイル
 * 
 * M5Stack CoreS3 開発ボード用のハードウェア設定を定義します。
 * オーディオ、ディスプレイ、カメラ、ボタンなどのピン配置と設定値を含みます。
 * 
 * @note このボードは以下の特徴があります：
 * - ESP32-S3 チップ搭載
 * - 内蔵 320x240 LCDディスプレイ
 * - AW88298 アンプ + ES7210 マイクロフォンアレイ
 * - カメラモジュール対応
 * - 充電機能付きバッテリー
 */
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ================================================================
// オーディオ関連設定
// ================================================================

/** @brief オーディオ入力リファレンス使用フラグ */
#define AUDIO_INPUT_REFERENCE    true

/** @brief オーディオ入力サンプリングレート（Hz） */
#define AUDIO_INPUT_SAMPLE_RATE  24000

/** @brief オーディオ出力サンプリングレート（Hz） */
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

// I2S オーディオインターフェースピン設定
/** @brief I2S マスタークロック（MCLK）ピン */
#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_0

/** @brief I2S ワードセレクト（WS/LRCK）ピン */
#define AUDIO_I2S_GPIO_WS GPIO_NUM_33

/** @brief I2S ビットクロック（BCLK）ピン */
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_34

/** @brief I2S データ入力（マイク→ESP32）ピン */
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_14

/** @brief I2S データ出力（ESP32→スピーカー）ピン */
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_13

// オーディオコーデック I2C 通信設定
/** @brief オーディオコーデック I2C データ（SDA）ピン */
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_12

/** @brief オーディオコーデック I2C クロック（SCL）ピン */
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_11

/** @brief AW88298 アンプの I2C アドレス */
#define AUDIO_CODEC_AW88298_ADDR AW88298_CODEC_DEFAULT_ADDR

/** @brief ES7210 マイクロフォンアレイの I2C アドレス */
#define AUDIO_CODEC_ES7210_ADDR  ES7210_CODEC_DEFAULT_ADDR

// ================================================================
// ボタン・LED関連設定
// ================================================================

/** @brief 内蔵LED GPIO（CoreS3では未使用） */
#define BUILTIN_LED_GPIO        GPIO_NUM_NC

/** @brief ブートボタン（リセットボタン）GPIO */
#define BOOT_BUTTON_GPIO        GPIO_NUM_0

/** @brief 音量アップボタンGPIO（CoreS3では未使用） */
#define VOLUME_UP_BUTTON_GPIO   GPIO_NUM_NC

/** @brief 音量ダウンボタンGPIO（CoreS3では未使用） */
#define VOLUME_DOWN_BUTTON_GPIO GPIO_NUM_NC

// ================================================================
// ディスプレイ関連設定
// ================================================================

/** @brief ディスプレイ I2C データピン（CoreS3では内蔵SPI使用のため未使用） */
#define DISPLAY_SDA_PIN GPIO_NUM_NC

/** @brief ディスプレイ I2C クロックピン（CoreS3では内蔵SPI使用のため未使用） */
#define DISPLAY_SCL_PIN GPIO_NUM_NC

/** @brief ディスプレイ幅（ピクセル） */
#define DISPLAY_WIDTH   320

/** @brief ディスプレイ高さ（ピクセル） */
#define DISPLAY_HEIGHT  240

/** @brief 水平ミラー設定（false: 通常表示） */
#define DISPLAY_MIRROR_X false

/** @brief 垂直ミラー設定（false: 通常表示） */
#define DISPLAY_MIRROR_Y false

/** @brief XY軸入れ替え設定（false: 通常方向） */
#define DISPLAY_SWAP_XY false

/** @brief ディスプレイX方向オフセット（ピクセル） */
#define DISPLAY_OFFSET_X  0

/** @brief ディスプレイY方向オフセット（ピクセル） */
#define DISPLAY_OFFSET_Y  0

/** @brief バックライト制御ピン（CoreS3では内蔵制御のため未使用） */
#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_NC

/** @brief バックライト出力反転フラグ */
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT true

// ================================================================
// カメラ関連設定
// ================================================================

/** @brief カメラ電源ダウンピン（未使用） */
#define CAMERA_PIN_PWDN GPIO_NUM_NC

/** @brief カメラリセットピン（未使用） */
#define CAMERA_PIN_RESET GPIO_NUM_NC

/** @brief カメラ外部クロック（XCLK）ピン（20MHz外部発振器使用のため未使用） */
#define CAMERA_PIN_XCLK  GPIO_NUM_NC

/** @brief カメラ I2C データ（SIOD）ピン（既存I2Cポート使用のため未使用） */
#define CAMERA_PIN_SIOD GPIO_NUM_NC

/** @brief カメラ I2C クロック（SIOC）ピン（既存I2Cポート使用のため未使用） */
#define CAMERA_PIN_SIOC GPIO_NUM_NC

// カメラデータバス（D0-D7）
/** @brief カメラデータビット0 */
#define CAMERA_PIN_D0 GPIO_NUM_39
/** @brief カメラデータビット1 */
#define CAMERA_PIN_D1 GPIO_NUM_40
/** @brief カメラデータビット2 */
#define CAMERA_PIN_D2 GPIO_NUM_41
/** @brief カメラデータビット3 */
#define CAMERA_PIN_D3 GPIO_NUM_42
/** @brief カメラデータビット4 */
#define CAMERA_PIN_D4 GPIO_NUM_15
/** @brief カメラデータビット5 */
#define CAMERA_PIN_D5 GPIO_NUM_16
/** @brief カメラデータビット6 */
#define CAMERA_PIN_D6 GPIO_NUM_48
/** @brief カメラデータビット7 */
#define CAMERA_PIN_D7 GPIO_NUM_47

/** @brief カメラ垂直同期（VSYNC）ピン */
#define CAMERA_PIN_VSYNC GPIO_NUM_46

/** @brief カメラ水平リファレンス（HREF）ピン */
#define CAMERA_PIN_HREF GPIO_NUM_38

/** @brief カメラピクセルクロック（PCLK）ピン */
#define CAMERA_PIN_PCLK GPIO_NUM_45

/** @brief カメラ外部クロック周波数（Hz） - 20MHz外部発振器 */
#define XCLK_FREQ_HZ 20000000

#endif // _BOARD_CONFIG_H_
