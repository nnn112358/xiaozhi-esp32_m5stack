/**
 * @file dual_network_board.cc
 * @brief デュアルネットワークボードクラスの実装
 * 
 * WiFi/4G両対応ボードの実装を提供します。
 * ユーザー設定に基づいてWiFiまたは4G(ML307)ネットワークを選択し、
 * 実行時に切り替えることができます。設定は不揮発性ストレージに保存されます。
 */

#include "dual_network_board.h"
#include "application.h"
#include "display.h"
#include "assets/lang_config.h"
#include "settings.h"
#include <esp_log.h>

static const char *TAG = "DualNetworkBoard";

/**
 * @brief DualNetworkBoardクラスのコンストラクタ
 * @param ml307_tx_pin ML307モデム用UART送信ピン
 * @param ml307_rx_pin ML307モデム用UART受信ピン
 * @param ml307_rx_buffer_size ML307用UART受信バッファサイズ
 * @param default_net_type デフォルトのネットワークタイプ（0:WiFi、1:ML307）
 * 
 * デュアルネットワークボードを初期化します。設定から現在のネットワークタイプを
 * 読み込み、対応するボードクラス（WifiBoardまたはMl307Board）を作成します。
 */
DualNetworkBoard::DualNetworkBoard(gpio_num_t ml307_tx_pin, gpio_num_t ml307_rx_pin, size_t ml307_rx_buffer_size, int32_t default_net_type) 
    : Board(), 
      ml307_tx_pin_(ml307_tx_pin), 
      ml307_rx_pin_(ml307_rx_pin), 
      ml307_rx_buffer_size_(ml307_rx_buffer_size) {
    
    // 設定からネットワークタイプを読み込み
    network_type_ = LoadNetworkTypeFromSettings(default_net_type);
    
    // 現在のネットワークタイプに対応するボードのみ初期化
    InitializeCurrentBoard();
}

/**
 * @brief 設定からネットワークタイプを読み込み
 * @param default_net_type デフォルトのネットワークタイプ
 * @return NetworkType 読み込まれたネットワークタイプ
 * 
 * 不揮発性ストレージ（NVS）からネットワークタイプ設定を読み込みます。
 * 設定が存在しない場合はデフォルト値を使用します。
 */
NetworkType DualNetworkBoard::LoadNetworkTypeFromSettings(int32_t default_net_type) {
    Settings settings("network", true);  // 自動保存有効でSettingsを作成
    int network_type = settings.GetInt("type", default_net_type);  // デフォルトはML307 (1)
    return network_type == 1 ? NetworkType::ML307 : NetworkType::WIFI;
}

/**
 * @brief ネットワークタイプを設定に保存
 * @param type 保存するネットワークタイプ
 * 
 * 選択されたネットワークタイプを不揮発性ストレージ（NVS）に保存します。
 * 次回起動時にこの設定が使用されます。
 */
void DualNetworkBoard::SaveNetworkTypeToSettings(NetworkType type) {
    Settings settings("network", true);  // 自動保存有効でSettingsを作成
    int network_type = (type == NetworkType::ML307) ? 1 : 0;  // ML307: 1, WiFi: 0
    settings.SetInt("type", network_type);
}

/**
 * @brief 現在のネットワークタイプに対応するボードを初期化
 * 
 * network_type_の値に基づいて、適切なボードクラス（Ml307BoardまたはWifiBoard）を
 * 作成します。unique_ptrを使用してメモリ管理を自動化しています。
 */
void DualNetworkBoard::InitializeCurrentBoard() {
    if (network_type_ == NetworkType::ML307) {
        ESP_LOGI(TAG, "Initialize ML307 board");  // 4G/LTEモデムボードを初期化
        current_board_ = std::make_unique<Ml307Board>(ml307_tx_pin_, ml307_rx_pin_, ml307_rx_buffer_size_);
    } else {
        ESP_LOGI(TAG, "Initialize WiFi board");   // WiFiボードを初期化
        current_board_ = std::make_unique<WifiBoard>();
    }
}

