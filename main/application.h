/**
 * @file application.h
 * @brief XiaoZhi ESP32メインアプリケーションクラス
 * 
 * このファイルはXiaoZhi ESP32デバイスのメインアプリケーションクラスを定義します。
 * シングルトンパターンを使用し、音声処理、プロトコル通信、IoTデバイス管理など
 * すべてのコア機能を統括します。
 */
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <esp_timer.h>

#include <string>
#include <mutex>
#include <list>
#include <vector>
#include <condition_variable>
#include <memory>

#include <opus_encoder.h>
#include <opus_decoder.h>
#include <opus_resampler.h>

#include "protocol.h"
#include "ota.h"
#include "background_task.h"
#include "audio_processor.h"

#if CONFIG_USE_WAKE_WORD_DETECT
#include "wake_word_detect.h"
#endif

// FreeRTOSイベント群のビットマスク定義
#define SCHEDULE_EVENT (1 << 0)                // タスクスケジューリングイベント
#define SEND_AUDIO_EVENT (1 << 1)              // 音声送信イベント
#define CHECK_NEW_VERSION_DONE_EVENT (1 << 2)  // バージョンチェック完了イベント

/**
 * @enum DeviceState
 * @brief デバイスの動作状態を表す列挙型
 */
enum DeviceState {
    kDeviceStateUnknown,        // 不明な状態
    kDeviceStateStarting,       // 起動中
    kDeviceStateWifiConfiguring, // WiFi設定中
    kDeviceStateIdle,           // 待機中
    kDeviceStateConnecting,     // サーバー接続中
    kDeviceStateListening,      // 音声入力待ち
    kDeviceStateSpeaking,       // 音声出力中
    kDeviceStateUpgrading,      // ファームウェア更新中
    kDeviceStateActivating,     // デバイス認証中
    kDeviceStateFatalError      // 致命的エラー
};

// Opus音声エンコーディング設定
#define OPUS_FRAME_DURATION_MS 60                           // Opusフレーム持続時間（ミリ秒）
#define MAX_AUDIO_PACKETS_IN_QUEUE (2400 / OPUS_FRAME_DURATION_MS)  // キューの最大音声パケット数

/**
 * @class Application
 * @brief XiaoZhi ESP32のメインアプリケーションクラス（シングルトン）
 * 
 * このクラスはXiaoZhi ESP32デバイスのすべての機能を統括します：
 * - 音声処理パイプライン（録音、エンコード、デコード、再生）
 * - ネットワーク通信（WebSocket/MQTT）
 * - IoTデバイス管理
 * - OTAファームウェア更新
 * - デバイス状態管理
 */
class Application {
public:
    static Application& GetInstance() {
        static Application instance;
        return instance;
    }
    // コピーコンストラクタと代入演算子を削除
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void Start();
    DeviceState GetDeviceState() const { return device_state_; }
    bool IsVoiceDetected() const { return voice_detected_; }
    void Schedule(std::function<void()> callback);
    void SetDeviceState(DeviceState state);
    void Alert(const char* status, const char* message, const char* emotion = "", const std::string_view& sound = "");
    void DismissAlert();
    void AbortSpeaking(AbortReason reason);
    void ToggleChatState();
    void StartListening();
    void StopListening();
    void UpdateIotStates();
    void Reboot();
    void WakeWordInvoke(const std::string& wake_word);
    void PlaySound(const std::string_view& sound);
    bool CanEnterSleepMode();
    void SendMcpMessage(const std::string& payload);

private:
    Application();
    ~Application();

#if CONFIG_USE_WAKE_WORD_DETECT
    WakeWordDetect wake_word_detect_;
#endif
    std::unique_ptr<AudioProcessor> audio_processor_;
    Ota ota_;
    std::mutex mutex_;
    std::list<std::function<void()>> main_tasks_;
    std::unique_ptr<Protocol> protocol_;
    EventGroupHandle_t event_group_ = nullptr;
    esp_timer_handle_t clock_timer_handle_ = nullptr;
    volatile DeviceState device_state_ = kDeviceStateUnknown;
    ListeningMode listening_mode_ = kListeningModeAutoStop;
#if CONFIG_USE_DEVICE_AEC || CONFIG_USE_SERVER_AEC
    bool realtime_chat_enabled_ = true;
#else
    bool realtime_chat_enabled_ = false;
#endif
    bool aborted_ = false;
    bool voice_detected_ = false;
    bool busy_decoding_audio_ = false;
    int clock_ticks_ = 0;
    TaskHandle_t check_new_version_task_handle_ = nullptr;

    // Audio encode / decode
    TaskHandle_t audio_loop_task_handle_ = nullptr;
    BackgroundTask* background_task_ = nullptr;
    std::chrono::steady_clock::time_point last_output_time_;
    std::list<AudioStreamPacket> audio_send_queue_;
    std::list<AudioStreamPacket> audio_decode_queue_;
    std::condition_variable audio_decode_cv_;

    // 追加：音声パケットのタイムスタンプキューを維持するため
    std::list<uint32_t> timestamp_queue_;
    std::mutex timestamp_mutex_;
    std::atomic<uint32_t> last_output_timestamp_ = 0;

    std::unique_ptr<OpusEncoderWrapper> opus_encoder_;
    std::unique_ptr<OpusDecoderWrapper> opus_decoder_;

    OpusResampler input_resampler_;
    OpusResampler reference_resampler_;
    OpusResampler output_resampler_;

    void MainEventLoop();
    void OnAudioInput();
    void OnAudioOutput();
    void ReadAudio(std::vector<int16_t>& data, int sample_rate, int samples);
    void ResetDecoder();
    void SetDecodeSampleRate(int sample_rate, int frame_duration);
    void CheckNewVersion();
    void ShowActivationCode();
    void OnClockTimer();
    void SetListeningMode(ListeningMode mode);
    void AudioLoop();
};

#endif // _APPLICATION_H_
