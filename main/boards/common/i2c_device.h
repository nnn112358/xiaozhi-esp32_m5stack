/**
 * @file i2c_device.h
 * @brief I2C デバイス制御の基底クラス
 * 
 * このファイルはI2C通信を使用するデバイス（センサー、コーデック等）の
 * 共通機能を提供します。レジスタの読み書きやデバイス初期化を含みます。
 */
#ifndef I2C_DEVICE_H
#define I2C_DEVICE_H

#include <driver/i2c_master.h>

/**
 * @class I2cDevice
 * @brief I2C デバイス制御の基底クラス
 * 
 * I2C バス上のデバイス（オーディオコーデック、センサー、電源管理IC等）を
 * 制御するための共通機能を提供します。レジスタアクセス機能を含みます。
 */
class I2cDevice {
public:
    /**
     * @brief I2C デバイスのコンストラクタ
     * @param i2c_bus I2C マスターバスのハンドル
     * @param addr デバイスの7ビットI2Cアドレス
     * 
     * 指定されたI2Cバス上のデバイスとの通信を初期化します。
     * デバイスハンドルを作成し、後続の通信で使用します。
     */
    I2cDevice(i2c_master_bus_handle_t i2c_bus, uint8_t addr);

protected:
    /** @brief I2C デバイスハンドル（ESP-IDF マスターデバイス） */
    i2c_master_dev_handle_t i2c_device_;

    /**
     * @brief レジスタへの書き込み
     * @param reg 書き込み先レジスタアドレス
     * @param value 書き込む8ビット値
     * 
     * 指定されたレジスタアドレスに1バイトのデータを書き込みます。
     * デバイス設定の変更やコマンド送信に使用されます。
     */
    void WriteReg(uint8_t reg, uint8_t value);
    
    /**
     * @brief レジスタからの読み込み
     * @param reg 読み込み元レジスタアドレス
     * @return uint8_t 読み込んだ8ビット値
     * 
     * 指定されたレジスタアドレスから1バイトのデータを読み込みます。
     * デバイス状態の確認や設定値の取得に使用されます。
     */
    uint8_t ReadReg(uint8_t reg);
    
    /**
     * @brief 複数レジスタからの連続読み込み
     * @param reg 読み込み開始レジスタアドレス
     * @param buffer 読み込みデータを格納するバッファ
     * @param length 読み込むバイト数
     * 
     * 指定されたレジスタから連続して複数バイトを読み込みます。
     * センサーデータの一括取得やFIFOバッファの読み込みに使用されます。
     */
    void ReadRegs(uint8_t reg, uint8_t* buffer, size_t length);
};

#endif // I2C_DEVICE_H
