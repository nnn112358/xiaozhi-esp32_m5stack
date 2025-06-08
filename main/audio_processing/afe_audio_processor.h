/**
 * @file afe_audio_processor.h
 * @brief ESP-SR AFE（Audio Front-End）を使用した音声処理プロセッサ
 * 
 * Espressif社のESP-SRライブラリのAFE機能を使用して、
 * エコーキャンセレーション、ノイズサプレッション、VAD（Voice Activity Detection）、
 * ビームフォーミングなどの高度な音声前処理を実行します。
 */
#ifndef AFE_AUDIO_PROCESSOR_H
#define AFE_AUDIO_PROCESSOR_H

#include <esp_afe_sr_models.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <string>
#include <vector>
#include <functional>

#include "audio_processor.h"
#include "audio_codec.h"

/**
 * @class AfeAudioProcessor
 * @brief ESP-SR AFEを使用した音声処理プロセッサ
 * 
 * Espressif社のESP-SRライブラリのAFE（Audio Front-End）機能を使用して、
 * リアルタイムで高品質な音声前処理を実行します。
 * エコー除去、ノイズ抑制、VADなどの機能を提供します。
 */
class AfeAudioProcessor : public AudioProcessor {
public:
    AfeAudioProcessor();
    ~AfeAudioProcessor();

    /** AFEをオーディオコーデックで初期化 */
    void Initialize(AudioCodec* codec) override;
    
    /** 音声データをAFEに供給 */
    void Feed(const std::vector<int16_t>& data) override;
    
    /** AFE処理を開始 */
    void Start() override;
    
    /** AFE処理を停止 */
    void Stop() override;
    
    /** AFEが動作中かどうかを確認 */
    bool IsRunning() override;
    
    /** 処理済み音声データのコールバック設定 */
    void OnOutput(std::function<void(std::vector<int16_t>&& data)> callback) override;
    
    /** VAD状態変化コールバック設定 */
    void OnVadStateChange(std::function<void(bool speaking)> callback) override;
    
    /** 1回のフィードで必要なサンプル数を取得 */
    size_t GetFeedSize() override;

private:
    // FreeRTOSイベントグループ
    EventGroupHandle_t event_group_ = nullptr;              /**< タスク間通信用イベントグループ */
    
    // ESP-SR AFEインターフェース
    esp_afe_sr_iface_t* afe_iface_ = nullptr;               /**< AFEインターフェース */
    esp_afe_sr_data_t* afe_data_ = nullptr;                 /**< AFEデータハンドル */
    
    // コールバック関数
    std::function<void(std::vector<int16_t>&& data)> output_callback_;          /**< 処理済み音声データコールバック */
    std::function<void(bool speaking)> vad_state_change_callback_;              /**< VAD状態変化コールバック */
    
    // オーディオコーデックと状態
    AudioCodec* codec_ = nullptr;                           /**< オーディオコーデックインスタンス */
    bool is_speaking_ = false;                              /**< 現在の音声活動状態 */

    /** AFE音声処理タスク */
    void AudioProcessorTask();
};

#endif 