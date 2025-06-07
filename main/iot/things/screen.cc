#include "iot/thing.h"
#include "board.h"
#include "display/lcd_display.h"
#include "settings.h"

#include <esp_log.h>
#include <string>

#define TAG "Screen"

namespace iot {

// ここではScreenの属性とメソッドのみを定義し、具体的な実装は含まない
class Screen : public Thing {
public:
    Screen() : Thing("Screen", "A screen that can set theme and brightness") {
        // デバイスのプロパティを定義
        properties_.AddStringProperty("theme", "Current theme", [this]() -> std::string {
            auto theme = Board::GetInstance().GetDisplay()->GetTheme();
            return theme;
        });

        properties_.AddNumberProperty("brightness", "Current brightness percentage", [this]() -> int {
            // ここに現在の明るさを取得するロジックを追加できる
            auto backlight = Board::GetInstance().GetBacklight();
            return backlight ? backlight->brightness() : 100;
        });

        // デバイスがリモートで実行できるコマンドを定義
        methods_.AddMethod("set_theme", "Set the screen theme", ParameterList({
            Parameter("theme_name", "Valid string values are 'light' and 'dark'", kValueTypeString, true)
        }), [this](const ParameterList& parameters) {
            std::string theme_name = static_cast<std::string>(parameters["theme_name"].string());
            auto display = Board::GetInstance().GetDisplay();
            if (display) {
                display->SetTheme(theme_name);
            }
        });
        
        methods_.AddMethod("set_brightness", "Set the brightness", ParameterList({
            Parameter("brightness", "An integer between 0 and 100", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            uint8_t brightness = static_cast<uint8_t>(parameters["brightness"].number());
            auto backlight = Board::GetInstance().GetBacklight();
            if (backlight) {
                backlight->SetBrightness(brightness, true);
            }
        });
    }
};

} // namespace iot

DECLARE_THING(Screen);