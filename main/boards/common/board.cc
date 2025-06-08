/**
 * @file board.cc
 * @brief ハードウェアボード抽象化クラスの実装
 * 
 * すべてのESP32ボードに共通する基本機能を実装します。
 * デバイス識別、システム情報取得、JSON生成などの機能を提供します。
 */

#include "board.h"
#include "system_info.h"
#include "settings.h"
#include "display/display.h"
#include "assets/lang_config.h"

#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_chip_info.h>
#include <esp_random.h>

#define TAG "Board"

/**
 * @brief Boardクラスのコンストラクタ
 * 
 * ボードの基本初期化を行います。デバイス固有のUUIDを生成または復元し、
 * ボード識別情報をログに出力します。UUIDはデバイスの固有識別子として
 * OTAアップデートやクラウド連携で使用されます。
 */
Board::Board() {
    // ボード設定の読み込み（自動保存有効）
    Settings settings("board", true);
    
    // 既存のUUIDを取得
    uuid_ = settings.GetString("uuid");
    
    // UUIDが存在しない場合は新規生成
    if (uuid_.empty()) {
        uuid_ = GenerateUuid();
        settings.SetString("uuid", uuid_);
    }
    
    // ボード情報をログ出力
    ESP_LOGI(TAG, "UUID=%s SKU=%s", uuid_.c_str(), BOARD_NAME);
}

/**
 * @brief UUID v4の生成
 * @return std::string 生成されたUUID文字列（例: "550e8400-e29b-41d4-a716-446655440000"）
 * 
 * RFC 4122に準拠したUUID version 4を生成します。ESP32のハードウェア
 * 乱数生成器を使用して暗号学的に安全な乱数を生成し、UUIDの標準形式に
 * 変換します。生成されたUUIDはデバイスの一意識別子として使用されます。
 */
std::string Board::GenerateUuid() {
    // UUID v4は16バイトのランダムデータが必要
    uint8_t uuid[16];
    
    // ESP32のハードウェア乱数生成器を使用
    // セキュアで予測不可能な乱数を生成
    esp_fill_random(uuid, sizeof(uuid));
    
    // UUID v4の規格に従ってバージョンとバリアントビットを設定
    uuid[6] = (uuid[6] & 0x0F) | 0x40;    // バージョン4を設定（上位4ビット = 0100）
    uuid[8] = (uuid[8] & 0x3F) | 0x80;    // バリアント1を設定（上位2ビット = 10）
    
    // 16進数文字列を標準のUUID形式に変換
    // 形式: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    char uuid_str[37];  // 32文字 + 4ハイフン + NULL終端
    snprintf(uuid_str, sizeof(uuid_str),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        uuid[0], uuid[1], uuid[2], uuid[3],      // 時刻下位
        uuid[4], uuid[5],                        // 時刻中位
        uuid[6], uuid[7],                        // 時刻上位 + バージョン
        uuid[8], uuid[9],                        // クロックシーケンス + バリアント
        uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]); // ノード
    
    return std::string(uuid_str);
}

/**
 * @brief バッテリー情報の取得（基底実装）
 * @param level バッテリー残量（0-100%）の出力先
 * @param charging 充電中フラグの出力先
 * @param discharging 放電中フラグの出力先
 * @return bool 取得成功時true、失敗時false
 * 
 * 基底クラスではバッテリー機能をサポートしないため常にfalseを返します。
 * バッテリー搭載ボードでは派生クラスでオーバーライドして実装します。
 */
bool Board::GetBatteryLevel(int &level, bool& charging, bool& discharging) {
    return false;  // バッテリー非対応
}

/**
 * @brief ESP32チップ温度の取得（基底実装）
 * @param esp32temp チップ温度（摂氏）の出力先
 * @return bool 取得成功時true、失敗時false
 * 
 * 基底クラスでは温度センサー機能をサポートしないため常にfalseを返します。
 * 温度センサー搭載ボードでは派生クラスでオーバーライドして実装します。
 */
bool Board::GetTemperature(float& esp32temp){
    return false;  // 温度センサー非対応
}

/**
 * @brief ディスプレイインスタンスの取得
 * @return Display* ディスプレイオブジェクトへのポインタ
 * 
 * 基底クラスでは何も表示しないNoDisplayを返します。
 * 実際のディスプレイ搭載ボードでは派生クラスでオーバーライドします。
 */
Display* Board::GetDisplay() {
    static NoDisplay display;  // ダミーディスプレイ（何も表示しない）
    return &display;
}

/**
 * @brief カメラインスタンスの取得
 * @return Camera* カメラオブジェクトへのポインタ（nullptrで非対応）
 * 
 * 基底クラスではカメラ機能をサポートしないためnullptrを返します。
 * カメラ搭載ボードでは派生クラスでオーバーライドして実装します。
 */
Camera* Board::GetCamera() {
    return nullptr;  // カメラ非対応
}

/**
 * @brief LEDインスタンスの取得
 * @return Led* LEDオブジェクトへのポインタ
 * 
 * 基底クラスでは何も点灯しないNoLedを返します。
 * 実際のLED搭載ボードでは派生クラスでオーバーライドします。
 */
Led* Board::GetLed() {
    static NoLed led;  // ダミーLED（何も点灯しない）
    return &led;
}