/**
 * @brief ネットワークタイプの切り替え
 * 
 * 現在のネットワークタイプを反対のタイプに切り替えます。
 * 設定を保存し、ユーザーに通知した後、デバイスを再起動して変更を適用します。
 * 再起動により、新しいネットワークタイプに対応するボードが初期化されます。
 */
void DualNetworkBoard::SwitchNetworkType() {
    auto display = GetDisplay();
    
    // 現在のネットワークタイプに応じて切り替え先を決定
    if (network_type_ == NetworkType::WIFI) {    
        // WiFiから4Gに切り替え
        SaveNetworkTypeToSettings(NetworkType::ML307);
        display->ShowNotification(Lang::Strings::SWITCH_TO_4G_NETWORK);  // "4Gネットワークに切り替えます"
    } else {
        // 4GからWiFiに切り替え
        SaveNetworkTypeToSettings(NetworkType::WIFI);
        display->ShowNotification(Lang::Strings::SWITCH_TO_WIFI_NETWORK); // "WiFiネットワークに切り替えます"
    }
    
    // ユーザーが通知を読む時間を確保
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // デバイスを再起動して新しい設定を適用
    auto& app = Application::GetInstance();
    app.Reboot();
}

/**
 * @brief ボードタイプ名の取得
 * @return std::string 現在アクティブなボードのタイプ名
 * 
 * 現在初期化されているボード（WiFiまたはML307）のタイプ名を返します。
 */
std::string DualNetworkBoard::GetBoardType() {
    return current_board_->GetBoardType();
}

/**
 * @brief ネットワーク接続の開始
 * 
 * 現在のネットワークタイプに応じて適切な状態メッセージを表示し、
 * 実際のネットワーク接続処理を委託します。
 */
void DualNetworkBoard::StartNetwork() {
    auto display = Board::GetInstance().GetDisplay();
    
    // ネットワークタイプに応じた状態メッセージを表示
    if (network_type_ == NetworkType::WIFI) {
        display->SetStatus(Lang::Strings::CONNECTING);        // "接続中"
    } else {
        display->SetStatus(Lang::Strings::DETECTING_MODULE);  // "モジュール検出中"
    }
    
    // 実際のネットワーク接続処理を委託
    current_board_->StartNetwork();
}

/**
 * @brief HTTPクライアントの作成
 * @return Http* 現在のネットワークタイプ用HTTPクライアント
 */
Http* DualNetworkBoard::CreateHttp() {
    return current_board_->CreateHttp();
}

/**
 * @brief WebSocketクライアントの作成
 * @return WebSocket* 現在のネットワークタイプ用WebSocketクライアント
 */
WebSocket* DualNetworkBoard::CreateWebSocket() {
    return current_board_->CreateWebSocket();
}

/**
 * @brief MQTTクライアントの作成
 * @return Mqtt* 現在のネットワークタイプ用MQTTクライアント
 */
Mqtt* DualNetworkBoard::CreateMqtt() {
    return current_board_->CreateMqtt();
}

/**
 * @brief UDPクライアントの作成
 * @return Udp* 現在のネットワークタイプ用UDPクライアント
 */
Udp* DualNetworkBoard::CreateUdp() {
    return current_board_->CreateUdp();
}

/**
 * @brief ネットワーク状態アイコンの取得
 * @return const char* ディスプレイ用アイコン文字列
 */
const char* DualNetworkBoard::GetNetworkStateIcon() {
    return current_board_->GetNetworkStateIcon();
}

/**
 * @brief 省電力モードの設定
 * @param enabled 省電力モードの有効/無効
 */
void DualNetworkBoard::SetPowerSaveMode(bool enabled) {
    current_board_->SetPowerSaveMode(enabled);
}

/**
 * @brief ボード情報JSONの取得
 * @return std::string ボード情報を含むJSON文字列
 */
std::string DualNetworkBoard::GetBoardJson() {   
    return current_board_->GetBoardJson();
}

/**
 * @brief デバイス状態JSONの取得
 * @return std::string デバイス状態を含むJSON文字列
 */
std::string DualNetworkBoard::GetDeviceStatusJson() {
    return current_board_->GetDeviceStatusJson();
}
