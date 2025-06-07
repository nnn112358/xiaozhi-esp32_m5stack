#include "iot/thing.h"
#include "board.h"
#include "audio_codec.h"

#include <esp_log.h>

#define TAG "Speaker"

namespace iot {

// ここではSpeakerの属性とメソッドのみを定義し、具体的な実装は含まない
class Speaker : public Thing {
public:
    Speaker() : Thing("AudioSpeaker", "The audio speaker of the device") {
        // デバイスのプロパティを定義
        properties_.AddNumberProperty("volume", "Current audio volume value", [this]() -> int {
            auto codec = Board::GetInstance().GetAudioCodec();
            return codec->output_volume();
        });

        // デバイスがリモートで実行できるコマンドを定義
        methods_.AddMethod("set_volume", "Set the audio volume", ParameterList({
            Parameter("volume", "An integer between 0 and 100", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            auto codec = Board::GetInstance().GetAudioCodec();
            codec->SetOutputVolume(static_cast<uint8_t>(parameters["volume"].number()));
        });
    }
};

} // namespace iot

DECLARE_THING(Speaker);
