/**
 * @file settings.h
 * @brief デバイス設定管理クラス
 * 
 * ESP32のNVS（Non-Volatile Storage）を使用して、デバイスの設定情報を
 * 永続化して保存・読み込みします。WiFi設定、ボリューム、明るさなどの
 * ユーザー設定をフラッシュメモリに保存し、電源オフ後も設定を維持します。
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <nvs_flash.h>

/**
 * @class Settings
 * @brief NVSを使用したデバイス設定管理クラス
 * 
 * ESP32のNVS（Non-Volatile Storage）をラップして、シンプルな
 * キーバリューストアインターフェースを提供します。
 * 文字列、整数の保存・読み込みが可能で、ネームスペースで管理されます。
 */
class Settings {
public:
    /**
     * @brief 設定クラスコンストラクタ
     * @param ns NVSネームスペース名
     * @param read_write 書き込み権限が必要かどうか（デフォルト：読み取り専用）
     */
    Settings(const std::string& ns, bool read_write = false);
    ~Settings();

    /**
     * @brief 文字列設定を取得
     * @param key 設定キー
     * @param default_value デフォルト値（キーがない場合）
     * @return 設定値またはデフォルト値
     */
    std::string GetString(const std::string& key, const std::string& default_value = "");
    
    /**
     * @brief 文字列設定を保存
     * @param key 設定キー
     * @param value 保存する値
     */
    void SetString(const std::string& key, const std::string& value);
    
    /**
     * @brief 整数設定を取得
     * @param key 設定キー
     * @param default_value デフォルト値（キーがない場合）
     * @return 設定値またはデフォルト値
     */
    int32_t GetInt(const std::string& key, int32_t default_value = 0);
    
    /**
     * @brief 整数設定を保存
     * @param key 設定キー
     * @param value 保存する値
     */
    void SetInt(const std::string& key, int32_t value);
    
    /**
     * @brief 特定のキーを削除
     * @param key 削除するキー
     */
    void EraseKey(const std::string& key);
    
    /** すべての設定を削除 */
    void EraseAll();

private:
    std::string ns_;                /**< NVSネームスペース名 */
    nvs_handle_t nvs_handle_ = 0;   /**< NVSハンドル */
    bool read_write_ = false;       /**< 書き込み権限フラグ */
    bool dirty_ = false;            /**< 変更フラグ（未使用） */
};

#endif
