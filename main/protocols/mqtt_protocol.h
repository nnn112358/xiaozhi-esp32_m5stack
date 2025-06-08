/**
 * @file mqtt_protocol.h
 * @brief MQTT通信プロトコル実装
 * 
 * MQTTとUDPを組み合わせたハイブリッド通信プロトコルです。
 * MQTTで制御メッセージを交換し、UDPで高速な音声データ伝送を行います。
 * AES暗号化によりセキュアな通信を実現します。
 */
#ifndef MQTT_PROTOCOL_H
#define MQTT_PROTOCOL_H


#include "protocol.h"
#include <mqtt.h>
#include <udp.h>
#include <cJSON.h>
#include <mbedtls/aes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

#include <functional>
#include <string>
#include <map>
#include <mutex>

// MQTT通信パラメータ
#define MQTT_PING_INTERVAL_SECONDS 90       /**< MQTTキープアライブ間隔（秒） */
#define MQTT_RECONNECT_INTERVAL_MS 10000    /**< MQTT再接続間隔（ミリ秒） */

// イベントビットマスク定義
#define MQTT_PROTOCOL_SERVER_HELLO_EVENT (1 << 0)  /**< MQTTサーバーHelloメッセージ受信イベント */

/**
 * @class MqttProtocol
 * @brief MQTT + UDPハイブリッド通信プロトコル実装クラス
 * 
 * MQTTでコントロールメッセージを交換し、UDPで音声データを高速伝送します。
 * AES暗号化とシーケンス番号によりセキュアで信頼性の高い通信を実現します。
 * ネットワークの制約がある環境でも安定した音声通信が可能です。
 */
class MqttProtocol : public Protocol {
public:
    MqttProtocol();
    ~MqttProtocol();

    /** MQTT接続とUDPチャンネルを開始 */
    bool Start() override;
    
    /** 音声データパケットをUDPで送信 */
    bool SendAudio(const AudioStreamPacket& packet) override;
    
    /** 音声チャンネルをオープン */
    bool OpenAudioChannel() override;
    
    /** 音声チャンネルをクローズ */
    void CloseAudioChannel() override;
    
    /** 音声チャンネルがオープンされているかどうかを確認 */
    bool IsAudioChannelOpened() const override;

private:
    // FreeRTOSイベント管理
    EventGroupHandle_t event_group_handle_;         /**< プロトコルイベント管理用 */

    // MQTT設定
    std::string publish_topic_;                     /**< MQTTパブリッシュトピック */

    // スレッド安全性とネットワーク接続
    std::mutex channel_mutex_;                      /**< チャンネルアクセス用ミューテックス */
    Mqtt* mqtt_ = nullptr;                          /**< MQTTクライアントインスタンス */
    Udp* udp_ = nullptr;                            /**< UDPクライアントインスタンス */
    
    // AES暗号化関連
    mbedtls_aes_context aes_ctx_;                   /**< AES暗号化コンテキスト */
    std::string aes_nonce_;                         /**< AES暗号化nonce値 */
    
    // UDP接続情報
    std::string udp_server_;                        /**< UDPサーバーアドレス */
    int udp_port_;                                  /**< UDPサーバーポート番号 */
    
    // パケットシーケンス管理
    uint32_t local_sequence_;                       /**< ローカルシーケンス番号 */
    uint32_t remote_sequence_;                      /**< リモートシーケンス番号 */

    /** MQTTクライアントを開始 */
    bool StartMqttClient(bool report_error=false);
    
    /** サーバーからのHelloメッセージを解析 */
    void ParseServerHello(const cJSON* root);
    
    /** 16進数文字列をバイナリデータに変換 */
    std::string DecodeHexString(const std::string& hex_string);

    /** テキストメッセージをMQTTで送信 */
    bool SendText(const std::string& text) override;
    
    /** クライアントHelloメッセージを生成 */
    std::string GetHelloMessage();
};


#endif // MQTT_PROTOCOL_H
