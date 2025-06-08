/**
 * @file gpio_led.h
 * @brief GPIO制御単一LEDクラス
 * 
 * GPIOピンで制御される単一LEDの制御クラスです。
 * PWM制御による明るさ調整、点滅アニメーション、フェード効果などの
 * 機能を提供します。デバイスの状態を視覚的に表示します。
 */
#ifndef _GPIO_LED_H_
#define _GPIO_LED_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "led.h"
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_timer.h>
#include <atomic>
#include <mutex>

/**
 * @class GpioLed
 * @brief GPIO制御単一LEDクラス
 * 
 * GPIOピンで制御される単一LEDを管理します。
 * ESP32のLEDC（PWM）コントローラーを使用して明るさ制御、
 * 点滅アニメーション、フェード効果などを実現します。
 */
class GpioLed : public Led {
 public:
    /**
     * @brief GPIO LEDコンストラクタ（シンプル）
     * @param gpio LED制御用GPIOピン
     */
    GpioLed(gpio_num_t gpio);
    
    /**
     * @brief GPIO LEDコンストラクタ（出力反転指定）
     * @param gpio LED制御用GPIOピン
     * @param output_invert 出力反転（0：正論理、1：負論理）
     */
    GpioLed(gpio_num_t gpio, int output_invert);
    
    /**
     * @brief GPIO LEDコンストラクタ（詳細設定）
     * @param gpio LED制御用GPIOピン
     * @param output_invert 出力反転フラグ
     * @param timer_num LEDCタイマー番号
     * @param channel LEDCチャンネル番号
     */
    GpioLed(gpio_num_t gpio, int output_invert, ledc_timer_t timer_num, ledc_channel_t channel);
    virtual ~GpioLed();

    /** デバイス状態変化時のLED表示制御 */
    void OnStateChanged() override;
    
    /** LEDを点灯 */
    void TurnOn();
    
    /** LEDを消灯 */
    void TurnOff();
    
    /** LEDの明るさを設定（0-255） */
    void SetBrightness(uint8_t brightness);

 private:
    // スレッド安全性とタスク管理
    std::mutex mutex_;                              /**< LEDアクセス用ミューテックス */
    TaskHandle_t blink_task_ = nullptr;             /**< 点滅タスクハンドル */
    
    // LEDC PWM設定
    ledc_channel_config_t ledc_channel_ = {0};      /**< LEDCチャンネル設定 */
    bool ledc_initialized_ = false;                 /**< LEDC初期化状態 */
    uint32_t duty_ = 0;                             /**< 現在のPWMデューティ比 */
    
    // 点滅アニメーション制御
    int blink_counter_ = 0;                         /**< 点滅回数カウンタ */
    int blink_interval_ms_ = 0;                     /**< 点滅間隔（ミリ秒） */
    esp_timer_handle_t blink_timer_ = nullptr;      /**< 点滅タイマー */
    bool fade_up_ = true;                           /**< フェード方向（true:明るく、false:暗く） */

    /** 点滅タスクを開始 */
    void StartBlinkTask(int times, int interval_ms);
    
    /** 点滅タイマーコールバック */
    void OnBlinkTimer();

    /** 1回だけ点滅 */
    void BlinkOnce();
    
    /** 指定回数点滅 */
    void Blink(int times, int interval_ms);
    
    /** 連続点滅を開始 */
    void StartContinuousBlink(int interval_ms);
    
    /** フェードタスクを開始 */
    void StartFadeTask();
    
    /** フェード終了時の処理 */
    void OnFadeEnd();
    
    /** LEDCフェードコールバック関数（IRAM配置） */
    static bool IRAM_ATTR FadeCallback(const ledc_cb_param_t *param, void *user_arg);
};

#endif  // _GPIO_LED_H_
