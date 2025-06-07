/**
 * @file es8388_audio_codec.h
 * @brief ES8388オーディオコーデックドライバー
 * 
 * Everest Semiconductor社ES8388チップ用のオーディオコーデックドライバーです。
 * ES8311/ES8374と同様のI2Cコントロール、I2Sデータ伝送インターフェースを持ち、
 * 全二重通信モードで音声の入出力を同時に行えます。
 */
#ifndef _ES8388_AUDIO_CODEC_H
#define _ES8388_AUDIO_CODEC_H

#include "audio_codec.h"

#include <driver/i2c_master.h>
#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>

/**
 * @class Es8388AudioCodec
 * @brief ES8388チップ用オーディオコーデックドライバー
 * 
 * Everest ES8388オーディオコーデックチップの制御を行います。
 * 高品質なオーディオ処理に対応し、I2Cでコントロール、
 * I2Sでデータ伝送を行います。
 */
class Es8388AudioCodec : public AudioCodec {
private:
    // ESP-ADFコーデックインターフェース
    const audio_codec_data_if_t* data_if_ = nullptr;   /**< データインターフェース */
    const audio_codec_ctrl_if_t* ctrl_if_ = nullptr;   /**< 制御インターフェース */
    const audio_codec_if_t* codec_if_ = nullptr;       /**< コーデックインターフェース */
    const audio_codec_gpio_if_t* gpio_if_ = nullptr;   /**< GPIOインターフェース */

    // デバイスハンドル
    esp_codec_dev_handle_t output_dev_ = nullptr;      /**< 出力デバイスハンドル */
    esp_codec_dev_handle_t input_dev_ = nullptr;       /**< 入力デバイスハンドル */
    gpio_num_t pa_pin_ = GPIO_NUM_NC;                  /**< パワーアンプ制御PIN */

    /** 全二重通信用I2Sチャンネルを作成 */
    void CreateDuplexChannels(gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);

    /** オーディオデータを読み取り（マイク入力） */
    virtual int Read(int16_t* dest, int samples) override;
    
    /** オーディオデータを書き込み（スピーカー出力） */
    virtual int Write(const int16_t* data, int samples) override;

public:
    /**
     * @brief ES8388オーディオコーデックコンストラクタ
     * @param i2c_master_handle I2Cマスターハンドル
     * @param i2c_port I2Cポート番号
     * @param input_sample_rate 入力サンプリングレート
     * @param output_sample_rate 出力サンプリングレート
     * @param mclk マスタークロックGPIOピン
     * @param bclk ビットクロックGPIOピン
     * @param ws ワードセレクトGPIOピン
     * @param dout データ出力GPIOピン
     * @param din データ入力GPIOピン
     * @param pa_pin パワーアンプ制御ピン
     * @param es8388_addr ES8388のI2Cアドレス
     */
    Es8388AudioCodec(void* i2c_master_handle, i2c_port_t i2c_port, int input_sample_rate, int output_sample_rate,
        gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din,
        gpio_num_t pa_pin, uint8_t es8388_addr);
    
    virtual ~Es8388AudioCodec();

    /** 出力ボリュームを設定 */
    virtual void SetOutputVolume(int volume) override;
    
    /** 音声入力を有効/無効化 */
    virtual void EnableInput(bool enable) override;
    
    /** 音声出力を有効/無効化 */
    virtual void EnableOutput(bool enable) override;
};

#endif // _ES8388_AUDIO_CODEC_H
