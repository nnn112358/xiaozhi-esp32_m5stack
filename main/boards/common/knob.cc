/**
 * @file knob.cc
 * @brief ロータリーエンコーダーノブクラスの実装
 * 
 * 2ピンロータリーエンコーダーの回転検出を実装します。
 * ESP-IOT-SOLUTIONのiot_knobライブラリを使用して、左右回転の検出と
 * コールバック通知を提供します。
 */

#include "knob.h"

static const char* TAG = "Knob";

/**
 * @brief Knobクラスのコンストラクタ
 * @param pin_a ロータリーエンコーダーのAピン（位相、0°）
 * @param pin_b ロータリーエンコーダーのBピン（位相を90°）
 * 
 * 2ピンロータリーエンコーダーを初期化し、左右回転の検出コールバックを登録します。
 * GPIOピンのプルアップ設定やチャタリング除去はiot_knobライブラリが自動で行います。
 */
Knob::Knob(gpio_num_t pin_a, gpio_num_t pin_b) {
    // ロータリーエンコーダーの設定
    knob_config_t config = {
        .default_direction = 0,                        // デフォルト回転方向（0=時計回り）
        .gpio_encoder_a = static_cast<uint8_t>(pin_a), // Aピン番号（フェーズ1）
        .gpio_encoder_b = static_cast<uint8_t>(pin_b), // Bピン番号（フェーズ2）
    };

    esp_err_t err = ESP_OK;
    
    // ノブインスタンスを作成
    knob_handle_ = iot_knob_create(&config);
    if (knob_handle_ == NULL) {
        ESP_LOGE(TAG, "Failed to create knob instance");
        return;
    }

    // 左回転（反時計回り）コールバックを登録
    err = iot_knob_register_cb(knob_handle_, KNOB_LEFT, knob_callback, this);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register left callback: %s", esp_err_to_name(err));
        return;
    }

    // 右回転（時計回り）コールバックを登録
    err = iot_knob_register_cb(knob_handle_, KNOB_RIGHT, knob_callback, this);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register right callback: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Knob initialized with pins A:%d B:%d", pin_a, pin_b);
}

/**
 * @brief Knobクラスのデストラクタ
 * 
 * ノブインスタンスを削除し、GPIOリソースを解放します。
 * 登録されたコールバックも自動的に解除されます。
 */
Knob::~Knob() {
    if (knob_handle_ != NULL) {
        iot_knob_delete(knob_handle_);  // ノブリソースを解放
        knob_handle_ = NULL;
    }
}

/**
 * @brief 回転イベントのコールバック関数を登録
 * @param callback 回転時に呼び出される関数（引数: true=右回転、false=左回転）
 * 
 * ロータリーエンコーダーが回転されたときに呼び出されるコールバック関数を設定します。
 * コールバックは回転方向をbool値で受け取り、音量調節やメニュー選択などに利用できます。
 */
void Knob::OnRotate(std::function<void(bool)> callback) {
    on_rotate_ = callback;  // コールバック関数を保存
}

/**
 * @brief 内部コールバック関数（静的メソッド）
 * @param arg ノブハンドル（iot_knobライブラリから渡される）
 * @param data Knobインスタンスへのポインタ（コンストラクタで設定）
 * 
 * iot_knobライブラリから呼び出される内部コールバック関数です。
 * 回転イベントを取得し、ユーザーが登録したコールバック関数に方向情報を渡します。
 * CスタイルAPIとC++メソッドのブリッジ役を担います。
 */
void Knob::knob_callback(void* arg, void* data) {
    Knob* knob = static_cast<Knob*>(data);    // ユーザーデータからKnobインスタンスを取得
    knob_event_t event = iot_knob_get_event(arg); // 回転イベントを取得
    
    // ユーザーコールバックが登録されている場合は呼び出し
    if (knob->on_rotate_) {
        knob->on_rotate_(event == KNOB_RIGHT);  // KNOB_RIGHT=true、KNOB_LEFT=false
    }
}