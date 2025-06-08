/**
 * @file knob.h
 * @brief ロータリーエンコーダー（ノブ）制御クラス
 * 
 * このファイルはロータリーエンコーダーの回転検出機能を提供します。
 * 音量調整やメニュー操作など、回転による入力制御に使用されます。
 */
#ifndef KNOB_H_
#define KNOB_H_

#include <driver/gpio.h>
#include <functional>
#include <esp_log.h>
#include <iot_knob.h>

/**
 * @class Knob
 * @brief ロータリーエンコーダー制御クラス
 * 
 * 2つのGPIOピンに接続されたロータリーエンコーダーの回転方向を検出し、
 * コールバック関数で通知する機能を提供します。音量調整やメニュー操作に使用されます。
 */
class Knob {
public:
    /**
     * @brief ロータリーエンコーダーのコンストラクタ
     * @param pin_a エンコーダーのA相信号ピン番号
     * @param pin_b エンコーダーのB相信号ピン番号
     * 
     * 2つのGPIOピンに接続されたロータリーエンコーダーを初期化します。
     * A相とB相の位相差により回転方向を検出します。
     */
    Knob(gpio_num_t pin_a, gpio_num_t pin_b);
    
    /**
     * @brief デストラクタ
     * 
     * ノブハンドルを削除し、使用していたリソースを解放します。
     */
    ~Knob();

    /**
     * @brief 回転検出時のコールバック設定
     * @param callback 回転検出時に呼び出される関数
     *                 引数: bool - 回転方向（true: 時計回り, false: 反時計回り）
     * 
     * ロータリーエンコーダーが回転した際に呼び出されるコールバック関数を設定します。
     * 回転方向に応じて音量調整やメニュー項目の選択などを行えます。
     */
    void OnRotate(std::function<void(bool)> callback);

private:
    /**
     * @brief ESP-IDF ノブライブラリからのコールバック関数
     * @param arg ユーザーデータ（Knobオブジェクトのポインタ）
     * @param data ノブイベントデータ
     * 
     * ESP-IDF のiot_knob ライブラリから呼び出される静的コールバック関数です。
     * 内部でインスタンスメソッドを呼び出します。
     */
    static void knob_callback(void* arg, void* data);

    /** @brief ESP-IDF ノブハンドル */
    knob_handle_t knob_handle_;
    
    /** @brief A相信号のGPIOピン番号 */
    gpio_num_t pin_a_;
    
    /** @brief B相信号のGPIOピン番号 */
    gpio_num_t pin_b_;
    
    /** @brief 回転検出時のコールバック関数 */
    std::function<void(bool)> on_rotate_;
};

#endif // KNOB_H_