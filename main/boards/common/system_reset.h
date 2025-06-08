/**
 * @file system_reset.h
 * @brief システムリセット制御クラス
 * 
 * このファイルはシステムリセット機能を提供します。
 * NVS設定リセット、ファクトリーリセット、システム再起動などを含みます。
 */
#ifndef _SYSTEM_RESET_H
#define _SYSTEM_RESET_H

#include <driver/gpio.h>

/**
 * @class SystemReset
 * @brief システムリセット制御クラス
 * 
 * 指定されたGPIOピンの状態を監視し、システムリセット操作を制御します。
 * NVS設定のリセット、ファクトリーリセット、システム再起動などの機能を提供します。
 */
class SystemReset {
public:
    /**
     * @brief SystemResetのコンストラクタ
     * @param reset_nvs_pin NVS設定リセット用GPIOピン
     * @param reset_factory_pin ファクトリーリセット用GPIOピン
     * 
     * 指定されたGPIOピンをリセット制御用に初期化します。
     * これらのピンの状態を監視してリセット操作を実行します。
     */
    SystemReset(gpio_num_t reset_nvs_pin, gpio_num_t reset_factory_pin);
    
    /**
     * @brief リセットボタン状態の確認
     * 
     * 設定されたGPIOピンの状態を確認し、リセット操作が要求されているかを判定します。
     * 必要に応じて対応するリセット処理を実行します。
     */
    void CheckButtons();

private:
    /** @brief NVS設定リセット用GPIOピン番号 */
    gpio_num_t reset_nvs_pin_;
    
    /** @brief ファクトリーリセット用GPIOピン番号 */
    gpio_num_t reset_factory_pin_;

    /**
     * @brief NVSフラッシュのリセット
     * 
     * 不揮発性ストレージ（NVS）に保存された設定をすべてクリアします。
     * WiFi設定、ユーザー設定、キャリブレーションデータなどが削除されます。
     */
    void ResetNvsFlash();
    
    /**
     * @brief ファクトリーリセットの実行
     * 
     * システムを工場出荷時の状態に戻します。
     * すべての設定、データ、カスタマイズが削除されます。
     */
    void ResetToFactory();
    
    /**
     * @brief 指定秒数後にシステム再起動
     * @param seconds 再起動までの待機時間（秒）
     * 
     * 指定された時間が経過した後にシステムを再起動します。
     * ユーザーに再起動の予告を表示する時間を提供します。
     */
    void RestartInSeconds(int seconds);
};

#endif
