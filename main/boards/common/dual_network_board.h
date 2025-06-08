/**
 * @file dual_network_board.h
 * @brief デュアルネットワーク対応ボードの抽象化クラス
 * 
 * このファイルはWiFiと4G（ML307）の両方に対応し、動的に切り替え可能な
 * ネットワーク機能を提供します。ユーザーの設定や環境に応じて最適な
 * ネットワーク接続方式を選択できます。
 */
#ifndef DUAL_NETWORK_BOARD_H
#define DUAL_NETWORK_BOARD_H

#include "board.h"
#include "wifi_board.h"
#include "ml307_board.h"
#include <memory>

/**
 * @enum NetworkType
 * @brief ネットワーク接続方式の列挙型
 */
enum class NetworkType {
    WIFI,    /**< WiFi接続 */
    ML307    /**< ML307 4G/LTE接続 */
};

/**
 * @class DualNetworkBoard
 * @brief デュアルネットワーク対応ボードクラス
 * 
 * WiFiとML307 4Gモジュールの両方を搭載したボードで、
 * 実行時にネットワーク接続方式を切り替えることができます。
 * 設定は永続化され、再起動後も保持されます。
 */
class DualNetworkBoard : public Board {
private:
    /** @brief 現在アクティブなボードインスタンス（WiFiまたはML307） */
    std::unique_ptr<Board> current_board_;
    
    /** @brief 現在のネットワーク接続タイプ（デフォルト: ML307） */
    NetworkType network_type_ = NetworkType::ML307;

    /** @brief ML307モジュールのUART送信ピン */
    gpio_num_t ml307_tx_pin_;
    
    /** @brief ML307モジュールのUART受信ピン */
    gpio_num_t ml307_rx_pin_;
    
    /** @brief ML307モジュールのUART受信バッファサイズ */
    size_t ml307_rx_buffer_size_;
    
    /**
     * @brief 設定からネットワークタイプを読み込み
     * @param default_net_type デフォルトのネットワークタイプ
     * @return NetworkType 読み込まれたネットワークタイプ
     * 
     * NVS（不揮発性ストレージ）からネットワーク設定を読み込みます。
     * 設定が存在しない場合はデフォルト値を使用します。
     */
    NetworkType LoadNetworkTypeFromSettings(int32_t default_net_type);
    
    /**
     * @brief ネットワークタイプを設定に保存
     * @param type 保存するネットワークタイプ
     * 
     * 現在のネットワークタイプをNVSに永続化します。
     * 次回起動時に同じ設定が復元されます。
     */
    void SaveNetworkTypeToSettings(NetworkType type);

    /**
     * @brief 現在のネットワークタイプに対応するボードを初期化
     * 
     * network_type_の値に基づいて、WiFiボードまたはML307ボードの
     * インスタンスを作成し、current_board_に設定します。
     */
    void InitializeCurrentBoard();
 
public:
    /**
     * @brief デュアルネットワークボードのコンストラクタ
     * @param ml307_tx_pin ML307モジュールのUART送信ピン
     * @param ml307_rx_pin ML307モジュールのUART受信ピン  
     * @param ml307_rx_buffer_size ML307のUART受信バッファサイズ（デフォルト: 4096）
     * @param default_net_type デフォルトのネットワークタイプ（デフォルト: 1=ML307）
     * 
     * 設定からネットワークタイプを読み込み、対応するボードを初期化します。
     */
    DualNetworkBoard(gpio_num_t ml307_tx_pin, gpio_num_t ml307_rx_pin, size_t ml307_rx_buffer_size = 4096, int32_t default_net_type = 1);
    
    /**
     * @brief デストラクタ
     */
    virtual ~DualNetworkBoard() = default;
 
    /**
     * @brief ネットワークタイプの切り替え
     * 
     * WiFiとML307間でネットワーク接続方式を切り替えます。
     * 設定は自動的に保存され、対応するボードインスタンスが再作成されます。
     */
    void SwitchNetworkType();
    
    /**
     * @brief 現在のネットワークタイプ取得
     * @return NetworkType 現在アクティブなネットワークタイプ
     */
    NetworkType GetNetworkType() const { return network_type_; }
    
    /**
     * @brief 現在アクティブなボードインスタンスの取得
     * @return Board& 現在のボードインスタンスへの参照
     */
    Board& GetCurrentBoard() const { return *current_board_; }
    
    // Board インターフェースの実装（現在アクティブなボードに委譲）
    
    /**
     * @brief ボードタイプ名の取得
     * @return std::string 現在アクティブなボードのタイプ名
     */
    virtual std::string GetBoardType() override;
    
    /**
     * @brief ネットワーク接続開始
     * 
     * 現在選択されているネットワーク方式で接続を開始します。
     */
    virtual void StartNetwork() override;
    
    /**
     * @brief HTTP クライアント作成
     * @return Http* 現在のネットワーク経由のHTTP通信用クライアント
     */
    virtual Http* CreateHttp() override;
    
    /**
     * @brief WebSocket クライアント作成
     * @return WebSocket* 現在のネットワーク経由のWebSocket通信用クライアント
     */
    virtual WebSocket* CreateWebSocket() override;
    
    /**
     * @brief MQTT クライアント作成
     * @return Mqtt* 現在のネットワーク経由のMQTT通信用クライアント
     */
    virtual Mqtt* CreateMqtt() override;
    
    /**
     * @brief UDP クライアント作成
     * @return Udp* 現在のネットワーク経由のUDP通信用クライアント
     */
    virtual Udp* CreateUdp() override;
    
    /**
     * @brief ネットワーク状態アイコン取得
     * @return const char* 現在のネットワーク接続状態を表すアイコン文字
     */
    virtual const char* GetNetworkStateIcon() override;
    
    /**
     * @brief 省電力モード設定
     * @param enabled true: 省電力モード有効, false: 無効
     */
    virtual void SetPowerSaveMode(bool enabled) override;
    
    /**
     * @brief ボード情報のJSON取得
     * @return std::string 現在のボード設定情報を含むJSON文字列
     */
    virtual std::string GetBoardJson() override;
    
    /**
     * @brief デバイス状態のJSON取得
     * @return std::string 現在のデバイス状態を含むJSON文字列
     */
    virtual std::string GetDeviceStatusJson() override;
};

#endif // DUAL_NETWORK_BOARD_H 