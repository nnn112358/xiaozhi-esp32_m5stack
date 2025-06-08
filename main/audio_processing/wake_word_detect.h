/**
 * @file wake_word_detect.h
 * @brief ウェイクワード検出システム
 * 
 * ESP-SRライブラリを使用して、リアルタイムでウェイクワードを検出します。
 * 検出されたウェイクワード音声はOpusエンコードされ、サーバーに送信されます。
 * AFE（Audio Front-End）処理、音声活動検出、エコーキャンセレーションなどを組み合わせます。
 */
#ifndef WAKE_WORD_DETECT_H
#define WAKE_WORD_DETECT_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <esp_afe_sr_models.h>
#include <esp_nsn_models.h>

#include <list>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "audio_codec.h"

/**
 * @class WakeWordDetect
 * @brief ウェイクワード検出システム
 * 
 * ESP-SRライブラリを使用して、リアルタイムでウェイクワードを検出します。
 * 検出された音声はOpusエンコードされ、サーバーに送信されます。
 * エコーキャンセレーション、ノイズ抑制、ビームフォーミングなどの高度な音声処理を行います。
 */
class WakeWordDetect {
public:
    WakeWordDetect();
    ~WakeWordDetect();

    /** ウェイクワード検出システムをオーディオコーデックで初期化 */
    void Initialize(AudioCodec* codec);
    
    /** 音声データをウェイクワード検出システムに供給 */
    void Feed(const std::vector<int16_t>& data);
    
    /** ウェイクワード検出時のコールバックを設定 */
    void OnWakeWordDetected(std::function<void(const std::string& wake_word)> callback);
    
    /** ウェイクワード検出を開始 */
    void StartDetection();
    
    /** ウェイクワード検出を停止 */
    void StopDetection();
    
    /** ウェイクワード検出が動作中かどうかを確認 */
    bool IsDetectionRunning();
    
    /** 1回のフィードで必要なサンプル数を取得 */
    size_t GetFeedSize();
    
    /** ウェイクワード音声データをOpusエンコード */
    void EncodeWakeWordData();
    
    /** エンコード済みウェイクワードOpusデータを取得 */
    bool GetWakeWordOpus(std::vector<uint8_t>& opus);
    
    /** 最後に検出されたウェイクワードを取得 */
    const std::string& GetLastDetectedWakeWord() const { return last_detected_wake_word_; }

private:
    // ESP-SR AFEインターフェース
    esp_afe_sr_iface_t* afe_iface_ = nullptr;               /**< AFEインターフェース */
    esp_afe_sr_data_t* afe_data_ = nullptr;                 /**< AFEデータハンドル */
    char* wakenet_model_ = NULL;                            /**< ウェイクネットモデル */
    std::vector<std::string> wake_words_;                   /**< 検出可能なウェイクワードリスト */
    
    // FreeRTOSイベントとコールバック
    EventGroupHandle_t event_group_;                                            /**< タスク間通信用イベントグループ */
    std::function<void(const std::string& wake_word)> wake_word_detected_callback_;  /**< ウェイクワード検出コールバック */
    AudioCodec* codec_ = nullptr;                           /**< オーディオコーデックインスタンス */
    std::string last_detected_wake_word_;                   /**< 最後に検出したウェイクワード */

    // Opusエンコードタスク関連
    TaskHandle_t wake_word_encode_task_ = nullptr;          /**< ウェイクワードエンコードタスクハンドル */
    StaticTask_t wake_word_encode_task_buffer_;             /**< タスクバッファ */
    StackType_t* wake_word_encode_task_stack_ = nullptr;    /**< タスクスタック */
    
    // ウェイクワードデータキュー
    std::list<std::vector<int16_t>> wake_word_pcm_;         /**< ウェイクワードPCMデータキュー */
    std::list<std::vector<uint8_t>> wake_word_opus_;        /**< ウェイクワードOpusデータキュー */
    std::mutex wake_word_mutex_;                            /**< ウェイクワードデータの排他制御 */
    std::condition_variable wake_word_cv_;                  /**< ウェイクワードデータの通知 */

    /** ウェイクワード音声データを保存 */
    void StoreWakeWordData(uint16_t* data, size_t size);
    
    /** 音声検出タスク */
    void AudioDetectionTask();
};

#endif
