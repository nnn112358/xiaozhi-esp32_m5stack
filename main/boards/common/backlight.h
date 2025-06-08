/**
 * @file backlight.h
 * @brief ディスプレイバックライト制御クラス
 * 
 * このファイルはディスプレイのバックライト明度制御を提供します。
 * PWM制御による滑らかな明度調整とトランジション効果に対応します。
 */
#pragma once

#include <cstdint>
#include <functional>

#include <driver/gpio.h>
#include <esp_timer.h>

/**
 * @class Backlight
 * @brief バックライト制御の抽象基底クラス
 * 
 * ディスプレイのバックライト明度を制御する機能を提供します。
 * 段階的な明度変更（フェードイン/アウト）とタイマー制御を含みます。
 */
class Backlight {
public:
    /**
     * @brief コンストラクタ
     * 
     * バックライト制御の初期化を行います。
     * トランジションタイマーを作成し、初期明度を0に設定します。
     */
    Backlight();
    
    /**
     * @brief デストラクタ
     * 
     * トランジションタイマーを削除し、リソースを解放します。
     */
    ~Backlight();

    /**
     * @brief 保存済み明度に復元
     * 
     * 以前に設定された明度値に戻します。
     * 省電力モードからの復帰時などに使用されます。
     */
    void RestoreBrightness();
    
    /**
     * @brief 明度設定
     * @param brightness 設定する明度（0-100）
     * @param permanent true: 設定値を保存, false: 一時的な変更
     * 
     * 指定された明度に段階的に変更します。
     * permanentがtrueの場合、設定値は復元用に保存されます。
     */
    void SetBrightness(uint8_t brightness, bool permanent = false);
    
    /**
     * @brief 現在の明度取得
     * @return uint8_t 現在の明度値（0-100）
     */
    inline uint8_t brightness() const { return brightness_; }

protected:
    /**
     * @brief トランジションタイマーコールバック
     * 
     * 段階的な明度変更を実行します。
     * target_brightness_に向かって step_ ずつ変更します。
     */
    void OnTransitionTimer();
    
    /**
     * @brief 実際の明度設定実装（純粋仮想関数）
     * @param brightness 設定する明度（0-100）
     * 
     * 派生クラスで具体的なハードウェア制御を実装します。
     */
    virtual void SetBrightnessImpl(uint8_t brightness) = 0;

    /** @brief 段階的明度変更用タイマーハンドル */
    esp_timer_handle_t transition_timer_ = nullptr;
    
    /** @brief 現在の明度値（0-100） */
    uint8_t brightness_ = 0;
    
    /** @brief 目標明度値（0-100） */
    uint8_t target_brightness_ = 0;
    
    /** @brief 明度変更時のステップサイズ */
    uint8_t step_ = 1;
};

/**
 * @class PwmBacklight
 * @brief PWM制御によるバックライトクラス
 * 
 * PWM信号を使用してバックライトの明度を制御します。
 * 多くのLCDディスプレイで使用される一般的な制御方式です。
 */
class PwmBacklight : public Backlight {
public:
    /**
     * @brief PWMバックライトのコンストラクタ
     * @param pin バックライト制御用GPIOピン番号
     * @param output_invert 出力反転フラグ（true: 反転出力, false: 通常出力）
     * 
     * 指定されたGPIOピンでPWM制御を初期化します。
     * 一部のディスプレイでは論理反転が必要な場合があります。
     */
    PwmBacklight(gpio_num_t pin, bool output_invert = false);
    
    /**
     * @brief デストラクタ
     * 
     * PWMチャンネルを停止し、リソースを解放します。
     */
    ~PwmBacklight();

    /**
     * @brief PWM明度設定の実装
     * @param brightness 設定する明度（0-100）
     * 
     * PWM デューティサイクルを調整して明度を制御します。
     */
    void SetBrightnessImpl(uint8_t brightness) override;
};
