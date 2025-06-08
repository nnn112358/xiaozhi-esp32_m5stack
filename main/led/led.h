/**
 * @file led.h
 * @brief LED制御の基底抽象クラス
 * 
 * さまざまなLED制御（単一LED、LEDストリップなど）の統一インターフェース。
 * デバイスの状態に応じてLEDの表示を制御します。
 */
#ifndef _LED_H_
#define _LED_H_

/**
 * @class Led
 * @brief LED制御の基底抽象クラス
 * 
 * デバイスの状態変化に応じてLEDの表示を変更するための
 * 基底インターフェースです。各ボードのLED構成に応じて継承します。
 */
class Led {
public:
    virtual ~Led() = default;
    
    /** デバイスの状態変化時に呼び出されるLED制御メソッド */
    virtual void OnStateChanged() = 0;
};

/**
 * @class NoLed
 * @brief LEDなしボード用のダミーLEDクラス
 * 
 * LEDが搭載されていないボード用のプレースホルダークラスです。
 * 何も処理を行いません。
 */
class NoLed : public Led {
public:
    /** 状態変化時の処理（何もしない） */
    virtual void OnStateChanged() override {}
};

#endif // _LED_H_
