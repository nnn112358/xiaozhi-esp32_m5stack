/**
 * @file audio_codec.h
 * @brief オーディオコーデックの基底抽象クラス
 * 
 * ESP32のI2Sインターフェースを使用して、さまざまなオーディオコーデックチップ
 * （ES8311、ES8374、ES8388など）を抽象化した統一インターフェースです。
 * 音声の入力、出力、ボリューム制御などを提供します。
 */
#ifndef _AUDIO_CODEC_H
#define _AUDIO_CODEC_H

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <driver/i2s_std.h>

#include <vector>
#include <string>
#include <functional>

#include "board.h"

// DMAバッファ設定
#define AUDIO_CODEC_DMA_DESC_NUM 6    // DMAディスクリプタ数
#define AUDIO_CODEC_DMA_FRAME_NUM 240 // フレームあたりのサンプル数

/**
 * @class AudioCodec
 * @brief オーディオコーデックの基底抽象クラス
 * 
 * さまざまなオーディオコーデックチップの共通インターフェースを定義します。
 * I2Sバスを介して音声データの入出力を行い、ボリューム制御、
 * チャンネルの有効/無効化などの機能を提供します。
 */
class AudioCodec {
public:
    AudioCodec();
    virtual ~AudioCodec();
    
    /** 出力ボリュームを設定（パーセント） */
    virtual void SetOutputVolume(int volume);
    
    /** 音声入力を有効/無効化 */
    virtual void EnableInput(bool enable);
    
    /** 音声出力を有効/無効化 */
    virtual void EnableOutput(bool enable);

    /** オーディオコーデックを開始 */
    void Start();
    
    /** 音声データを出力（スピーカー再生） */
    void OutputData(std::vector<int16_t>& data);
    
    /** 音声データを入力（マイク録音） */
    bool InputData(std::vector<int16_t>& data);

    // ゲッターメソッド群
    inline bool duplex() const { return duplex_; }                          /**< 全二重通信モードかどうか */
    inline bool input_reference() const { return input_reference_; }        /**< 入力リファレンスが有効かどうか */
    inline int input_sample_rate() const { return input_sample_rate_; }     /**< 入力サンプリングレート */
    inline int output_sample_rate() const { return output_sample_rate_; }   /**< 出力サンプリングレート */
    inline int input_channels() const { return input_channels_; }           /**< 入力チャンネル数 */
    inline int output_channels() const { return output_channels_; }         /**< 出力チャンネル数 */
    inline int output_volume() const { return output_volume_; }             /**< 現在の出力ボリューム */
    inline bool input_enabled() const { return input_enabled_; }           /**< 入力が有効かどうか */
    inline bool output_enabled() const { return output_enabled_; }         /**< 出力が有効かどうか */

protected:
    // I2Sハンドル
    i2s_chan_handle_t tx_handle_ = nullptr;  /**< I2S送信ハンドル（出力） */
    i2s_chan_handle_t rx_handle_ = nullptr;  /**< I2S受信ハンドル（入力） */

    // コーデック設定
    bool duplex_ = false;              /**< 全二重通信モード */
    bool input_reference_ = false;     /**< 入力リファレンス有効 */
    bool input_enabled_ = false;       /**< 入力有効フラグ */
    bool output_enabled_ = false;      /**< 出力有効フラグ */
    int input_sample_rate_ = 0;        /**< 入力サンプリングレート */
    int output_sample_rate_ = 0;       /**< 出力サンプリングレート */
    int input_channels_ = 1;           /**< 入力チャンネル数（デフォルト：モノラル） */
    int output_channels_ = 1;          /**< 出力チャンネル数（デフォルト：モノラル） */
    int output_volume_ = 70;           /**< 出力ボリューム（パーセント、デフォルト：70%） */

    /** コーデックから音声データを読み取り（サブクラスで実装） */
    virtual int Read(int16_t* dest, int samples) = 0;
    
    /** コーデックに音声データを書き込み（サブクラスで実装） */
    virtual int Write(const int16_t* data, int samples) = 0;
};

#endif // _AUDIO_CODEC_H
