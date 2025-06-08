/**
 * @file power_save_timer.h
 * @brief 省電力モード制御タイマークラス
 * 
 * このファイルは省電力管理機能を提供します。
 * 自動スリープ、CPU周波数制御、シャットダウンタイマー、電力管理コールバックを含みます。
 */
#pragma once

#include <functional>

#include <esp_timer.h>
#include <esp_pm.h>

/**
 * @class PowerSaveTimer
 * @brief 省電力モード制御タイマークラス
 * 
 * デバイスの省電力管理を行うクラスです。
 * 一定時間非アクティブ状態が続いた場合の自動スリープ、CPU周波数の調整、
 * 自動シャットダウンなどの機能を提供します。
 */
class PowerSaveTimer {
public:
    /**
     * @brief PowerSaveTimerのコンストラクタ
     * @param cpu_max_freq CPU最大周波数（MHz）
     * @param seconds_to_sleep スリープモードに入るまでの秒数（デフォルト: 20秒）
     * @param seconds_to_shutdown シャットダウンまでの秒数（デフォルト: -1で無効）
     * 
     * 省電力タイマーを初期化し、CPU電力管理を設定します。
     * スリープとシャットダウンの時間間隔を設定できます。
     */
    PowerSaveTimer(int cpu_max_freq, int seconds_to_sleep = 20, int seconds_to_shutdown = -1);
    
    /**
     * @brief デストラクタ
     * 
     * 省電力タイマーを停止し、リソースを解放します。
     */
    ~PowerSaveTimer();

    /**
     * @brief 省電力機能の有効/無効設定
     * @param enabled true: 省電力機能有効, false: 無効
     * 
     * 省電力タイマーの動作を制御します。
     * 無効にすると自動スリープやシャットダウンが停止されます。
     */
    void SetEnabled(bool enabled);
    
    /**
     * @brief スリープモード開始時のコールバック設定
     * @param callback スリープモードに入る際に呼び出される関数
     * 
     * デバイスがスリープモードに入る前に実行される処理を設定します。
     * ディスプレイ消灯、ネットワーク切断などの準備処理に使用されます。
     */
    void OnEnterSleepMode(std::function<void()> callback);
    
    /**
     * @brief スリープモード終了時のコールバック設定
     * @param callback スリープモードから復帰する際に呼び出される関数
     * 
     * デバイスがスリープモードから復帰した際に実行される処理を設定します。
     * ディスプレイ復帰、ネットワーク再接続などの復帰処理に使用されます。
     */
    void OnExitSleepMode(std::function<void()> callback);
    
    /**
     * @brief シャットダウン要求時のコールバック設定
     * @param callback シャットダウンが要求された際に呼び出される関数
     * 
     * 自動シャットダウンタイマーが発動した際に実行される処理を設定します。
     * データ保存、安全なシャットダウン準備などに使用されます。
     */
    void OnShutdownRequest(std::function<void()> callback);
    
    /**
     * @brief ウェイクアップ処理
     * 
     * デバイスをアクティブ状態に戻し、省電力タイマーをリセットします。
     * ユーザー操作やイベント発生時に呼び出されます。
     */
    void WakeUp();

private:
    /**
     * @brief 省電力チェック処理
     * 
     * 定期的に呼び出される内部関数で、省電力モードへの移行や
     * シャットダウンタイミングを判定します。
     */
    void PowerSaveCheck();

    /** @brief 省電力管理用タイマーハンドル */
    esp_timer_handle_t power_save_timer_ = nullptr;
    
    /** @brief 省電力機能有効フラグ */
    bool enabled_ = false;
    
    /** @brief 現在スリープモード中かのフラグ */
    bool in_sleep_mode_ = false;
    
    /** @brief 内部タイマーカウンタ */
    int ticks_ = 0;
    
    /** @brief CPU最大周波数（MHz） */
    int cpu_max_freq_;
    
    /** @brief スリープモードに入るまでの秒数 */
    int seconds_to_sleep_;
    
    /** @brief シャットダウンまでの秒数（-1で無効） */
    int seconds_to_shutdown_;

    /** @brief スリープモード開始時のコールバック関数 */
    std::function<void()> on_enter_sleep_mode_;
    
    /** @brief スリープモード終了時のコールバック関数 */
    std::function<void()> on_exit_sleep_mode_;
    
    /** @brief シャットダウン要求時のコールバック関数 */
    std::function<void()> on_shutdown_request_;
};
