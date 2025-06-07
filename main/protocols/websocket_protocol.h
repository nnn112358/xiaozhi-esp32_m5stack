/**
 * @file websocket_protocol.h
 * @brief WebSocket通信プロトコル実装
 * 
 * WebSocketを使用してサーバーとリアルタイムで音声データや制御メッセージを
 * 交換します。バイナリプロトコルでOpusエンコードされた音声データと
 * JSONフォーマットの制御メッセージを送受信します。
 */
#ifndef _WEBSOCKET_PROTOCOL_H_
#define _WEBSOCKET_PROTOCOL_H_


#include "protocol.h"

#include <web_socket.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

// イベントビットマスク定義
#define WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT (1 << 0)  /**< サーバーからのHelloメッセージ受信イベント */

/**
 * @class WebsocketProtocol
 * @brief WebSocket通信プロトコル実装クラス
 * 
 * WebSocketを使用してサーバーと双方向通信を行います。
 * 音声データはOpusエンコードされ、バイナリプロトコルで送信されます。
 * 制御メッセージはJSONフォーマットで交換されます。
 */
class WebsocketProtocol : public Protocol {
public:
    WebsocketProtocol();
    ~WebsocketProtocol();

    /** WebSocket接続とプロトコルを開始 */
    bool Start() override;
    
    /** 音声データパケットをサーバーに送信 */
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
    
    // WebSocket接続
    WebSocket* websocket_ = nullptr;                /**< WebSocketインスタンス */
    int version_ = 1;                               /**< プロトコルバージョン */

    /** サーバーからのHelloメッセージを解析 */
    void ParseServerHello(const cJSON* root);
    
    /** テキストメッセージをサーバーに送信 */
    bool SendText(const std::string& text) override;
    
    /** クライアントHelloメッセージを生成 */
    std::string GetHelloMessage();
};

#endif
