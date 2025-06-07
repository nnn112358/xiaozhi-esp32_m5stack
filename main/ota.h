/**
 * @file ota.h
 * @brief OTA（Over-The-Air）ファームウェア更新システム
 * 
 * インターネット経由でファームウェアのバージョンチェック、ダウンロード、
 * インストールを自動化するシステムです。デバイスのアクティベーション、
 * サーバー設定の取得なども同時に行います。
 */
#ifndef _OTA_H
#define _OTA_H

#include <functional>
#include <string>

#include <esp_err.h>
#include "board.h"

/**
 * @class Ota
 * @brief OTAファームウェア更新システム
 * 
 * インターネット経由でファームウェアのバージョンチェック、ダウンロード、
 * インストールを行います。同時にデバイスのアクティベーション、
 * サーバー設定の取得なども実行し、デバイスの初期設定を自動化します。
 */
class Ota {
public:
    Ota();
    ~Ota();

    /** サーバーから新しいバージョンがあるかチェック */
    bool CheckVersion();
    
    /** デバイスをアクティベート（サーバーに登録） */
    esp_err_t Activate();
    
    // 状態確認メソッド群
    /** アクティベーションチャレンジがあるかどうか */
    bool HasActivationChallenge() { return has_activation_challenge_; }
    
    /** 新しいバージョンがあるかどうか */
    bool HasNewVersion() { return has_new_version_; }
    
    /** MQTT設定が取得されたかどうか */
    bool HasMqttConfig() { return has_mqtt_config_; }
    
    /** WebSocket設定が取得されたかどうか */
    bool HasWebsocketConfig() { return has_websocket_config_; }
    
    /** アクティベーションコードがあるかどうか */
    bool HasActivationCode() { return has_activation_code_; }
    
    /** サーバー時刻が取得されたかどうか */
    bool HasServerTime() { return has_server_time_; }
    
    /** OTAアップグレードを開始（進捗コールバック付き） */
    void StartUpgrade(std::function<void(int progress, size_t speed)> callback);
    
    /** 現在のバージョンを有効としてマーク */
    void MarkCurrentVersionValid();

    // 情報取得メソッド群
    /** サーバーから取得したファームウェアバージョンを取得 */
    const std::string& GetFirmwareVersion() const { return firmware_version_; }
    
    /** 現在のファームウェアバージョンを取得 */
    const std::string& GetCurrentVersion() const { return current_version_; }
    
    /** アクティベーションメッセージを取得 */
    const std::string& GetActivationMessage() const { return activation_message_; }
    
    /** アクティベーションコードを取得 */
    const std::string& GetActivationCode() const { return activation_code_; }
    
    /** バージョンチェックURLを生成 */
    std::string GetCheckVersionUrl();

private:
    // アクティベーション関連
    std::string activation_message_;            /**< アクティベーションメッセージ */
    std::string activation_code_;               /**< アクティベーションコード */
    std::string activation_challenge_;          /**< アクティベーションチャレンジ */
    std::string serial_number_;                 /**< シリアル番号 */
    int activation_timeout_ms_ = 30000;         /**< アクティベーションタイムアウト */
    
    // サーバーからの情報取得状態
    bool has_new_version_ = false;              /**< 新バージョン有無 */
    bool has_mqtt_config_ = false;              /**< MQTT設定取得済み */
    bool has_websocket_config_ = false;         /**< WebSocket設定取得済み */
    bool has_server_time_ = false;              /**< サーバー時刻取得済み */
    bool has_activation_code_ = false;          /**< アクティベーションコード取得済み */
    bool has_serial_number_ = false;            /**< シリアル番号取得済み */
    bool has_activation_challenge_ = false;     /**< アクティベーションチャレンジ取得済み */
    
    // バージョン情報
    std::string current_version_;               /**< 現在のファームウェアバージョン */
    std::string firmware_version_;              /**< サーバーの最新バージョン */
    std::string firmware_url_;                  /**< ファームウェアダウンロードURL */

    // アップグレード関連
    std::function<void(int progress, size_t speed)> upgrade_callback_;  /**< アップグレード進捗コールバック */

    /** ファームウェアアップグレードを実行 */
    void Upgrade(const std::string& firmware_url);
    
    /** バージョン文字列を解析して数値配列に変換 */
    std::vector<int> ParseVersion(const std::string& version);
    
    /** 新しいバージョンが利用可能かどうかを確認 */
    bool IsNewVersionAvailable(const std::string& currentVersion, const std::string& newVersion);
    
    /** アクティベーション用ペイロードを生成 */
    std::string GetActivationPayload();
    
    /** HTTPクライアントを設定 */
    Http* SetupHttp();
};

#endif // _OTA_H
