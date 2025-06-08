/**
 * @file settings.cc
 * @brief デバイス設定管理クラス実装
 * 
 * ESP32のNVS（Non-Volatile Storage）を使用した設定管理システムの実装です。
 * WiFi設定、音量、明るさなどのユーザー設定を永続化して保存します。
 */

#include "settings.h"

#include <esp_log.h>
#include <nvs_flash.h>

#define TAG "Settings"

/**
 * @brief Settingsクラスコンストラクタ
 * @param ns NVSネームスペース名
 * @param read_write 書き込み権限の有無
 * 
 * 指定されたネームスペースでNVSハンドルを開きます。
 */
Settings::Settings(const std::string& ns, bool read_write) : ns_(ns), read_write_(read_write) {
    // NVSハンドルを開く（読み取り専用または読み書き可能）
    nvs_open(ns.c_str(), read_write_ ? NVS_READWRITE : NVS_READONLY, &nvs_handle_);
}

/**
 * @brief Settingsクラスデストラクタ
 * 
 * 変更がある場合はNVSにコミットし、ハンドルを閉じます。
 */
Settings::~Settings() {
    if (nvs_handle_ != 0) {
        // 書き込み権限があり変更がある場合はコミット
        if (read_write_ && dirty_) {
            ESP_ERROR_CHECK(nvs_commit(nvs_handle_));
        }
        nvs_close(nvs_handle_);
    }
}

/**
 * @brief 文字列設定値を取得
 * @param key 設定キー
 * @param default_value デフォルト値（キーが存在しない場合）
 * @return 設定値またはデフォルト値
 */
std::string Settings::GetString(const std::string& key, const std::string& default_value) {
    // ハンドルが無効な場合はデフォルト値を返す
    if (nvs_handle_ == 0) {
        return default_value;
    }

    // まず文字列の長さを取得
    size_t length = 0;
    if (nvs_get_str(nvs_handle_, key.c_str(), nullptr, &length) != ESP_OK) {
        return default_value;
    }

    // 文字列データを読み取り
    std::string value;
    value.resize(length);
    ESP_ERROR_CHECK(nvs_get_str(nvs_handle_, key.c_str(), value.data(), &length));
    
    // 末尾のnull文字を除去
    while (!value.empty() && value.back() == '\0') {
        value.pop_back();
    }
    return value;
}

/**
 * @brief 文字列設定値を保存
 * @param key 設定キー
 * @param value 保存する値
 */
void Settings::SetString(const std::string& key, const std::string& value) {
    if (read_write_) {
        // NVSに文字列を保存
        ESP_ERROR_CHECK(nvs_set_str(nvs_handle_, key.c_str(), value.c_str()));
        dirty_ = true;  // 変更フラグを設定
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

/**
 * @brief 整数設定値を取得
 * @param key 設定キー
 * @param default_value デフォルト値（キーが存在しない場合）
 * @return 設定値またはデフォルト値
 */
int32_t Settings::GetInt(const std::string& key, int32_t default_value) {
    // ハンドルが無効な場合はデフォルト値を返す
    if (nvs_handle_ == 0) {
        return default_value;
    }

    int32_t value;
    if (nvs_get_i32(nvs_handle_, key.c_str(), &value) != ESP_OK) {
        return default_value;
    }
    return value;
}

/**
 * @brief 整数設定値を保存
 * @param key 設定キー
 * @param value 保存する値
 */
void Settings::SetInt(const std::string& key, int32_t value) {
    if (read_write_) {
        // NVSに整数を保存
        ESP_ERROR_CHECK(nvs_set_i32(nvs_handle_, key.c_str(), value));
        dirty_ = true;  // 変更フラグを設定
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

/**
 * @brief 指定されたキーを削除
 * @param key 削除するキー
 */
void Settings::EraseKey(const std::string& key) {
    if (read_write_) {
        auto ret = nvs_erase_key(nvs_handle_, key.c_str());
        // キーが存在しない場合はエラーとしない
        if (ret != ESP_ERR_NVS_NOT_FOUND) {
            ESP_ERROR_CHECK(ret);
        }
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}

/**
 * @brief ネームスペース内のすべての設定を削除
 */
void Settings::EraseAll() {
    if (read_write_) {
        // ネームスペース内のすべてのキーを削除
        ESP_ERROR_CHECK(nvs_erase_all(nvs_handle_));
    } else {
        ESP_LOGW(TAG, "Namespace %s is not open for writing", ns_.c_str());
    }
}
