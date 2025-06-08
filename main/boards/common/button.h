/**
 * @file button.h
 * @brief ボタン入力制御クラス
 * 
 * このファイルはGPIOボタンやADCボタンの入力処理を提供します。
 * シングルクリック、ダブルクリック、長押しなど様々なボタンイベントに対応します。
 */
#ifndef BUTTON_H_
#define BUTTON_H_

#include <driver/gpio.h>
#include <iot_button.h>
#include <button_types.h>
#include <button_adc.h>
#include <button_gpio.h>
#include <functional>

/**
 * @class Button
 * @brief ボタン入力処理の基底クラス
 * 
 * GPIOピンに接続されたボタンの各種イベント（クリック、長押し等）を
 * 検出し、コールバック関数で通知する機能を提供します。
 */
class Button {
public:
    /**
     * @brief 既存のボタンハンドルからButton オブジェクトを作成
     * @param button_handle ESP-IDF のボタンハンドル
     */
    Button(button_handle_t button_handle);
    
    /**
     * @brief GPIOピンからButton オブジェクトを作成
     * @param gpio_num ボタンが接続されているGPIOピン番号
     * @param active_high ボタンのアクティブレベル（true: HIGH アクティブ, false: LOW アクティブ）
     * @param long_press_time 長押し判定時間（ms）。0の場合はデフォルト値を使用
     * @param short_press_time 短押し判定時間（ms）。0の場合はデフォルト値を使用
     */
    Button(gpio_num_t gpio_num, bool active_high = false, uint16_t long_press_time = 0, uint16_t short_press_time = 0);
    
    /**
     * @brief デストラクタ
     * 
     * ボタンハンドルを削除し、リソースを解放します。
     */
    ~Button();

    /**
     * @brief ボタン押下開始時のコールバック設定
     * @param callback ボタンが押された瞬間に呼び出される関数
     */
    void OnPressDown(std::function<void()> callback);
    
    /**
     * @brief ボタン押下終了時のコールバック設定
     * @param callback ボタンが離された瞬間に呼び出される関数
     */
    void OnPressUp(std::function<void()> callback);
    
    /**
     * @brief 長押し時のコールバック設定
     * @param callback 設定時間以上ボタンが押し続けられた時に呼び出される関数
     */
    void OnLongPress(std::function<void()> callback);
    
    /**
     * @brief シングルクリック時のコールバック設定
     * @param callback ボタンが一回押されて離された時に呼び出される関数
     */
    void OnClick(std::function<void()> callback);
    
    /**
     * @brief ダブルクリック時のコールバック設定
     * @param callback ボタンが短時間に2回クリックされた時に呼び出される関数
     */
    void OnDoubleClick(std::function<void()> callback);
    
    /**
     * @brief 複数回クリック時のコールバック設定
     * @param callback 指定回数クリックされた時に呼び出される関数
     * @param click_count 検出するクリック回数（デフォルト: 3回）
     */
    void OnMultipleClick(std::function<void()> callback, uint8_t click_count = 3);

protected:
    /** @brief ボタンが接続されているGPIOピン番号 */
    gpio_num_t gpio_num_;
    
    /** @brief ESP-IDF ボタンハンドル */
    button_handle_t button_handle_ = nullptr;

    /** @brief 押下開始時のコールバック関数 */
    std::function<void()> on_press_down_;
    
    /** @brief 押下終了時のコールバック関数 */
    std::function<void()> on_press_up_;
    
    /** @brief 長押し時のコールバック関数 */
    std::function<void()> on_long_press_;
    
    /** @brief シングルクリック時のコールバック関数 */
    std::function<void()> on_click_;
    
    /** @brief ダブルクリック時のコールバック関数 */
    std::function<void()> on_double_click_;
    
    /** @brief 複数回クリック時のコールバック関数 */
    std::function<void()> on_multiple_click_;
};

#if CONFIG_SOC_ADC_SUPPORTED
/**
 * @class AdcButton
 * @brief ADC入力によるボタンクラス
 * 
 * アナログ電圧値でボタン状態を判定するADCボタンの実装です。
 * 複数のボタンを一つのADCチャンネルで制御する場合などに使用されます。
 */
class AdcButton : public Button {
public:
    /**
     * @brief ADC設定からAdcButton オブジェクトを作成
     * @param adc_config ADCボタンの設定構造体
     */
    AdcButton(const button_adc_config_t& adc_config);
};
#endif

#endif // BUTTON_H_
