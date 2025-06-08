/**
 * @file button.cc
 * @brief ボタン入力制御クラスの実装
 * 
 * GPIOボタンやADCボタンの入力処理を実装します。
 * ESP-IOT-SOLUTIONのiot_buttonライブラリを使用して、
 * 様々なボタンイベントを検出しコールバック関数で通知します。
 */

#include "button.h"

#include <button_gpio.h>
#include <esp_log.h>

#define TAG "Button"

#if CONFIG_SOC_ADC_SUPPORTED
/**
 * @brief ADCボタンのコンストラクタ
 * @param adc_config ADCボタンの設定構造体
 * 
 * アナログ電圧値でボタン状態を判定するADCボタンを作成します。
 * 複数のボタンを一つのADCチャンネルで制御する場合に使用されます。
 */
AdcButton::AdcButton(const button_adc_config_t& adc_config) : Button(nullptr) {
    // ボタンの基本設定
    button_config_t btn_config = {
        .long_press_time = 2000,  // 長押し判定時間（2秒）
        .short_press_time = 0,    // 短押し判定時間（デフォルト値使用）
    };
    
    // ADCボタンデバイスを作成
    ESP_ERROR_CHECK(iot_button_new_adc_device(&btn_config, &adc_config, &button_handle_));
}
#endif

/**
 * @brief 既存のボタンハンドルからButtonオブジェクトを作成
 * @param button_handle ESP-IOT-SOLUTIONのボタンハンドル
 * 
 * 既に作成済みのiot_buttonハンドルをラップするコンストラクタです。
 * 外部でボタンデバイスを作成済みの場合に使用されます。
 */
Button::Button(button_handle_t button_handle) : button_handle_(button_handle) {
}

/**
 * @brief GPIOピンからButtonオブジェクトを作成
 * @param gpio_num ボタンが接続されているGPIOピン番号
 * @param active_high ボタンのアクティブレベル（true: HIGH、false: LOW）
 * @param long_press_time 長押し判定時間（ms）。0の場合はデフォルト値
 * @param short_press_time 短押し判定時間（ms）。0の場合はデフォルト値
 * 
 * 指定されたGPIOピンにボタンデバイスを作成します。内部プルアップ/
 * プルダウン抵抗が自動で設定され、チャタリング除去も行われます。
 */
Button::Button(gpio_num_t gpio_num, bool active_high, uint16_t long_press_time, uint16_t short_press_time) : gpio_num_(gpio_num) {
    // GPIO_NUM_NCが指定された場合はボタンを作成しない
    if (gpio_num == GPIO_NUM_NC) {
        return;
    }
    
    // ボタンの基本設定
    button_config_t button_config = {
        .long_press_time = long_press_time,   // 長押し判定時間（0でデフォルト）
        .short_press_time = short_press_time  // 短押し判定時間（0でデフォルト）
    };
    
    // GPIO設定
    button_gpio_config_t gpio_config = {
        .gpio_num = gpio_num,                               // GPIOピン番号
        .active_level = static_cast<uint8_t>(active_high ? 1 : 0),  // アクティブレベル
        .enable_power_save = false,                         // 省電力モード無効
        .disable_pull = false                               // 内部プル抵抗有効
    };
    
    // GPIOボタンデバイスを作成
    ESP_ERROR_CHECK(iot_button_new_gpio_device(&button_config, &gpio_config, &button_handle_));
}

/**
 * @brief Buttonクラスのデストラクタ
 * 
 * ボタンハンドルを削除し、使用していたリソースを解放します。
 * 登録されたコールバック関数も自動的に解除されます。
 */
Button::~Button() {
    if (button_handle_ != NULL) {
        iot_button_delete(button_handle_);
    }
}

/**
 * @brief ボタン押下開始時のコールバック設定
 * @param callback ボタンが押された瞬間に呼び出される関数
 * 
 * ボタンが物理的に押された瞬間（HIGHまたはLOWになった瞬間）に
 * 呼び出されるコールバック関数を設定します。チャタリング除去済みです。
 */
