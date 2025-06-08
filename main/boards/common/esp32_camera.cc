/**
 * @file esp32_camera.cc
 * @brief ESP32カメラクラスの実装
 * 
 * ESP32内蔵カメラセンサーを使用した画像キャプチャとAI解析機能を実装します。
 * カメラ初期化、画像キャプチャ、LVGL用プレビュー画像生成、リモートAI解析を提供します。
 * メモリ効率化のため、JPEG エンコーディングには独立スレッドとキューを使用します。
 */

#include "esp32_camera.h"
#include "mcp_server.h"
#include "display.h"
#include "board.h"
#include "system_info.h"

#include <esp_log.h>
#include <esp_heap_caps.h>
#include <img_converters.h>
#include <cstring>

#define TAG "Esp32Camera"

/**
 * @brief Esp32Cameraクラスのコンストラクタ
 * @param config カメラの設定構造体（ピン、解像度、フォーマット等）
 * 
 * ESP32カメラモジュールを初期化し、プレビュー画像用メモリを確保します。
 * GC0308センサーでは、ミラー設定を自動調整します。
 * LVGL用のRGB565フォーマットプレビュー画像バッファをPSRAMに作成します。
 */
Esp32Camera::Esp32Camera(const camera_config_t& config) {
    // カメラモジュールの初期化
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return;
    }

    // カメラセンサーの設定取得と調整
    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == GC0308_PID) {
        // GC0308センサー専用設定（水平ミラーを無効化）
        s->set_hmirror(s, 0);  // 0=ミラーなし、1=ミラーあり
    }

    // LVGL用プレビュー画像の初期化
    memset(&preview_image_, 0, sizeof(preview_image_));
    preview_image_.header.magic = LV_IMAGE_HEADER_MAGIC;                           // LVGLマジックナンバー
    preview_image_.header.cf = LV_COLOR_FORMAT_RGB565;                             // 16ビットRGBフォーマット
    preview_image_.header.flags = LV_IMAGE_FLAGS_ALLOCATED | LV_IMAGE_FLAGS_MODIFIABLE; // メモリ確保済み・変更可能

    // フレームサイズに応じた解像度設定
    if (config.frame_size == FRAMESIZE_VGA) {
        preview_image_.header.w = 640;   // VGA: 640x480
        preview_image_.header.h = 480;
    } else if (config.frame_size == FRAMESIZE_QVGA) {
        preview_image_.header.w = 320;   // QVGA: 320x240
        preview_image_.header.h = 240;
    }
    
    // 画像メモリの計算と確保
    preview_image_.header.stride = preview_image_.header.w * 2;  // 1ピクセル=2バイト（RGB565）
    preview_image_.data_size = preview_image_.header.w * preview_image_.header.h * 2;
    
    // PSRAMにプレビュー用メモリを確保（高速アクセス用）
    preview_image_.data = (uint8_t*)heap_caps_malloc(preview_image_.data_size, MALLOC_CAP_SPIRAM);
    if (preview_image_.data == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate memory for preview image");
        return;
    }
}

/**
 * @brief Esp32Cameraクラスのデストラクタ
 * 
 * カメラリソースを適切に解放します。フレームバッファ、プレビュー画像メモリ、
 * カメラドライバの上から順番にクリーンアップします。
 */
Esp32Camera::~Esp32Camera() {
    // カメラフレームバッファの解放
    if (fb_) {
        esp_camera_fb_return(fb_);  // ESP32カメラライブラリに返却
        fb_ = nullptr;
    }
    
    // プレビュー画像用メモリの解放
    if (preview_image_.data) {
        heap_caps_free((void*)preview_image_.data);  // PSRAMメモリを解放
        preview_image_.data = nullptr;
    }
    
    // カメラドライバの終了処理
    esp_camera_deinit();
}

/**
 * @brief AI画像解析サービスのURLと認証トークンを設定
 * @param url AI画像解析サービスのURL
 * @param token 認証用Bearerトークン（空文字列で認証なし）
 * 
 * Explain()メソッドで使用するリモートAIサービスの接続情報を設定します。
 * サービスは画像と質問を受け取り、AIによる画像解析結果を返します。
 */
void Esp32Camera::SetExplainUrl(const std::string& url, const std::string& token) {
    explain_url_ = url;     // AIサービスのエンドポイントURL
    explain_token_ = token; // Bearer認証トークン
}

