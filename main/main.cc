/**
 * @file main.cc
 * @brief XiaoZhi ESP32メインエントリーポイント
 * 
 * ESP32ファームウェアのメイン関数。システムの初期化と
 * アプリケーションの起動を行います。
 */

#include <esp_log.h>
#include <esp_err.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <driver/gpio.h>
#include <esp_event.h>

#include "application.h"
#include "system_info.h"

#define TAG "main"

/**
 * @brief ESP32メインエントリーポイント
 * 
 * この関数はESP32が起動時に呼び出されるメイン関数です。
 * システムの基本的な初期化を行い、アプリケーションを起動します。
 */
extern "C" void app_main(void)
{
    // デフォルトイベントループを初期化
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // WiFi設定用のNVSフラッシュを初期化
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // アプリケーションを起動
    Application::GetInstance().Start();
}
