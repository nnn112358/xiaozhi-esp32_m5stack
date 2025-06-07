/**
 * @file circular_strip.h
 * @brief 円形LEDストリップ制御クラス
 * 
 * WS2812BなどのアドレッサブルLEDストリップを円形に配置した構成での
 * LED制御クラスです。各LEDを個別に制御し、アニメーション効果（点滅、
 * スクロール、呼吸など）を実現できます。デバイスの状態を視覚的に表社します。
 */
#ifndef _CIRCULAR_STRIP_H_
#define _CIRCULAR_STRIP_H_

#include "led.h"
#include <driver/gpio.h>
#include <led_strip.h>
#include <esp_timer.h>
#include <atomic>
#include <mutex>
#include <vector>

// デフォルト明るさ設定
#define DEFAULT_BRIGHTNESS 32  /**< 標準明るさレベル */
#define LOW_BRIGHTNESS 4       /**< 低明るさレベル */

/**
 * @struct StripColor
 * @brief LEDストリップの色情報を表す構造体
 */
struct StripColor {
    uint8_t red = 0;    /**< 赤色成分（0-255） */
    uint8_t green = 0;  /**< 緑色成分（0-255） */
    uint8_t blue = 0;   /**< 青色成分（0-255） */
};

/**
 * @class CircularStrip
 * @brief 円形LEDストリップ制御クラス
 * 
 * WS2812BなどのアドレッサブルLEDストリップを円形に配置した構成で、
 * リッチなLEDアニメーションとデバイス状態表示を実現します。
 * スムーズなアニメーションと精密な色制御が可能です。
 */
class CircularStrip : public Led {
public:
    /**
     * @brief 円形LEDストリップコンストラクタ
     * @param gpio LEDストリップ制御用GPIOピン
     * @param max_leds LEDの総数
     */
    CircularStrip(gpio_num_t gpio, uint8_t max_leds);
    virtual ~CircularStrip();

    /** デバイス状態変化時のLED表示制御 */
    void OnStateChanged() override;
    
    /** LEDの明るさレベルを設定 */
    void SetBrightness(uint8_t default_brightness, uint8_t low_brightness);
    
    /** すべてのLEDを同じ色に設定 */
    void SetAllColor(StripColor color);
    
    /** 指定したLEDを特定の色に設定 */
    void SetSingleColor(uint8_t index, StripColor color);
    
    /** 点滅アニメーション */
    void Blink(StripColor color, int interval_ms);
    
    /** 呼吸アニメーション（明るさの漸変） */
    void Breathe(StripColor low, StripColor high, int interval_ms);
    
    /** スクロールアニメーション（流れるような動き） */
    void Scroll(StripColor low, StripColor high, int length, int interval_ms);

private:
    // スレッド安全性とタスク管理
    std::mutex mutex_;                              /**< LEDストリップアクセス用ミューテックス */
    TaskHandle_t blink_task_ = nullptr;             /**< 点滅タスクハンドル */
    
    // ESP-IDF LEDストリップインターフェース
    led_strip_handle_t led_strip_ = nullptr;        /**< LEDストリップハンドル */
    int max_leds_ = 0;                              /**< LED総数 */
    std::vector<StripColor> colors_;                /**< 各LEDの現在の色情報 */
    
    // アニメーション制御
    int blink_counter_ = 0;                         /**< 点滅カウンタ */
    int blink_interval_ms_ = 0;                     /**< 点滅間隔（ミリ秒） */
    esp_timer_handle_t strip_timer_ = nullptr;      /**< LEDストリップアニメーションタイマー */
    std::function<void()> strip_callback_ = nullptr; /**< アニメーションコールバック関数 */

    // 明るさ設定
    uint8_t default_brightness_ = DEFAULT_BRIGHTNESS; /**< 標準明るさレベル */
    uint8_t low_brightness_ = LOW_BRIGHTNESS;         /**< 低明るさレベル */

    /** LEDストリップアニメーションタスクを開始 */
    void StartStripTask(int interval_ms, std::function<void()> cb);
    
    /** レインボーアニメーション */
    void Rainbow(StripColor low, StripColor high, int interval_ms);
    
    /** フェードアウトアニメーション */
    void FadeOut(int interval_ms);
};

#endif // _CIRCULAR_STRIP_H_
