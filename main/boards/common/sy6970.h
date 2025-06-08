/**
 * @file sy6970.h
 * @brief SY6970バッテリー充電IC制御クラス
 * 
 * このファイルはSY6970バッテリー充電ICの制御機能を提供します。
 * 充電状態監視、電力品質確認、バッテリー電圧測定、充電制御を含みます。
 */
#ifndef __SY6970_H__
#define __SY6970_H__

#include "i2c_device.h"

/**
 * @class Sy6970
 * @brief SY6970バッテリー充電IC制御クラス
 * 
 * SY6970充電ICを使用したバッテリー管理機能を提供します。
 * 充電状態監視、電力品質確認、バッテリーレベル取得、電源制御などを行います。
 */
class Sy6970 : public I2cDevice {
public:
    /**
     * @brief SY6970のコンストラクタ
     * @param i2c_bus I2Cマスターバスハンドル
     * @param addr SY6970のI2Cアドレス
     */
    Sy6970(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    
    /**
     * @brief 充電状態の確認
     * @return bool 充電中の場合true、そうでなければfalse
     */
    bool IsCharging();
    
    /**
     * @brief 電力品質の確認
     * @return bool 電力供給が良好な場合true、そうでなければfalse
     * 
     * 入力電圧が適切な範囲にあり、安定した電力供給が
     * 行われているかを確認します。
     */
    bool IsPowerGood();
    
    /**
     * @brief 充電完了状態の確認
     * @return bool 充電完了の場合true、そうでなければfalse
     */
    bool IsChargingDone();
    
    /**
     * @brief バッテリー残量の取得
     * @return int バッテリー残量（パーセント：0-100）
     */
    int GetBatteryLevel();
    
    /**
     * @brief システム電源オフ
     * 
     * SY6970を使用してシステム全体の電源を切断します。
     */
    void PowerOff();

private:
    /**
     * @brief 充電状態の詳細取得
     * @return int 充電状態の詳細情報
     */
    int GetChangingStatus();
    
    /**
     * @brief バッテリー電圧の取得
     * @return int バッテリー電圧（mV）
     */
    int GetBatteryVoltage();
    
    /**
     * @brief 充電目標電圧の取得
     * @return int 設定された充電目標電圧（mV）
     */
    int GetChargeTargetVoltage();
};

#endif