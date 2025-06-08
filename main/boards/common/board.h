/**
 * @file board.h
 * @brief ハードウェアボード抽象化クラス
 * 
 * このファイルはさまざまなESP32ボードのハードウェア抽象化レイヤーを提供します。
 * 各ボードはこの基底クラスを継承し、固有のハードウェア構成を実装します。
 */
#ifndef BOARD_H
#define BOARD_H

#include <http.h>
#include <web_socket.h>
#include <mqtt.h>
#include <udp.h>
#include <string>

#include "led/led.h"
#include "backlight.h"
#include "camera.h"

// ファクトリ関数：各ボード実装で定義される
void* create_board();
class AudioCodec;
class Display;
/**
 * @class Board
 * @brief ハードウェアボードの基底抽象クラス
 * 
 * このクラスはシングルトンパターンで実装され、すべてのボード固有の
 * 機能（オーディオ、ディスプレイ、LED、ネットワークなど）への
 * 統一インターフェースを提供します。
 */
class Board {
private:
    Board(const Board&) = delete; // コピーコンストラクタを禁止
    Board& operator=(const Board&) = delete; // 代入演算子を禁止

protected:
    Board();
    std::string GenerateUuid();

    // ソフトウェアで生成されたデバイス固有識別子
    std::string uuid_;

public:
    /**
     * @brief シングルトンインスタンス取得
     * @return Board& ボードインスタンスの参照
     */
    static Board& GetInstance() {
        static Board* instance = static_cast<Board*>(create_board());
        return *instance;
    }

    virtual ~Board() = default;
    virtual std::string GetBoardType() = 0;
    virtual std::string GetUuid() { return uuid_; }
    virtual Backlight* GetBacklight() { return nullptr; }
    virtual Led* GetLed();
    virtual AudioCodec* GetAudioCodec() = 0;
    virtual bool GetTemperature(float& esp32temp);
    virtual Display* GetDisplay();
    virtual Camera* GetCamera();
    virtual Http* CreateHttp() = 0;
    virtual WebSocket* CreateWebSocket() = 0;
    virtual Mqtt* CreateMqtt() = 0;
    virtual Udp* CreateUdp() = 0;
    virtual void StartNetwork() = 0;
    virtual const char* GetNetworkStateIcon() = 0;
    virtual bool GetBatteryLevel(int &level, bool& charging, bool& discharging);
    virtual std::string GetJson();
    virtual void SetPowerSaveMode(bool enabled) = 0;
    virtual std::string GetBoardJson() = 0;
    virtual std::string GetDeviceStatusJson() = 0;
};

/**
 * @brief ボードクラス登録マクロ
 * 
 * 各ボード実装ファイルでこのマクロを使用して、
 * ファクトリ関数を自動生成します。
 */
#define DECLARE_BOARD(BOARD_CLASS_NAME) \
void* create_board() { \
    return new BOARD_CLASS_NAME(); \
}

#endif // BOARD_H
