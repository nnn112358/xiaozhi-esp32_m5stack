/**
 * @file wifi_board.h
 * @brief WiFi対応ボードの抽象化クラス
 * 
 * このファイルはWiFi機能を持つESP32ボードの共通機能を提供します。
 * WiFi接続管理、設定モード、ネットワーク通信プロトコルの実装を含みます。
 */
#ifndef WIFI_BOARD_H
#define WIFI_BOARD_H

#include "board.h"

/**
 * @class WifiBoard
 * @brief WiFi機能付きボードの基底クラス
 * 
 * このクラスはWiFi接続機能を持つESP32ボードの共通実装を提供します。
 * WiFi接続管理、設定モード切り替え、各種ネットワークプロトコルの
 * 初期化などを行います。
 */
class WifiBoard : public Board {
protected:
    /** @brief WiFi設定モードフラグ（true: 設定モード, false: 通常モード） */
    bool wifi_config_mode_ = false;
    
    /**
     * @brief WiFi設定モードに入る
     * 
     * WiFi接続情報が未設定の場合やリセット要求時に呼び出され、
     * APモードで設定用Webサーバーを起動します。
     */
    void EnterWifiConfigMode();
    
    /**
     * @brief ボード情報のJSON取得
     * @return std::string ボード固有の設定情報を含むJSON文字列
     */
    virtual std::string GetBoardJson() override;

public:
    /**
     * @brief コンストラクタ
     * 
     * WiFiボードの基本初期化を行います。
     * WiFi接続状態の確認と必要に応じて設定モードへの切り替えを行います。
     */
    WifiBoard();
    
    /**
     * @brief ボードタイプ名の取得
     * @return std::string "wifi" 固定文字列
     */
    virtual std::string GetBoardType() override;
    
    /**
     * @brief ネットワーク接続開始
     * 
     * WiFi Station モードでの接続を開始します。
     * 接続情報が未設定の場合は設定モードに移行します。
     */
    virtual void StartNetwork() override;
    
    /**
     * @brief HTTP クライアント作成
     * @return Http* HTTP通信用クライアントインスタンス
     */
    virtual Http* CreateHttp() override;
    
    /**
     * @brief WebSocket クライアント作成
     * @return WebSocket* WebSocket通信用クライアントインスタンス
     */
    virtual WebSocket* CreateWebSocket() override;
    
    /**
     * @brief MQTT クライアント作成
     * @return Mqtt* MQTT通信用クライアントインスタンス
     */
    virtual Mqtt* CreateMqtt() override;
    
    /**
     * @brief UDP クライアント作成
     * @return Udp* UDP通信用クライアントインスタンス
     */
    virtual Udp* CreateUdp() override;
    
    /**
     * @brief ネットワーク状態アイコン取得
     * @return const char* 現在のWiFi接続状態を表すアイコン文字
     */
    virtual const char* GetNetworkStateIcon() override;
    
    /**
     * @brief 省電力モード設定
     * @param enabled true: 省電力モード有効, false: 無効
     * 
     * WiFiの省電力機能（モデムスリープ等）を制御します。
     */
    virtual void SetPowerSaveMode(bool enabled) override;
    
    /**
     * @brief WiFi設定リセット
     * 
     * 保存されているWiFi接続情報を削除し、
     * 次回起動時に設定モードで起動するようにします。
     */
    virtual void ResetWifiConfiguration();
    
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

#endif // WIFI_BOARD_H