/**
 * @brief システム全体の情報をJSON形式で取得
 * @return std::string システム情報を含むJSON文字列
 * 
 * デバイスの詳細情報をJSON形式で返します。OTAアップデート、
 * デバッグ、システム監視で使用されます。含まれる情報：
 * - システムバージョンと言語設定
 * - フラッシュサイズとメモリ情報
 * - MACアドレスとデバイスUUID
 * - チップ情報（モデル、コア数、リビジョン、機能）
 * - アプリケーション情報（名前、バージョン、コンパイル時間、ESP-IDFバージョン、ELF SHA256）
 * - パーティションテーブル（すべてのパーティションの詳細）
 * - OTA情報（現在実行中のパーティション）
 * - ボード固有情報
 */
std::string Board::GetJson() {
    /* 
     * 生成されるJSONの構造例:
     * {
     *     "version": 2,
     *     "language": "ja-JP",
     *     "flash_size": 4194304,
     *     "minimum_free_heap_size": 123456,
     *     "mac_address": "AA:BB:CC:DD:EE:FF",
     *     "uuid": "550e8400-e29b-41d4-a716-446655440000",
     *     "chip_model_name": "esp32s3",
     *     "chip_info": {
     *         "model": 9,
     *         "cores": 2,
     *         "revision": 0,
     *         "features": 50
     *     },
     *     "application": {
     *         "name": "xiaozhi-esp32",
     *         "version": "1.0.0",
     *         "compile_time": "2024-01-01T12:00:00Z",
     *         "idf_version": "5.3.0",
     *         "elf_sha256": "abcd1234..."
     *     },
     *     "partition_table": [
     *         {
     *             "label": "nvs",
     *             "type": 1,
     *             "subtype": 2,
     *             "address": 36864,
     *             "size": 24576
     *         }
     *     ],
     *     "ota": {
     *         "label": "app"
     *     },
     *     "board": {
     *         // ボード固有情報
     *     }
     * }
     */
    
    std::string json = "{";
    
    // システム基本情報
    json += "\"version\":2,";  // JSONスキーマバージョン
    json += "\"language\":\"" + std::string(Lang::CODE) + "\",";  // 現在の言語設定
    json += "\"flash_size\":" + std::to_string(SystemInfo::GetFlashSize()) + ",";  // フラッシュサイズ（バイト）
    json += "\"minimum_free_heap_size\":" + std::to_string(SystemInfo::GetMinimumFreeHeapSize()) + ",";  // 最小空きヒープサイズ
    json += "\"mac_address\":\"" + SystemInfo::GetMacAddress() + "\",";  // MACアドレス
    json += "\"uuid\":\"" + uuid_ + "\",";  // デバイス固有UUID
    json += "\"chip_model_name\":\"" + SystemInfo::GetChipModelName() + "\",";  // チップモデル名
    
    // チップ詳細情報
    json += "\"chip_info\":{";
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    json += "\"model\":" + std::to_string(chip_info.model) + ",";      // チップモデル番号
    json += "\"cores\":" + std::to_string(chip_info.cores) + ",";      // CPUコア数
    json += "\"revision\":" + std::to_string(chip_info.revision) + ","; // チップリビジョン
    json += "\"features\":" + std::to_string(chip_info.features);      // 機能フラグ（WiFi、Bluetooth等）
    json += "},";

    // アプリケーション情報
    json += "\"application\":{";
    auto app_desc = esp_app_get_description();
    json += "\"name\":\"" + std::string(app_desc->project_name) + "\",";  // プロジェクト名
    json += "\"version\":\"" + std::string(app_desc->version) + "\",";     // アプリバージョン
    json += "\"compile_time\":\"" + std::string(app_desc->date) + "T" + std::string(app_desc->time) + "Z\",";  // コンパイル日時（ISO 8601形式）
    json += "\"idf_version\":\"" + std::string(app_desc->idf_ver) + "\","; // ESP-IDFバージョン

    // ELFファイルのSHA256ハッシュを16進文字列に変換
    char sha256_str[65];  // 32バイト * 2文字 + NULL終端
    for (int i = 0; i < 32; i++) {
        snprintf(sha256_str + i * 2, sizeof(sha256_str) - i * 2, "%02x", app_desc->app_elf_sha256[i]);
    }
    json += "\"elf_sha256\":\"" + std::string(sha256_str) + "\"";  // ELFファイルのSHA256
    json += "},";

    // パーティションテーブル情報
    json += "\"partition_table\": [";
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it) {
        const esp_partition_t *partition = esp_partition_get(it);
        json += "{";
        json += "\"label\":\"" + std::string(partition->label) + "\",";       // パーティションラベル
        json += "\"type\":" + std::to_string(partition->type) + ",";          // パーティションタイプ
        json += "\"subtype\":" + std::to_string(partition->subtype) + ",";    // パーティションサブタイプ
        json += "\"address\":" + std::to_string(partition->address) + ",";    // 開始アドレス
        json += "\"size\":" + std::to_string(partition->size);               // サイズ（バイト）
        json += "},";
        it = esp_partition_next(it);
    }
    if (json.back() == ',') {
        json.pop_back(); // 最後のカンマを削除
    }
    json += "],";

    // OTA情報（現在実行中のパーティション）
    json += "\"ota\":{";
    auto ota_partition = esp_ota_get_running_partition();
    json += "\"label\":\"" + std::string(ota_partition->label) + "\"";  // 実行中のOTAパーティション
    json += "},";

    // ボード固有情報を追加
    json += "\"board\":" + GetBoardJson();

    // JSONオブジェクトを閉じる
    json += "}";
    return json;
}