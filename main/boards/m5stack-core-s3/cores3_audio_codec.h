/**
 * @file cores3_audio_codec.h
 * @brief M5Stack CoreS3 専用オーディオコーデッククラス
 * 
 * このファイルはM5Stack CoreS3ボード専用のオーディオコーデック制御機能を提供します。
 * AW88298アンプとES7210マイクロフォンアレイを使用したデュアルコーデック構成に対応します。
 */
#ifndef _BOX_AUDIO_CODEC_H
#define _BOX_AUDIO_CODEC_H

#include "audio_codec.h"

#include <esp_codec_dev.h>
#include <esp_codec_dev_defaults.h>

/**
 * @class CoreS3AudioCodec
 * @brief M5Stack CoreS3専用オーディオコーデッククラス
 * 
 * M5Stack CoreS3ボードに搭載されたAW88298アンプ（出力用）とES7210マイクロフォンアレイ（入力用）を
 * 制御するデュアルコーデック構成のオーディオシステムです。高品質な音声入出力を提供します。
 */
class CoreS3AudioCodec : public AudioCodec {
private:
    /** @brief I2Sデータインターフェース */
    const audio_codec_data_if_t* data_if_ = nullptr;
    
    /** @brief 出力コーデック制御インターフェース（AW88298用） */
    const audio_codec_ctrl_if_t* out_ctrl_if_ = nullptr;
    
    /** @brief 出力コーデックインターフェース（AW88298用） */
    const audio_codec_if_t* out_codec_if_ = nullptr;
    
    /** @brief 入力コーデック制御インターフェース（ES7210用） */
    const audio_codec_ctrl_if_t* in_ctrl_if_ = nullptr;
    
    /** @brief 入力コーデックインターフェース（ES7210用） */
    const audio_codec_if_t* in_codec_if_ = nullptr;
    
    /** @brief GPIO制御インターフェース */
    const audio_codec_gpio_if_t* gpio_if_ = nullptr;

    /** @brief 出力デバイスハンドル（AW88298アンプ） */
    esp_codec_dev_handle_t output_dev_ = nullptr;
    
    /** @brief 入力デバイスハンドル（ES7210マイクアレイ） */
    esp_codec_dev_handle_t input_dev_ = nullptr;

    /**
     * @brief デュプレックスチャンネルの作成
     * @param mclk マスタークロックGPIOピン
     * @param bclk ビットクロックGPIOピン
     * @param ws ワードセレクトGPIOピン
     * @param dout データ出力GPIOピン（ESP32→アンプ）
     * @param din データ入力GPIOピン（マイク→ESP32）
     * 
     * 入力と出力で同じI2Sバスを共有するデュプレックス通信チャンネルを設定します。
     */
    void CreateDuplexChannels(gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);

    /**
     * @brief オーディオデータの読み込み（マイクからの入力）
     * @param dest 読み込み先バッファ
     * @param samples 読み込むサンプル数
     * @return int 実際に読み込んだサンプル数
     */
    virtual int Read(int16_t* dest, int samples) override;
    
    /**
     * @brief オーディオデータの書き込み（スピーカーへの出力）
     * @param data 書き込むデータバッファ
     * @param samples 書き込むサンプル数
     * @return int 実際に書き込んだサンプル数
     */
    virtual int Write(const int16_t* data, int samples) override;

public:
    /**
     * @brief CoreS3AudioCodecのコンストラクタ
     * @param i2c_master_handle I2Cマスターハンドル（コーデック制御用）
     * @param input_sample_rate 入力サンプリングレート（Hz）
     * @param output_sample_rate 出力サンプリングレート（Hz）
     * @param mclk マスタークロックGPIOピン
     * @param bclk ビットクロックGPIOピン
     * @param ws ワードセレクトGPIOピン
     * @param dout データ出力GPIOピン
     * @param din データ入力GPIOピン
     * @param aw88298_addr AW88298アンプのI2Cアドレス
     * @param es7210_addr ES7210マイクアレイのI2Cアドレス
     * @param input_reference 入力リファレンス使用フラグ
     * 
     * デュアルコーデック構成のオーディオシステムを初期化します。
     * AW88298とES7210の両方を設定し、デュプレックス通信を確立します。
     */
    CoreS3AudioCodec(void* i2c_master_handle, int input_sample_rate, int output_sample_rate,
        gpio_num_t mclk, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din,
        uint8_t aw88298_addr, uint8_t es7210_addr, bool input_reference);
    
    /**
     * @brief デストラクタ
     * 
     * コーデックデバイスを停止し、リソースを解放します。
     */
    virtual ~CoreS3AudioCodec();

    /**
     * @brief 出力音量の設定
     * @param volume 音量レベル（0-100）
     * 
     * AW88298アンプの出力音量を制御します。
     */
    virtual void SetOutputVolume(int volume) override;
    
    /**
     * @brief 音声入力の有効/無効切り替え
     * @param enable true: 入力有効, false: 入力無効
     * 
     * ES7210マイクロフォンアレイからの音声入力を制御します。
     */
    virtual void EnableInput(bool enable) override;
    
    /**
     * @brief 音声出力の有効/無効切り替え
     * @param enable true: 出力有効, false: 出力無効
     * 
     * AW88298アンプからの音声出力を制御します。
     */
    virtual void EnableOutput(bool enable) override;
};

#endif // _BOX_AUDIO_CODEC_H
