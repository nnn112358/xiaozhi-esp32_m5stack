/**
 * @file esp32_camera.h
 * @brief ESP32カメラモジュール制御クラス
 * 
 * このファイルはESP32内蔵カメラインターフェースを使用したカメラ制御機能を提供します。
 * 画像キャプチャ、JPEG エンコード、AI画像解析、LVGLディスプレイ連携を含みます。
 */
#ifndef ESP32_CAMERA_H
#define ESP32_CAMERA_H

#include <esp_camera.h>
#include <lvgl.h>
#include <thread>
#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "camera.h"

/**
 * @struct JpegChunk
 * @brief JPEG データの断片を表す構造体
 */
struct JpegChunk {
    uint8_t* data;  /**< JPEG データバッファへのポインタ */
    size_t len;     /**< データサイズ（バイト） */
};

/**
 * @class Esp32Camera
 * @brief ESP32内蔵カメラの制御クラス
 * 
 * ESP32のカメラインターフェースを使用してカメラモジュールを制御します。
 * 画像キャプチャ、表示設定、AI解析用画像送信などの機能を提供します。
 */
class Esp32Camera : public Camera {
private:
    /** @brief 現在のカメラフレームバッファ */
    camera_fb_t* fb_ = nullptr;
    
    /** @brief LVGLディスプレイ用画像記述子 */
    lv_img_dsc_t preview_image_;
    
    /** @brief AI画像解析APIのURL */
    std::string explain_url_;
    
    /** @brief AI画像解析API認証トークン */
    std::string explain_token_;
    
    /** @brief JPEG エンコード処理用スレッド */
    std::thread encoder_thread_;

public:
    /**
     * @brief ESP32カメラのコンストラクタ
     * @param config ESP32カメラの設定構造体
     * 
     * 指定された設定でカメラモジュールを初期化します。
     * ピン配置、解像度、画質、バッファ設定などを含みます。
     */
    Esp32Camera(const camera_config_t& config);
    
    /**
     * @brief デストラクタ
     * 
     * カメラリソースを解放し、エンコードスレッドを終了します。
     * フレームバッファやその他のメモリも適切に解放されます。
     */
    ~Esp32Camera();

    /**
     * @brief AI画像解析サーバーの設定
     * @param url AI画像解析APIのエンドポイントURL
     * @param token API認証用のトークン文字列
     */
    virtual void SetExplainUrl(const std::string& url, const std::string& token) override;
    
    /**
     * @brief 画像キャプチャ実行
     * @return bool キャプチャ成功時true、失敗時false
     * 
     * カメラから現在の映像を撮影し、内部バッファに保存します。
     * 撮影された画像はJPEG形式でエンコードされ、後でAI解析に使用できます。
     */
    virtual bool Capture() override;
    
    /**
     * @brief 水平ミラー（左右反転）設定
     * @param enabled true: 水平ミラー有効, false: 無効
     * @return bool 設定成功時true、失敗時false
     * 
     * カメラ画像の左右反転を制御します。
     * セルフィーカメラなど、自然な表示が必要な場合に使用されます。
     */
    virtual bool SetHMirror(bool enabled) override;
    
    /**
     * @brief 垂直フリップ（上下反転）設定
     * @param enabled true: 垂直フリップ有効, false: 無効
     * @return bool 設定成功時true、失敗時false
     * 
     * カメラ画像の上下反転を制御します。
     * カメラモジュールの取り付け方向によって必要になる場合があります。
     */
    virtual bool SetVFlip(bool enabled) override;
    
    /**
     * @brief AI による画像説明生成
     * @param question 画像に対する質問文
     * @return std::string AI生成の画像説明文
     * 
     * 最後にキャプチャした画像をJPEG形式でAIサーバーに送信し、
     * 指定された質問に対する回答を画像説明として取得します。
     * ネットワーク接続とAI API の設定が必要です。
     */
    virtual std::string Explain(const std::string& question) override;
};

#endif // ESP32_CAMERA_H