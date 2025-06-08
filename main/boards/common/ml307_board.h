/**
 * @file ml307_board.h
 * @brief ML307 4Gモジュール対応ボードの抽象化クラス
 * 
 * このファイルはML307 4Gモジュールを使用するESP32ボードの共通機能を提供します。
 * 4G/LTE接続、AT コマンド制御、ネットワーク通信プロトコルの実装を含みます。
 */
#ifndef ML307_BOARD_H
#define ML307_BOARD_H

#include "board.h"
#include <ml307_at_modem.h>

/**
 * @class Ml307Board
 * @brief ML307 4Gモジュール付きボードの基底クラス
 * 
 * このクラスはML307 4G/LTEモジュールを搭載したESP32ボードの共通実装を提供します。
 * ATコマンドによる4Gモジュール制御、ネットワーク接続管理、各種通信プロトコルの
 * 初期化などを行います。
 */
class Ml307Board : public Board {
protected:
    /** @brief ML307モジュール制御用ATモデムインスタンス */
    Ml307AtModem modem_;
    
    /**
     * @brief ボード情報のJSON取得
     * @return std::string ML307ボード固有の設定情報を含むJSON文字列
     */
    virtual std::string GetBoardJson() override;
    
    /**
     * @brief ネットワーク接続完了まで待機
     * 
     * ML307モジュールが4G/LTEネットワークに接続し、
     * IPアドレスが割り当てられるまで待機します。
     */
    void WaitForNetworkReady();

public:
    /**
     * @brief ML307ボードのコンストラクタ
     * @param tx_pin ESP32からML307へのUART送信ピン
     * @param rx_pin ML307からESP32へのUART受信ピン
     * @param rx_buffer_size UART受信バッファサイズ（デフォルト: 4096バイト）
     * 
     * ML307モジュールとの通信用UARTを初期化し、
     * ATコマンド制御の準備を行います。
     */
    Ml307Board(gpio_num_t tx_pin, gpio_num_t rx_pin, size_t rx_buffer_size = 4096);
    
    /**
     * @brief ボードタイプ名の取得
     * @return std::string "ml307" 固定文字列
     */
    virtual std::string GetBoardType() override;
    
    /**
     * @brief ネットワーク接続開始
     * 
     * ML307モジュールを初期化し、4G/LTEネットワークへの接続を開始します。
     * SIMカードの確認、ネットワーク登録、データ接続の確立を行います。
     */
    virtual void StartNetwork() override;
    
    /**
     * @brief HTTP クライアント作成
     * @return Http* ML307モジュール経由のHTTP通信用クライアントインスタンス
     */
    virtual Http* CreateHttp() override;
    
    /**
     * @brief WebSocket クライアント作成
     * @return WebSocket* ML307モジュール経由のWebSocket通信用クライアントインスタンス
     */
    virtual WebSocket* CreateWebSocket() override;
    
    /**
     * @brief MQTT クライアント作成
     * @return Mqtt* ML307モジュール経由のMQTT通信用クライアントインスタンス
     */
    virtual Mqtt* CreateMqtt() override;
    
    /**
     * @brief UDP クライアント作成
     * @return Udp* ML307モジュール経由のUDP通信用クライアントインスタンス
     */
    virtual Udp* CreateUdp() override;
    
    /**
     * @brief ネットワーク状態アイコン取得
     * @return const char* 現在の4G/LTE接続状態を表すアイコン文字
     */
    virtual const char* GetNetworkStateIcon() override;
    
    /**
     * @brief 省電力モード設定
     * @param enabled true: 省電力モード有効, false: 無効
     * 
     * ML307モジュールの省電力機能（PSM、eDRX等）を制御します。
     */
    virtual void SetPowerSaveMode(bool enabled) override;
    
    /**
     * @brief オーディオコーデック取得（未実装）
     * @return AudioCodec* nullptr（派生クラスで実装される）
     */
    virtual AudioCodec* GetAudioCodec() override { return nullptr; }
    
    /**
     * @brief デバイス状態のJSON取得
     * @return std::string デバイスの現在状態を含むJSON文字列
     */
    virtual std::string GetDeviceStatusJson() override;
};

#endif // ML307_BOARD_H
