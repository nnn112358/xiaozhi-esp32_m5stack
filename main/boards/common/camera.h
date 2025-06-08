/**
 * @file camera.h
 * @brief カメラ機能の抽象化インターフェース
 * 
 * このファイルはESP32カメラモジュールの制御とAI画像解析機能を提供します。
 * 画像キャプチャ、画像設定調整、AI による画像説明生成に対応します。
 */
#ifndef CAMERA_H
#define CAMERA_H

#include <string>

/**
 * @class Camera
 * @brief カメラ制御とAI画像解析の抽象基底クラス
 * 
 * ESP32に接続されたカメラモジュールの制御機能を提供します。
 * 画像キャプチャ、表示設定の調整、AI サーバーでの画像解析などを行います。
 */
class Camera {
public:
    /**
     * @brief AI画像解析サーバーの設定
     * @param url AI画像解析APIのエンドポイントURL
     * @param token API認証用のトークン文字列
     * 
     * 撮影した画像をAIで解析するためのサーバー接続情報を設定します。
     * このURLとトークンを使用して画像説明を取得します。
     */
    virtual void SetExplainUrl(const std::string& url, const std::string& token) = 0;
    
    /**
     * @brief 画像キャプチャ実行
     * @return bool キャプチャ成功時true、失敗時false
     * 
     * カメラで現在の映像を撮影し、メモリに保存します。
     * 撮影された画像は後でAI解析や表示に使用されます。
     */
    virtual bool Capture() = 0;
    
    /**
     * @brief 水平ミラー（左右反転）設定
     * @param enabled true: 水平ミラー有効, false: 無効
     * @return bool 設定成功時true、失敗時false
     * 
     * カメラ画像の水平方向の反転を制御します。
     * フロントカメラの場合、自然な表示のために有効にすることがあります。
     */
    virtual bool SetHMirror(bool enabled) = 0;
    
    /**
     * @brief 垂直フリップ（上下反転）設定
     * @param enabled true: 垂直フリップ有効, false: 無効
     * @return bool 設定成功時true、失敗時false
     * 
     * カメラ画像の垂直方向の反転を制御します。
     * カメラの取り付け向きによって調整が必要な場合があります。
     */
    virtual bool SetVFlip(bool enabled) = 0;
    
    /**
     * @brief AI による画像説明生成
     * @param question 画像に対する質問文（例: "この画像に何が写っていますか？"）
     * @return std::string AI生成の画像説明文
     * 
     * 最後にキャプチャした画像をAIサーバーに送信し、
     * 指定された質問に対する回答を画像説明として取得します。
     */
    virtual std::string Explain(const std::string& question) = 0;
};

#endif // CAMERA_H