void Button::OnPressDown(std::function<void()> callback) {
    if (button_handle_ == nullptr) {
        return;
    }
    on_press_down_ = callback;
    iot_button_register_cb(button_handle_, BUTTON_PRESS_DOWN, nullptr, [](void* handle, void* usr_data) {
        Button* button = static_cast<Button*>(usr_data);
        if (button->on_press_down_) {
            button->on_press_down_();
        }
    }, this);
}

/**
 * @brief ボタン押下終了時のコールバック設定
 * @param callback ボタンが離された瞬間に呼び出される関数
 * 
 * ボタンが物理的に離された瞬間（LOWまたはHIGHになった瞬間）に
 * 呼び出されるコールバック関数を設定します。
 */
void Button::OnPressUp(std::function<void()> callback) {
    if (button_handle_ == nullptr) {
        return;
    }
    on_press_up_ = callback;
    iot_button_register_cb(button_handle_, BUTTON_PRESS_UP, nullptr, [](void* handle, void* usr_data) {
        Button* button = static_cast<Button*>(usr_data);
        if (button->on_press_up_) {
            button->on_press_up_();
        }
    }, this);
}

/**
 * @brief 長押し時のコールバック設定
 * @param callback 設定時間以上ボタンが押し続けられた時に呼び出される関数
 * 
 * コンストラクタで指定した long_press_time 以上ボタンが押し続けられた
 * 場合に呼び出されるコールバック関数を設定します。
 */
void Button::OnLongPress(std::function<void()> callback) {
    if (button_handle_ == nullptr) {
        return;
    }
    on_long_press_ = callback;
    iot_button_register_cb(button_handle_, BUTTON_LONG_PRESS_START, nullptr, [](void* handle, void* usr_data) {
        Button* button = static_cast<Button*>(usr_data);
        if (button->on_long_press_) {
            button->on_long_press_();
        }
    }, this);
}

/**
 * @brief シングルクリック時のコールバック設定
 * @param callback ボタンが一回押されて離された時に呼び出される関数
 * 
 * ボタンが短時間で押されて離された場合（シングルクリック）に
 * 呼び出されるコールバック関数を設定します。ダブルクリック判定の
 * 待機時間経過後に確定されます。
 */
void Button::OnClick(std::function<void()> callback) {
    if (button_handle_ == nullptr) {
        return;
    }
    on_click_ = callback;
    iot_button_register_cb(button_handle_, BUTTON_SINGLE_CLICK, nullptr, [](void* handle, void* usr_data) {
        Button* button = static_cast<Button*>(usr_data);
        if (button->on_click_) {
            button->on_click_();
        }
    }, this);
}

/**
 * @brief ダブルクリック時のコールバック設定
 * @param callback ボタンが短時間に2回クリックされた時に呼び出される関数
 * 
 * ボタンが短時間内に2回連続でクリックされた場合に呼び出される
 * コールバック関数を設定します。この場合OnClickは呼び出されません。
 */
void Button::OnDoubleClick(std::function<void()> callback) {
    if (button_handle_ == nullptr) {
        return;
    }
    on_double_click_ = callback;
    iot_button_register_cb(button_handle_, BUTTON_DOUBLE_CLICK, nullptr, [](void* handle, void* usr_data) {
        Button* button = static_cast<Button*>(usr_data);
        if (button->on_double_click_) {
            button->on_double_click_();
        }
    }, this);
}

/**
 * @brief 複数回クリック時のコールバック設定
 * @param callback 指定回数クリックされた時に呼び出される関数
 * @param click_count 検出するクリック回数（デフォルト: 3回）
 * 
 * ボタンが指定された回数連続でクリックされた場合に呼び出される
 * コールバック関数を設定します。トリプルクリック以上の検出に使用されます。
 */
void Button::OnMultipleClick(std::function<void()> callback, uint8_t click_count) {
    if (button_handle_ == nullptr) {
        return;
    }
    on_multiple_click_ = callback;
    
    // 複数クリックの検出回数を設定
    button_event_args_t event_args = {
        .multiple_clicks = {
            .clicks = click_count  // 検出するクリック回数
        }
    };
    
    iot_button_register_cb(button_handle_, BUTTON_MULTIPLE_CLICK, &event_args, [](void* handle, void* usr_data) {
        Button* button = static_cast<Button*>(usr_data);
        if (button->on_multiple_click_) {
            button->on_multiple_click_();
        }
    }, this);
}