/**
 * @file no_audio_codec.h
 * @brief オーディオコーデックチップなしの直接I2S接続ドライバー
 * 
 * 外部オーディオコーデックチップを使用せず、ESP32のI2Sインターフェースを
 * 直接スピーカーやマイクに接続するためのドライバーです。
 * 全二重通信、単一方向通信、PDMマイク対応など、特殊な構成に対応します。
 */
#ifndef _NO_AUDIO_CODEC_H
#define _NO_AUDIO_CODEC_H

#include "audio_codec.h"

#include <driver/gpio.h>
#include <driver/i2s_pdm.h>

/**
 * @class NoAudioCodec
 * @brief オーディオコーデックチップなしの基底クラス
 * 
 * 外部オーディオコーデックチップを使用せず、直接I2Sインターフェースで
 * スピーカーやマイクを制御するための基底クラスです。
 */
class NoAudioCodec : public AudioCodec {
private:
    /** オーディオデータを書き込み（スピーカー出力） */
    virtual int Write(const int16_t* data, int samples) override;
    
    /** オーディオデータを読み取り（マイク入力） */
    virtual int Read(int16_t* dest, int samples) override;

public:
    virtual ~NoAudioCodec();
};

/**
 * @class NoAudioCodecDuplex
 * @brief 全二重通信対応の直接I2S接続コーデック
 * 
 * 同時に音声の入力と出力を行うためのクラスです。
 * スピーカーとマイクが同じI2Sバスを共有します。
 */
class NoAudioCodecDuplex : public NoAudioCodec {
public:
    /**
     * @brief 全二重通信コーデックコンストラクタ
     * @param input_sample_rate 入力サンプリングレート
     * @param output_sample_rate 出力サンプリングレート
     * @param bclk ビットクロックGPIOピン
     * @param ws ワードセレクトGPIOピン
     * @param dout データ出力GPIOピン
     * @param din データ入力GPIOピン
     */
    NoAudioCodecDuplex(int input_sample_rate, int output_sample_rate, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);
};

/**
 * @class ATK_NoAudioCodecDuplex
 * @brief ATKボード用全二重通信コーデック
 * 
 * ATK（Alientek）ボード用に最適化された全二重通信コーデックです。
 */
class ATK_NoAudioCodecDuplex : public NoAudioCodec {
public:
    /**
     * @brief ATKボード用全二重通信コーデックコンストラクタ
     */
    ATK_NoAudioCodecDuplex(int input_sample_rate, int output_sample_rate, gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);
};

/**
 * @class NoAudioCodecSimplex
 * @brief 単方向通信用直接I2S接続コーデック
 * 
 * スピーカーとマイクが別々のI2Sバスを使用する構成用です。
 * 高品質な音声処理が必要な場合に適しています。
 */
class NoAudioCodecSimplex : public NoAudioCodec {
public:
    /**
     * @brief 単方向通信コーデックコンストラクタ（シンプル）
     */
    NoAudioCodecSimplex(int input_sample_rate, int output_sample_rate, gpio_num_t spk_bclk, gpio_num_t spk_ws, gpio_num_t spk_dout, gpio_num_t mic_sck, gpio_num_t mic_ws, gpio_num_t mic_din);
    
    /**
     * @brief 単方向通信コーデックコンストラクタ（スロットマスク指定）
     */
    NoAudioCodecSimplex(int input_sample_rate, int output_sample_rate, gpio_num_t spk_bclk, gpio_num_t spk_ws, gpio_num_t spk_dout, i2s_std_slot_mask_t spk_slot_mask, gpio_num_t mic_sck, gpio_num_t mic_ws, gpio_num_t mic_din, i2s_std_slot_mask_t mic_slot_mask);
};

/**
 * @class NoAudioCodecSimplexPdm
 * @brief PDMマイク対応単方向通信コーデック
 * 
 * I2SスピーカーとPDM（Pulse Density Modulation）マイクを組み合わせた構成です。
 * PDMマイクは高品質な音声入力を提供します。
 */
class NoAudioCodecSimplexPdm : public NoAudioCodec {
public:
    /**
     * @brief PDMマイク対応コーデックコンストラクタ
     * @param input_sample_rate 入力サンプリングレート
     * @param output_sample_rate 出力サンプリングレート
     * @param spk_bclk スピーカー用ビットクロックGPIOピン
     * @param spk_ws スピーカー用ワードセレクトGPIOピン
     * @param spk_dout スピーカー用データ出力GPIOピン
     * @param mic_sck PDMマイク用クロックGPIOピン
     * @param mic_din PDMマイク用データ入力GPIOピン
     */
    NoAudioCodecSimplexPdm(int input_sample_rate, int output_sample_rate, gpio_num_t spk_bclk, gpio_num_t spk_ws, gpio_num_t spk_dout, gpio_num_t mic_sck,  gpio_num_t mic_din);
    
    /** PDMマイクからデータを読み取り */
    int Read(int16_t* dest, int samples);
};

#endif // _NO_AUDIO_CODEC_H
