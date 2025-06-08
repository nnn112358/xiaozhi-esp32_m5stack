/**
 * @file oled_display.h
 * @brief OLEDディスプレイ制御クラス
 * 
 * I2Cインターフェースで接続されるOLEDディスプレイをLVGLで制御します。
 * 128x64、128x32などの一般的なOLEDサイズに対応し、
 * 高コントラストで省電力な表示を実現します。
 */
#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "display.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

/**
 * @class OledDisplay
 * @brief OLEDディスプレイ制御クラス
 * 
 * I2Cインターフェースで接続されるOLEDディスプレイをLVGLで制御します。
 * モノクロ表示で高コントラストな表示が特徴で、省電力で動作します。
 * 128x64、128x32などの一般的なサイズに対応しています。
 */
class OledDisplay : public Display {
private:
    // ESP-IDF LCDパネルインターフェース
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;  /**< OLEDパネルIOハンドル（I2C） */
    esp_lcd_panel_handle_t panel_ = nullptr;        /**< OLEDパネルハンドル */

    // LVGL UI要素
    lv_obj_t* status_bar_ = nullptr;                /**< ステータスバーオブジェクト */
    lv_obj_t* content_ = nullptr;                   /**< メインコンテンツオブジェクト */
    lv_obj_t* content_left_ = nullptr;              /**< 左側コンテンツオブジェクト */
    lv_obj_t* content_right_ = nullptr;             /**< 右側コンテンツオブジェクト */
    lv_obj_t* container_ = nullptr;                 /**< コンテナオブジェクト */
    lv_obj_t* side_bar_ = nullptr;                  /**< サイドバーオブジェクト */

    DisplayFonts fonts_;                            /**< 使用するフォント群 */

    /** LVGLミューテックスをロック */
    virtual bool Lock(int timeout_ms = 0) override;
    
    /** LVGLミューテックスをアンロック */
    virtual void Unlock() override;

    /** 128x64 OLED用UIレイアウトを設定 */
    void SetupUI_128x64();
    
    /** 128x32 OLED用UIレイアウトを設定 */
    void SetupUI_128x32();

public:
    /**
     * @brief OLEDディスプレイコンストラクタ
     * @param panel_io OLEDパネルIOハンドル
     * @param panel OLEDパネルハンドル
     * @param width OLEDディスプレイ幅
     * @param height OLEDディスプレイ高さ
     * @param mirror_x X軸ミラーリング有効フラグ
     * @param mirror_y Y軸ミラーリング有効フラグ
     * @param fonts 使用するフォント群
     */
    OledDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width, int height, bool mirror_x, bool mirror_y,
                DisplayFonts fonts);
    ~OledDisplay();

    /** チャットメッセージをOLEDに表示 */
    virtual void SetChatMessage(const char* role, const char* content) override;
};

#endif // OLED_DISPLAY_H
