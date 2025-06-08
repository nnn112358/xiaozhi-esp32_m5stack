/**
 * @file protocol.h
 * @brief ネットワーク通信プロトコル基底クラス
 * 
 * WebSocketやMQTTなどの各種プロトコルの統一インターフェースと
 * バイナリメッセージフォーマットを定義します。
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cJSON.h>
#include <string>
#include <functional>
#include <chrono>
#include <vector>

/**
 * @struct AudioStreamPacket
 * @brief 音声ストリームパケット構造体
 */
struct AudioStreamPacket {
    uint32_t timestamp = 0;        // タイムスタンプ（ミリ秒）
    std::vector<uint8_t> payload;  // 音声データ（Opusエンコード済み）
};

/**
 * @struct BinaryProtocol2
 * @brief バイナリプロトコルバージョン2フォーマット
 */
struct BinaryProtocol2 {
    uint16_t version;       // プロトコルバージョン
    uint16_t type;          // メッセージタイプ (0: OPUS, 1: JSON)
    uint32_t reserved;      // 将来の拡張用予約領域
    uint32_t timestamp;     // タイムスタンプ（ミリ秒、サーバー側AEC用）
    uint32_t payload_size;  // ペイロードサイズ（バイト）
    uint8_t payload[];      // ペイロードデータ
} __attribute__((packed));

/**
 * @struct BinaryProtocol3
 * @brief バイナリプロトコルバージョン3フォーマット（軽量版）
 */
struct BinaryProtocol3 {
    uint8_t type;           // メッセージタイプ
    uint8_t reserved;       // 予約領域
    uint16_t payload_size;  // ペイロードサイズ
    uint8_t payload[];      // ペイロードデータ
} __attribute__((packed));

/**
 * @enum AbortReason
 * @brief 音声録音中断理由
 */
enum AbortReason {
    kAbortReasonNone,            // 中断なし
    kAbortReasonWakeWordDetected // ウェイクワード検出で中断
};

/**
 * @enum ListeningMode
 * @brief 音声入力モード
 */
enum ListeningMode {
    kListeningModeAutoStop,   // 自動停止モード
    kListeningModeManualStop, // 手動停止モード
    kListeningModeRealtime    // リアルタイムモード（AECサポートが必要）
};

/**
 * @class Protocol
 * @brief ネットワーク通信プロトコルの基底抽象クラス
 * 
 * WebSocket、MQTTなどの具体的なプロトコル実装の基底となるクラス。
 * 音声データの送受信、JSONメッセージ処理、接続管理などの
 * 機能を統一インターフェースで提供します。
 */
class Protocol {
public:
    virtual ~Protocol() = default;

    inline int server_sample_rate() const {
        return server_sample_rate_;
    }
    inline int server_frame_duration() const {
        return server_frame_duration_;
    }
    inline const std::string& session_id() const {
        return session_id_;
    }

    void OnIncomingAudio(std::function<void(AudioStreamPacket&& packet)> callback);
    void OnIncomingJson(std::function<void(const cJSON* root)> callback);
    void OnAudioChannelOpened(std::function<void()> callback);
    void OnAudioChannelClosed(std::function<void()> callback);
    void OnNetworkError(std::function<void(const std::string& message)> callback);

    virtual bool Start() = 0;
    virtual bool OpenAudioChannel() = 0;
    virtual void CloseAudioChannel() = 0;
    virtual bool IsAudioChannelOpened() const = 0;
    virtual bool SendAudio(const AudioStreamPacket& packet) = 0;
    virtual void SendWakeWordDetected(const std::string& wake_word);
    virtual void SendStartListening(ListeningMode mode);
    virtual void SendStopListening();
    virtual void SendAbortSpeaking(AbortReason reason);
    virtual void SendIotDescriptors(const std::string& descriptors);
    virtual void SendIotStates(const std::string& states);
    virtual void SendMcpMessage(const std::string& message);

protected:
    std::function<void(const cJSON* root)> on_incoming_json_;
    std::function<void(AudioStreamPacket&& packet)> on_incoming_audio_;
    std::function<void()> on_audio_channel_opened_;
    std::function<void()> on_audio_channel_closed_;
    std::function<void(const std::string& message)> on_network_error_;

    int server_sample_rate_ = 24000;
    int server_frame_duration_ = 60;
    bool error_occurred_ = false;
    std::string session_id_;
    std::chrono::time_point<std::chrono::steady_clock> last_incoming_time_;

    virtual bool SendText(const std::string& text) = 0;
    virtual void SetError(const std::string& message);
    virtual bool IsTimeout() const;
};

#endif // PROTOCOL_H

