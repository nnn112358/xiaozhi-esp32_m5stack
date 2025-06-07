/**
 * @file box_audio_codec.h
 * @brief ESP-BOXシリーズ用組み合わせオーディオコーデック
 * 
 * ESP-BOXデバイス用の特別なオーディオコーデックドライバーです。
 * ES8311（出力用）とES7210（入力用）の組み合わせで、
 * 高品質なステレオ出力と高精度なマイク入力を実現します。
 */
#ifndef _BOX_AUDIO_CODEC_H
#define _BOX_AUDIO_CODEC_H

#include "audio_codec.h"

#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>

/**
 * @class BoxAudioCodec
 * @brief ESP-BOX用組み合わせオーディオコーデック
 * 
 * ESP-BOXデバイス用の特別なオーディオシステムを実装します。
 * ES8311チップでステレオ出力、ES7210チップでマルチチャンネル入力を行い、
 * エコーキャンセレーション用のリファレンス信号も提供します。
 */
class BoxAudioCodec : public AudioCodec {
private:
    // ESP-ADFコーデックインターフェース
    const audio_codec_data_if_t* data_if_ = nullptr;      /**< データインターフェース */
    const audio_codec_ctrl_if_t* out_ctrl_if_ = nullptr;  /**< 出力制御インターフェース */
    const audio_codec_if_t* out_codec_if_ = nullptr;      /**< 出力コーデックインターフェース */
    const audio_codec_ctrl_if_t* in_ctrl_if_ = nullptr;   /**< 入力制御インターフェース */
    const audio_codec_if_t* in_codec_if_ = nullptr;       /**< 入力コーデックインターフェース */
    const audio_codec_gpio_if_t* gpio_if_ = nullptr;      /**< GPIOインターフェース */

    // デバイスハンドル
    esp_codec_dev_handle_t output_dev_ = nullptr;         /**< 出力デバイスハンドル（ES8311） */
    esp_codec_dev_handle_t input_dev_ = nullptr;          /**< 入力デバイスハンドル（ES7210） */

    /** 全二重通信用I2Sチャンネルを作成 */
    void CreateDuplexChannels(gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);

    /** オーディオデータを読み取り（マイク入力） */
    virtual int Read(int16_t* dest, int samples) override;
    
    /** オーディオデータを書き込み（スピーカー出力） */
    virtual int Write(const int16_t* data, int samples) override;

public:
    /**
     * @brief ESP-BOXオーディオコーデックコンストラクタ
     * @param i2c_master_handle I2Cマスターハンドル
     * @param input_sample_rate 入力サンプリングレート
     * @param output_sample_rate 出力サンプリングレート
     * @param mclk マスタークロックGPIOピン
     * @param bclk ビットクロックGPIOピン
     * @param ws ワードセレクトGPIOピン
     * @param dout データ出力GPIOピン
     * @param din データ入力GPIOピン
     * @param pa_pin パワーアンプ制御ピン
     * @param es8311_addr ES8311のI2Cアドレス
     * @param es7210_addr ES7210のI2Cアドレス
     * @param input_reference リファレンス信号を使用するかどうか
     */
    BoxAudioCodec(void* i2c_master_handle, int input_sample_rate, int output_sample_rate,
        gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din,
        gpio_num_t pa_pin, uint8_t es8311_addr, uint8_t es7210_addr, bool input_reference);
    
    virtual ~BoxAudioCodec();

    /** 出力ボリュームを設定 */
    virtual void SetOutputVolume(int volume) override;
    
    /** 音声入力を有効/無効化 */
    virtual void EnableInput(bool enable) override;
    
    /** 音声出力を有効/無効化 */
    virtual void EnableOutput(bool enable) override;
};

#endif // _BOX_AUDIO_CODEC_H
