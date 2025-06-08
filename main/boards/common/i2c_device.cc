/**
 * @file i2c_device.cc
 * @brief I2Cデバイス基底クラスの実装
 * 
 * ESP-IDFのI2Cマスタードライバを使用したI2Cデバイス通信の基底クラスを実装します。
 * レジスタの読み書きインターフェースを提供し、PMIC、オーディオコーデック、
 * センサーなどのI2Cデバイス制御クラスで継承して使用されます。
 */

#include "i2c_device.h"

#include <esp_log.h>

#define TAG "I2cDevice"


/**
 * @brief I2cDeviceクラスのコンストラクタ
 * @param i2c_bus I2Cマスターバスハンドル
 * @param addr デバイスのI2Cアドレス（7ビット）
 * 
 * I2Cデバイスをマスターバスに追加し、通信インターフェースを初期化します。
 * 標準的な設定（400kHz、7ビットアドレス、ACKチェック有効）で動作します。
 * 初期化が失敗した場合はアサーションエラーで停止します。
 */
I2cDevice::I2cDevice(i2c_master_bus_handle_t i2c_bus, uint8_t addr) {
    // I2Cデバイス設定構造体の初期化
    i2c_device_config_t i2c_device_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,  // 7ビットアドレス（標準）
        .device_address = addr,                  // デバイスI2Cアドレス
        .scl_speed_hz = 400 * 1000,             // SCLクロック周波数（400kHz・高速モード）
        .scl_wait_us = 0,                       // SCL待機時間（デフォルト）
        .flags = {
            .disable_ack_check = 0,             // ACKチェック有効（通信エラー検出用）
        },
    };
    
    // デバイスをI2Cマスターバスに追加
    ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &i2c_device_cfg, &i2c_device_));
    assert(i2c_device_ != NULL);  // 初期化失敗時はアサーションエラー
}

/**
 * @brief レジスタに1バイト書き込み
 * @param reg 書き込み先レジスタアドレス
 * @param value 書き込むデータ値
 * 
 * 指定されたレジスタアドレスに1バイトのデータを書き込みます。
 * 多くのI2Cデバイスで使用される標準的な「レジスタアドレス + データ」形式の書き込みです。
 * 通信エラーが発生した場合はESP_ERROR_CHECKによりアプリケーションが停止します。
 */
void I2cDevice::WriteReg(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};  // レジスタアドレス + データ値
    ESP_ERROR_CHECK(i2c_master_transmit(i2c_device_, buffer, 2, 100));  // 100msタイムアウト
}

/**
 * @brief レジスタから1バイト読み取り
 * @param reg 読み取り元レジスタアドレス
 * @return uint8_t 読み取ったデータ値
 * 
 * 指定されたレジスタアドレスから1バイトのデータを読み取ります。
 * 書き込み-読み取り組み合わせ転送を使用して、レジスタアドレス送信後にデータを受信します。
 * 通信エラーが発生した場合はESP_ERROR_CHECKによりアプリケーションが停止します。
 */
uint8_t I2cDevice::ReadReg(uint8_t reg) {
    uint8_t buffer[1];
    // レジスタアドレス送信 → データ受信（100msタイムアウト）
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_device_, &reg, 1, buffer, 1, 100));
    return buffer[0];
}

/**
 * @brief レジスタから複数バイト読み取り
 * @param reg 読み取り開始レジスタアドレス
 * @param buffer 読み取ったデータを格納するバッファ
 * @param length 読み取るバイト数
 * 
 * 指定されたレジスタアドレスから連続する複数バイトのデータを読み取ります。
 * 多くのI2Cデバイスではレジスタアドレスが自動インクリメントされ、
 * 連続したレジスタ値を効率的に読み取ることができます。
 * 通信エラーが発生した場合はESP_ERROR_CHECKによりアプリケーションが停止します。
 */
void I2cDevice::ReadRegs(uint8_t reg, uint8_t* buffer, size_t length) {
    // レジスタアドレス送信 → 複数バイトデータ受信（100msタイムアウト）
    ESP_ERROR_CHECK(i2c_master_transmit_receive(i2c_device_, &reg, 1, buffer, length, 100));
}