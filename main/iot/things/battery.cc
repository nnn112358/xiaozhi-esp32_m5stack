#include "iot/thing.h"
#include "board.h"

#include <esp_log.h>

#define TAG "Battery"

namespace iot {

// ここではBatteryの属性とメソッドのみを定義し、具体的な実装は含まない
class Battery : public Thing {
private:
    int level_ = 0;
    bool charging_ = false;
    bool discharging_ = false;

public:
    Battery() : Thing("Battery", "The battery of the device") {
        // デバイスのプロパティを定義
        properties_.AddNumberProperty("level", "Current battery level", [this]() -> int {
            auto& board = Board::GetInstance();
            if (board.GetBatteryLevel(level_, charging_, discharging_)) {
                return level_;
            }
            return 0;
        });
        properties_.AddBooleanProperty("charging", "Whether the battery is charging", [this]() -> int {
            return charging_;
        });
    }
};

} // namespace iot

DECLARE_THING(Battery);