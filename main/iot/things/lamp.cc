#include "iot/thing.h"
#include "board.h"
#include "audio_codec.h"

#include <driver/gpio.h>
#include <esp_log.h>

#define TAG "Lamp"

namespace iot {

// ここではLampの属性とメソッドのみを定義し、具体的な実装は含まない
class Lamp : public Thing {
private:
#ifdef CONFIG_IDF_TARGET_ESP32
    gpio_num_t gpio_num_ = GPIO_NUM_12;
#else
    gpio_num_t gpio_num_ = GPIO_NUM_18;
#endif
    bool power_ = false;

    void InitializeGpio() {
        gpio_config_t config = {
            .pin_bit_mask = (1ULL << gpio_num_),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&config));
        gpio_set_level(gpio_num_, 0);
    }

public:
    Lamp() : Thing("Lamp", "A test lamp"), power_(false) {
        InitializeGpio();

        // デバイスのプロパティを定義
        properties_.AddBooleanProperty("power", "Whether the lamp is on", [this]() -> bool {
            return power_;
        });

        // デバイスがリモートで実行できるコマンドを定義
        methods_.AddMethod("turn_on", "Turn on the lamp", ParameterList(), [this](const ParameterList& parameters) {
            power_ = true;
            gpio_set_level(gpio_num_, 1);
        });

        methods_.AddMethod("turn_off", "Turn off the lamp", ParameterList(), [this](const ParameterList& parameters) {
            power_ = false;
            gpio_set_level(gpio_num_, 0);
        });
    }
};

} // namespace iot

DECLARE_THING(Lamp);