/**
 * @brief カメラ画像のキャプチャとプレビュー表示
 * @return bool キャプチャ成功時true、失敗時false
 * 
 * カメラから安定したフレームを取得し、LVGLディスプレイ用のプレビュー画像を作成します。
 * エンコーダースレッドの終了を待ち、複数フレームを取得して安定性を確保します。
 * RGB565フォーマットのバイトオーダーをLVGL用に変換して表示します。
 */
bool Esp32Camera::Capture() {
    // 前回のエンコーダースレッドの終了を待機
    if (encoder_thread_.joinable()) {
        encoder_thread_.join();
    }

    // 安定したフレームを取得するために複数回キャプチャ
    int frames_to_get = 2;
    for (int i = 0; i < frames_to_get; i++) {
        // 前回のフレームバッファを解放
        if (fb_ != nullptr) {
            esp_camera_fb_return(fb_);
        }
        
        // 新しいフレームを取得
        fb_ = esp_camera_fb_get();
        if (fb_ == nullptr) {
            ESP_LOGE(TAG, "Camera capture failed");
            return false;
        }
    }

    // ディスプレイにプレビュー画像を表示
    auto display = Board::GetInstance().GetDisplay();
    if (display != nullptr) {
        auto src = (uint16_t*)fb_->buf;            // カメラフレームデータ（RGB565）
        auto dst = (uint16_t*)preview_image_.data; // プレビュー画像バッファ
        size_t pixel_count = fb_->len / 2;         // 16ビットピクセル数
        
        // バイトオーダー変換（カメラ→LVGLフォーマット）
        for (size_t i = 0; i < pixel_count; i++) {
            // 16ビット内のバイト順序を入れ替え（エンディアン変換）
            dst[i] = __builtin_bswap16(src[i]);
        }
        
        // LVGLディスプレイにプレビュー画像を設定
        display->SetPreviewImage(&preview_image_);
    }
    return true;
}
/**
 * @brief 水平ミラー（左右反転）の設定
 * @param enabled trueでミラー有効、falseで無効
 * @return bool 設定成功時true、失敗時false
 * 
 * カメラセンサーの水平ミラー機能を制御します。
 * ユーザーがセルフィーを撮影する場合など、自然な方向で表示するために使用します。
 */
bool Esp32Camera::SetHMirror(bool enabled) {
    // カメラセンサーハンドルを取得
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return false;
    }
    
    // 水平ミラー設定を適用
    esp_err_t err = s->set_hmirror(s, enabled);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set horizontal mirror: %d", err);
        return false;
    }
    
    ESP_LOGI(TAG, "Camera horizontal mirror set to: %s", enabled ? "enabled" : "disabled");
    return true;
}

/**
 * @brief 垂直反転（上下反転）の設定
 * @param enabled trueで反転有効、falseで無効
 * @return bool 設定成功時true、失敗時false
 * 
 * カメラセンサーの垂直反転機能を制御します。
 * カメラモジュールの取り付け向きやユーザーの好みに合わせて調整できます。
 */
bool Esp32Camera::SetVFlip(bool enabled) {
    // カメラセンサーハンドルを取得
    sensor_t *s = esp_camera_sensor_get();
    if (s == nullptr) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        return false;
    }
    
    // 垂直反転設定を適用
    esp_err_t err = s->set_vflip(s, enabled);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set vertical flip: %d", err);
        return false;
    }
    
    ESP_LOGI(TAG, "Camera vertical flip set to: %s", enabled ? "enabled" : "disabled");
    return true;
}

/**
 * @brief 将摄像头捕获的图像发送到远程服务器进行AI分析和解释
 * 
 * 该函数将当前摄像头缓冲区中的图像编码为JPEG格式，并通过HTTP POST请求
 * 以multipart/form-data的形式发送到指定的解释服务器。服务器将根据提供的
 * 问题对图像进行AI分析并返回结果。
 * 
 * 实现特点：
 * - 使用独立线程编码JPEG，与主线程分离
 * - 采用分块传输编码(chunked transfer encoding)优化内存使用
 * - 通过队列机制实现编码线程和发送线程的数据同步
 * - 支持设备ID、客户端ID和认证令牌的HTTP头部配置
 * 
 * @param question 要向AI提出的关于图像的问题，将作为表单字段发送
 * @return std::string 服务器返回的JSON格式响应字符串
 *         成功时包含AI分析结果，失败时包含错误信息
 *         格式示例：{"success": true, "result": "分析结果"}
 *                  {"success": false, "message": "错误信息"}
 * 
 * @note 调用此函数前必须先调用SetExplainUrl()设置服务器URL
 * @note 函数会等待之前的编码线程完成后再开始新的处理
 * @warning 如果摄像头缓冲区为空或网络连接失败，将返回错误信息
 */
