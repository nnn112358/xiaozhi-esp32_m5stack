/**
 * @file lcd_display.h
 * @brief LCDディスプレイ制御クラス
 * 
 * LVGLライブラリを使用したLCDディスプレイの統一インターフェースです。
 * RGB、MIPI、SPI、QSPI、MCU8080などさまざまなインターフェースのLCDに対応し、
 * テーマ切り替え、チャットUI、アイコン表示などの機能を提供します。
 */
#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "display.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <font_emoji.h>

#include <atomic>

/**
 * @struct ThemeColors
 * @brief LCDディスプレイのテーマ色構造体
 * 
 * ライトテーマとダークテーマの切り替えに使用される色情報を定義します。
 */
struct ThemeColors {
    lv_color_t background;      /**< 背景色 */
    lv_color_t text;            /**< テキスト色 */
    lv_color_t chat_background; /**< チャット背景色 */
    lv_color_t user_bubble;     /**< ユーザーメッセージバブル色 */
    lv_color_t assistant_bubble; /**< アシスタントメッセージバブル色 */
    lv_color_t system_bubble;   /**< システムメッセージバブル色 */
    lv_color_t system_text;     /**< システムテキスト色 */
    lv_color_t border;          /**< 枠線色 */
    lv_color_t low_battery;     /**< バッテリー低下警告色 */
};


/**
 * @class LcdDisplay
 * @brief LCDディスプレイ制御の基底クラス
 * 
 * LVGLを使用してLCDディスプレイを制御します。
 * ステータスバー、チャットUI、アイコン表示、テーマ切り替えなどの
 * 機能を提供します。各種インターフェースのLCDの基底クラスです。
 */
class LcdDisplay : public Display {
protected:
    // ESP-IDF LCDパネルインターフェース
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;  /**< LCDパネルIOハンドル */
    esp_lcd_panel_handle_t panel_ = nullptr;        /**< LCDパネルハンドル */
    
    // LVGL描画バッファとUI要素
    lv_draw_buf_t draw_buf_;                        /**< LVGL描画バッファ */
    lv_obj_t* status_bar_ = nullptr;                /**< ステータスバーオブジェクト */
    lv_obj_t* content_ = nullptr;                   /**< メインコンテンツオブジェクト */
    lv_obj_t* container_ = nullptr;                 /**< コンテナオブジェクト */
    lv_obj_t* side_bar_ = nullptr;                  /**< サイドバーオブジェクト */
    lv_obj_t* preview_image_ = nullptr;             /**< プレビュー画像オブジェクト */

    // フォントとテーマ
    DisplayFonts fonts_;                            /**< 使用するフォント群 */
    ThemeColors current_theme_;                     /**< 現在のテーマ色 */

    /** UIレイアウトを設定 */
    void SetupUI();
    
    /** LVGLミューテックスをロック */
    virtual bool Lock(int timeout_ms = 0) override;
    
    /** LVGLミューテックスをアンロック */
    virtual void Unlock() override;

protected:
    /**
     * @brief LCDディスプレイ基底コンストラクタ（サブクラス用）
     * @param panel_io LCDパネルIOハンドル
     * @param panel LCDパネルハンドル
     * @param fonts 使用するフォント群
     * @param width ディスプレイ幅
     * @param height ディスプレイ高さ
     */
    LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, DisplayFonts fonts, int width, int height);
    
public:
    ~LcdDisplay();
    
    /** 感情アイコンを設定 */
    virtual void SetEmotion(const char* emotion) override;
    
    /** アイコンを設定 */
    virtual void SetIcon(const char* icon) override;
    
    /** プレビュー画像を設定 */
    virtual void SetPreviewImage(const lv_img_dsc_t* img_dsc) override;
    
#if CONFIG_USE_WECHAT_MESSAGE_STYLE
    /** チャットメッセージを表示（WeChatスタイル） */
    virtual void SetChatMessage(const char* role, const char* content) override; 
#endif  

    /** テーマ（ライト/ダーク）を切り替え */
    virtual void SetTheme(const std::string& theme_name) override;
};

/**
 * @class RgbLcdDisplay
 * @brief RGBインターフェースLCDディスプレイ
 * 
 * RGBパラレルインターフェースで接続されるLCDディスプレイを制御します。
 */
class RgbLcdDisplay : public LcdDisplay {
public:
    RgbLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
};

/**
 * @class MipiLcdDisplay  
 * @brief MIPI DSIインターフェースLCDディスプレイ
 * 
 * MIPI DSIインターフェースで接続される高解像LCDディスプレイを制御します。
 */
class MipiLcdDisplay : public LcdDisplay {
public:
    MipiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   DisplayFonts fonts);
};

/**
 * @class SpiLcdDisplay
 * @brief SPIインターフェースLCDディスプレイ
 * 
 * SPIインターフェースで接続される小型から中型LCDディスプレイを制御します。
 */
class SpiLcdDisplay : public LcdDisplay {
public:
    SpiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
};

/**
 * @class QspiLcdDisplay
 * @brief QSPIインターフェースLCDディスプレイ
 * 
 * QSPI（Quad SPI）インターフェースで接続される高速LCDディスプレイを制御します。
 */
class QspiLcdDisplay : public LcdDisplay {
public:
    QspiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   DisplayFonts fonts);
};

/**
 * @class Mcu8080LcdDisplay
 * @brief MCU 8080インターフェースLCDディスプレイ
 * 
 * 8080パラレルインターフェースで接続されるLCDディスプレイを制御します。
 */
class Mcu8080LcdDisplay : public LcdDisplay {
public:
    Mcu8080LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                      int width, int height, int offset_x, int offset_y,
                      bool mirror_x, bool mirror_y, bool swap_xy,
                      DisplayFonts fonts);
};
#endif // LCD_DISPLAY_H
