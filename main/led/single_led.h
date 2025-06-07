/**
 * @file single_led.h
 * @brief 単一アドレッサブルLED制御クラス
 * 
 * WS2812BなどのアドレッサブルLED、1個だけを制御するクラスです。
 * RGB色制御、点滅アニメーションなどの機能を提供し、
 * デバイスの状態を色で表現します。
 */
#ifndef _SINGLE_LED_H_
#define _SINGLE_LED_H_

#include "led.h"
#include <driver/gpio.h>
#include <led_strip.h>
#include <esp_timer.h>
#include <atomic>
#include <mutex>

/**
 * @class SingleLed
 * @brief 単一アドレッサブルLED制御クラス
 * 
 * WS2812BなどのアドレッサブルLED、1個だけを制御します。
 * RGB色制御、点滅アニメーション、ON/OFF制御などの機能を提供し、
 * シンプルな状態表示に適しています。
 */
class SingleLed : public Led {
public:
    /**
     * @brief 単一LEDコンストラクタ
     * @param gpio LED制御用GPIOピン
     */
    SingleLed(gpio_num_t gpio);
    virtual ~SingleLed();

    /** デバイス状態変化時のLED表示制御 */
    void OnStateChanged() override;

private:
    // スレッド安全性とタスク管理
    std::mutex mutex_;                              /**< LEDアクセス用ミューテックス */
    TaskHandle_t blink_task_ = nullptr;             /**< 点滅タスクハンドル */
    
    // ESP-IDF LEDストリップインターフェース
    led_strip_handle_t led_strip_ = nullptr;        /**< LEDストリップハンドル（1個LED用） */
    
    // 色情報
    uint8_t r_ = 0, g_ = 0, b_ = 0;                  /**< 現在のRGB色値 */
    
    // 点滅アニメーション制御
    int blink_counter_ = 0;                         /**< 点滅回数カウンタ */
    int blink_interval_ms_ = 0;                     /**< 点滅間隔（ミリ秒） */
    esp_timer_handle_t blink_timer_ = nullptr;      /**< 点滅タイマー */

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
    
    /** LEDを点灯 */
    void TurnOn();
    
    /** LEDを消灯 */
    void TurnOff();
    
    /** LEDのRGB色を設定 */
    void SetColor(uint8_t r, uint8_t g, uint8_t b);
};

#endif // _SINGLE_LED_H_