std::string Esp32Camera::Explain(const std::string& question) {
    if (explain_url_.empty()) {
        return "{\"success\": false, \"message\": \"Image explain URL or token is not set\"}";
    }

    // 创建局部的 JPEG 队列, 40 entries is about to store 512 * 40 = 20480 bytes of JPEG data
    QueueHandle_t jpeg_queue = xQueueCreate(40, sizeof(JpegChunk));
    if (jpeg_queue == nullptr) {
        ESP_LOGE(TAG, "Failed to create JPEG queue");
        return "{\"success\": false, \"message\": \"Failed to create JPEG queue\"}";
    }

    // We spawn a thread to encode the image to JPEG
    encoder_thread_ = std::thread([this, jpeg_queue]() {
        frame2jpg_cb(fb_, 80, [](void* arg, size_t index, const void* data, size_t len) -> unsigned int {
            auto jpeg_queue = (QueueHandle_t)arg;
            JpegChunk chunk = {
                .data = (uint8_t*)heap_caps_malloc(len, MALLOC_CAP_SPIRAM),
                .len = len
            };
            memcpy(chunk.data, data, len);
            xQueueSend(jpeg_queue, &chunk, portMAX_DELAY);
            return len;
        }, jpeg_queue);
    });

    auto http = Board::GetInstance().CreateHttp();
    // 构造multipart/form-data请求体
    std::string boundary = "----ESP32_CAMERA_BOUNDARY";
    
    // 构造question字段
    std::string question_field;
    question_field += "--" + boundary + "\r\n";
    question_field += "Content-Disposition: form-data; name=\"question\"\r\n";
    question_field += "\r\n";
    question_field += question + "\r\n";
    
    // 构造文件字段头部
    std::string file_header;
    file_header += "--" + boundary + "\r\n";
    file_header += "Content-Disposition: form-data; name=\"file\"; filename=\"camera.jpg\"\r\n";
    file_header += "Content-Type: image/jpeg\r\n";
    file_header += "\r\n";
    
    // 构造尾部
    std::string multipart_footer;
    multipart_footer += "\r\n--" + boundary + "--\r\n";

    // 配置HTTP客户端，使用分块传输编码
    http->SetHeader("Device-Id", SystemInfo::GetMacAddress().c_str());
    http->SetHeader("Client-Id", Board::GetInstance().GetUuid().c_str());
    if (!explain_token_.empty()) {
        http->SetHeader("Authorization", "Bearer " + explain_token_);
    }
    http->SetHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
    http->SetHeader("Transfer-Encoding", "chunked");
    if (!http->Open("POST", explain_url_)) {
        ESP_LOGE(TAG, "Failed to connect to explain URL");
        // Clear the queue
        encoder_thread_.join();
        JpegChunk chunk;
        while (xQueueReceive(jpeg_queue, &chunk, portMAX_DELAY) == pdPASS) {
            if (chunk.data != nullptr) {
                heap_caps_free(chunk.data);
            } else {
                break;
            }
        }
        vQueueDelete(jpeg_queue);
        return "{\"success\": false, \"message\": \"Failed to connect to explain URL\"}";
    }
    
    // 第一块：question字段
    http->Write(question_field.c_str(), question_field.size());
    
    // 第二块：文件字段头部
    http->Write(file_header.c_str(), file_header.size());
    
    // 第三块：JPEG数据
    size_t total_sent = 0;
    while (true) {
        JpegChunk chunk;
        if (xQueueReceive(jpeg_queue, &chunk, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "Failed to receive JPEG chunk");
            break;
        }
        if (chunk.data == nullptr) {
            break; // The last chunk
        }
        http->Write((const char*)chunk.data, chunk.len);
        total_sent += chunk.len;
        heap_caps_free(chunk.data);
    }
    // Wait for the encoder thread to finish
    encoder_thread_.join();
    // 清理队列
    vQueueDelete(jpeg_queue);

    // 第四块：multipart尾部
    http->Write(multipart_footer.c_str(), multipart_footer.size());
    
    // 结束块
    http->Write("", 0);

    if (http->GetStatusCode() != 200) {
        ESP_LOGE(TAG, "Failed to upload photo, status code: %d", http->GetStatusCode());
        return "{\"success\": false, \"message\": \"Failed to upload photo\"}";
    }

    std::string result = http->ReadAll();
    http->Close();

    ESP_LOGI(TAG, "Explain image size=%dx%d, compressed size=%d, question=%s\n%s", fb_->width, fb_->height, total_sent, question.c_str(), result.c_str());
    return result;
}
