/**
 * @file display.h
 * @brief ディスプレイ制御の基底クラス
 * 
 * LVGLライブラリを使用したLCD/OLEDディスプレイの統一インターフェース。
 * ステータス表示、通知、チャットメッセージ、アイコン表示などの
 * UI機能を提供します。
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <lvgl.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_pm.h>

#include <string>

/**
 * @struct DisplayFonts
 * @brief ディスプレイで使用するフォント群
 */
struct DisplayFonts {
    const lv_font_t* text_font = nullptr;  // テキスト用フォント
    const lv_font_t* icon_font = nullptr;  // アイコン用フォント
    const lv_font_t* emoji_font = nullptr; // 絵文字用フォント
};

/**
 * @class Display
 * @brief ディスプレイ制御の基底クラス
 * 
 * LVGLを使用してLCDやOLEDディスプレイを制御します。
 * ステータスバー、通知、アイコン、チャットメッセージなどの
 * UI要素を管理します。
 */
class Display {
public:
    Display();
    virtual ~Display();

    /** ステータステキストを設定 */
    virtual void SetStatus(const char* status);
    
    /** 通知メッセージを表示（指定時間後に消える） */
    virtual void ShowNotification(const char* notification, int duration_ms = 3000);
    virtual void ShowNotification(const std::string &notification, int duration_ms = 3000);
    
    /** 感情表現を設定 */
    virtual void SetEmotion(const char* emotion);
    
    /** チャットメッセージを表示 */
    virtual void SetChatMessage(const char* role, const char* content);
    
    /** アイコンを設定 */
    virtual void SetIcon(const char* icon);
    
    /** プレビュー画像を設定 */
    virtual void SetPreviewImage(const lv_img_dsc_t* image);
    
    /** テーマ（ライト/ダーク）を設定 */
    virtual void SetTheme(const std::string& theme_name);
    
    /** 現在のテーマを取得 */
    virtual std::string GetTheme() { return current_theme_name_; }
    
    /** ステータスバーを更新 */
    virtual void UpdateStatusBar(bool update_all = false);

    inline int width() const { return width_; }
    inline int height() const { return height_; }

protected:
    int width_ = 0;
    int height_ = 0;
    
    esp_pm_lock_handle_t pm_lock_ = nullptr;
    lv_display_t *display_ = nullptr;

    lv_obj_t *emotion_label_ = nullptr;
    lv_obj_t *network_label_ = nullptr;
    lv_obj_t *status_label_ = nullptr;
    lv_obj_t *notification_label_ = nullptr;
    lv_obj_t *mute_label_ = nullptr;
    lv_obj_t *battery_label_ = nullptr;
    lv_obj_t* chat_message_label_ = nullptr;
    lv_obj_t* low_battery_popup_ = nullptr;
    lv_obj_t* low_battery_label_ = nullptr;
    
    const char* battery_icon_ = nullptr;
    const char* network_icon_ = nullptr;
    bool muted_ = false;
    std::string current_theme_name_;

    esp_timer_handle_t notification_timer_ = nullptr;

    friend class DisplayLockGuard;
    virtual bool Lock(int timeout_ms = 0) = 0;
    virtual void Unlock() = 0;
};


class DisplayLockGuard {
public:
    DisplayLockGuard(Display *display) : display_(display) {
        if (!display_->Lock(30000)) {
            ESP_LOGE("Display", "Failed to lock display");
        }
    }
    ~DisplayLockGuard() {
        display_->Unlock();
    }

private:
    Display *display_;
};

class NoDisplay : public Display {
private:
    virtual bool Lock(int timeout_ms = 0) override {
        return true;
    }
    virtual void Unlock() override {}
};

#endif
