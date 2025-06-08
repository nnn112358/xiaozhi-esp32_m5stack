/**
 * @file axp2101.h
 * @brief AXP2101 電源管理IC制御クラス
 * 
 * このファイルはAXP2101電源管理ICの制御機能を提供します。
 * バッテリー充電状態、電圧監視、温度測定、電源制御を含みます。
 */
#ifndef __AXP2101_H__
#define __AXP2101_H__

#include "i2c_device.h"

/**
 * @class Axp2101
 * @brief AXP2101電源管理IC制御クラス
 * 
 * AXP2101 PMICを使用したバッテリー管理機能を提供します。
 * 充電状態監視、バッテリーレベル取得、温度測定、電源制御などを行います。
 */
class Axp2101 : public I2cDevice {
public:
    /**
     * @brief AXP2101のコンストラクタ
     * @param i2c_bus I2Cマスターバスハンドル
     * @param addr AXP2101のI2Cアドレス
     */
    Axp2101(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    
    /**
     * @brief 充電状態の確認
     * @return bool 充電中の場合true、そうでなければfalse
     */
    bool IsCharging();
    
    /**
     * @brief 放電状態の確認
     * @return bool 放電中の場合true、そうでなければfalse
     */
    bool IsDischarging();
    
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
     * @brief チップ温度の取得
     * @return float チップ温度（摂氏）
     */
    float GetTemperature();
    
    /**
     * @brief システム電源オフ
     * 
     * AXP2101を使用してシステム全体の電源を切断します。
     */
    void PowerOff();

private:
    /**
     * @brief バッテリー電流方向の取得
     * @return int 電流方向（正：充電、負：放電）
     */
    int GetBatteryCurrentDirection();
};

#endif
