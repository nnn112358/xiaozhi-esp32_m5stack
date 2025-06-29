# WebSocket通信プロトコル仕様書

以下は、コード実装に基づいて整理されたWebSocket通信プロトコルドキュメントです。クライアント（デバイス）とサーバー間のWebSocket通信方法を概説しています。このドキュメントは提供されたコードから推定されたものであり、実際のデプロイ時にはサーバー側実装と合わせてさらなる確認や補完が必要な場合があります。

---

## 1. 全体フロー概要

1. **デバイス側初期化**  
   - デバイス起動、`Application`初期化：  
     - 音声コーデック、ディスプレイ、LEDなどを初期化  
     - ネットワーク接続  
     - `Protocol`インターフェースを実装するWebSocketプロトコルインスタンス（`WebsocketProtocol`）を作成・初期化  
   - メインループに入り、イベント（音声入力、音声出力、スケジュールタスクなど）を待機。

2. **WebSocket接続の確立**  
   - デバイスが音声セッションを開始する必要がある時（ユーザーのウェイクアップ、手動ボタントリガーなど）、`OpenAudioChannel()`を呼び出し：  
     - 設定からWebSocket URLを取得
     - 複数のリクエストヘッダー（`Authorization`, `Protocol-Version`, `Device-Id`, `Client-Id`）を設定  
     - `Connect()`を呼び出してサーバーとWebSocket接続を確立  

3. **クライアント "hello" メッセージの送信**  
   - 接続成功後、デバイスはJSONメッセージを送信、例：  
   ```json
   {
     "type": "hello",
     "version": 1,
     "transport": "websocket",
     "audio_params": {
       "format": "opus",
       "sample_rate": 16000,
       "channels": 1,
       "frame_duration": 60
     }
   }
   ```
   - この`"frame_duration"`の値は`OPUS_FRAME_DURATION_MS`（例：60ms）に対応。

4. **サーバーからの "hello" 応答**  
   - デバイスはサーバーから`"type": "hello"`を含むJSONメッセージの返信を待ち、`"transport": "websocket"`がマッチするかをチェック。  
   - マッチした場合、サーバーが準備完了と判断し、音声チャンネルのオープン成功をマーク。  
   - タイムアウト時間（デフォルト10秒）内に正しい応答を受信しない場合、接続失敗と判断しネットワークエラーコールバックをトリガー。

5. **後続メッセージのやり取り**  
   - デバイス側とサーバー側間で2つの主要なデータタイプを送信可能：  
     1. **バイナリ音声データ**（Opus エンコード）  
     2. **テキストJSONメッセージ**（チャット状態、TTS/STTイベント、IoTコマンドなどの伝送用）  

   - コード内では、受信コールバックは主に以下に分かれる：  
     - `OnData(...)`:  
       - `binary`が`true`の場合、音声フレームと認識；デバイスはOpusデータとしてデコードする。  
       - `binary`が`false`の場合、JSONテキストと認識し、デバイス側でcJSONで解析して相応のビジネスロジック処理を行う（下記メッセージ構造参照）。  

   - サーバーまたはネットワークが切断された場合、`OnDisconnected()`コールバックがトリガー：  
     - デバイスは`on_audio_channel_closed_()`を呼び出し、最終的にアイドル状態に戻る。

6. **WebSocket接続のクローズ**  
   - デバイスが音声セッションを終了する必要がある時、`CloseAudioChannel()`を呼び出して能動的に接続を切断し、アイドル状態に戻る。  
   - またはサーバー側が能動的に切断した場合も、同様のコールバックフローを引き起こす。

---

## 2. 共通リクエストヘッダー

WebSocket接続確立時、コード例では以下のリクエストヘッダーが設定される：

- `Authorization`: アクセストークンを格納、形式は`"Bearer <token>"`  
- `Protocol-Version`: 例では固定値`"1"`  
- `Device-Id`: デバイス物理NIC MACアドレス  
- `Client-Id`: デバイスUUID（アプリケーション内でデバイスを一意識別）

これらのヘッダーはWebSocketハンドシェイクと共にサーバーに送信され、サーバーは必要に応じて検証、認証などを行うことができる。

---

## 3. JSONメッセージ構造

WebSocketテキストフレームはJSON形式で伝送される。以下は一般的な`"type"`フィールドとその対応ビジネスロジック。メッセージに記載されていないフィールドがある場合、オプションまたは特定の実装詳細の可能性がある。

### 3.1 クライアント→サーバー

1. **Hello**  
   - 接続成功後、クライアントが送信し、サーバーに基本パラメータを通知。  
   - 例：
     ```json
     {
       "type": "hello",
       "version": 1,
       "transport": "websocket",
       "audio_params": {
         "format": "opus",
         "sample_rate": 16000,
         "channels": 1,
         "frame_duration": 60
       }
     }
     ```

2. **Listen**  
   - クライアントが録音監視を開始または停止することを表示。  
   - 一般的なフィールド：  
     - `"session_id"`：セッション識別子  
     - `"type": "listen"`  
     - `"state"`：`"start"`, `"stop"`, `"detect"`（ウェイクアップ検出がトリガー済み）  
     - `"mode"`：`"auto"`, `"manual"` または `"realtime"`、認識モードを表示。  
   - 例：監視開始  
     ```json
     {
       "session_id": "xxx",
       "type": "listen",
       "state": "start",
       "mode": "manual"
     }
     ```

3. **Abort**  
   - 現在の発話（TTS再生）または音声チャンネルを終了。  
   - 例：
     ```json
     {
       "session_id": "xxx",
       "type": "abort",
       "reason": "wake_word_detected"
     }
     ```
   - `reason`値は`"wake_word_detected"`またはその他。

4. **Wake Word Detected**  
   - クライアントがサーバーにウェイクワード検出を通知するために使用。  
   - 例：
     ```json
     {
       "session_id": "xxx",
       "type": "listen",
       "state": "detect",
       "text": "你好小明"
     }
     ```

5. **IoT**  
   - 現在のデバイスのIoT関連情報を送信：  
     - **Descriptors**（デバイス機能、属性などを記述）  
     - **States**（デバイス状態のリアルタイム更新）  
   - 例：  
     ```json
     {
       "session_id": "xxx",
       "type": "iot",
       "descriptors": { ... }
     }
     ```
     または
     ```json
     {
       "session_id": "xxx",
       "type": "iot",
       "states": { ... }
     }
     ```

---

### 3.2 サーバー→クライアント

1. **Hello**  
   - サーバー側が返すハンドシェイク確認メッセージ。  
   - `"type": "hello"`と`"transport": "websocket"`を含む必要がある。  
   - `audio_params`を含む場合があり、サーバーが期待する音声パラメータ、またはクライアントと整合する設定を表示。  
   - 正常受信後、クライアントはイベントフラグを設定し、WebSocketチャンネル準備完了を表示。

2. **STT**  
   - `{"type": "stt", "text": "..."}`
   - サーバー側がユーザーの音声を認識したことを表示。（例：音声からテキストへの変換結果）  
   - デバイスはこのテキストを画面に表示し、その後回答フローなどに進む可能性がある。

3. **LLM**  
   - `{"type": "llm", "emotion": "happy", "text": "😀"}`
   - サーバーがデバイスに表情アニメーション/UI表現の調整を指示。  

4. **TTS**  
   - `{"type": "tts", "state": "start"}`：サーバーがTTS音声配信を準備、クライアントは "speaking" 再生状態に入る。  
   - `{"type": "tts", "state": "stop"}`：今回のTTS終了を表示。  
   - `{"type": "tts", "state": "sentence_start", "text": "..."}`
     - デバイスのインターフェースに現在再生または朗読するテキスト片を表示させる（例：ユーザーに表示するため）。  

5. **IoT**  
   - `{"type": "iot", "commands": [ ... ]}`
   - サーバーがデバイスにIoTアクション指令を送信、デバイスは解析して実行（照明オン、温度設定など）。

6. **音声データ：バイナリフレーム**  
   - サーバーが音声バイナリフレーム（Opusエンコード）を送信する時、クライアントはデコードして再生。  
   - クライアントが "listening"（録音）状態にある場合、受信した音声フレームは競合を防ぐため無視またはクリアされる。

---

## 4. 音声エンコード・デコード

1. **クライアントによる録音データ送信**  
   - 音声入力は、エコーキャンセレーション、ノイズ除去、音量ゲインなどの処理後、Opusエンコードでバイナリフレームにパッケージしてサーバーに送信。  
   - クライアントが各エンコードで生成するバイナリフレームサイズがNバイトの場合、WebSocketの**binary**メッセージでこのデータを送信する。

2. **クライアントによる受信音声の再生**  
   - サーバーからバイナリフレームを受信した時、同様にOpusデータと判定。  
   - デバイス側でデコードを行い、その後音声出力インターフェースに渡して再生。  
   - サーバーの音声サンプリングレートがデバイスと一致しない場合、デコード後に再サンプリングを行う。

---

## 5. 一般的な状態遷移

以下、デバイス側の主要状態遷移をWebSocketメッセージと対応して簡述：

1. **Idle** → **Connecting**  
   - ユーザートリガーまたはウェイクアップ後、デバイスが`OpenAudioChannel()`を呼び出し → WebSocket接続確立 → `"type":"hello"`送信。  

2. **Connecting** → **Listening**  
   - 接続確立成功後、`SendStartListening(...)`を継続実行すると録音状態に入る。この時デバイスはマイクデータを継続的にエンコードしてサーバーに送信。  

3. **Listening** → **Speaking**  
   - サーバーTTS Startメッセージ (`{"type":"tts","state":"start"}`) を受信 → 録音停止し受信音声を再生。  

4. **Speaking** → **Idle**  
   - サーバーTTS Stop (`{"type":"tts","state":"stop"}`) → 音声再生終了。自動監視を継続しない場合はIdleに戻る；自動ループが設定されている場合は再度Listeningに入る。  

5. **Listening** / **Speaking** → **Idle**（異常または能動的中断に遭遇）  
   - `SendAbortSpeaking(...)`または`CloseAudioChannel()`を呼び出し → セッション中断 → WebSocketクローズ → 状態をIdleに戻す。  

---

## 6. エラー処理

1. **接続失敗**  
   - `Connect(url)`が失敗を返すか、サーバー "hello" メッセージ待機時にタイムアウトした場合、`on_network_error_()`コールバックをトリガー。デバイスは"サービスに接続できません"または類似のエラー情報を表示。

2. **サーバー切断**  
   - WebSocketが異常切断した場合、`OnDisconnected()`コールバック：  
     - デバイスは`on_audio_channel_closed_()`をコールバック  
     - Idleまたはその他の再試行ロジックに切り替え。

---

## 7. その他の注意事項

1. **認証**  
   - デバイスは`Authorization: Bearer <token>`設定で認証を提供、サーバー側で有効性を検証する必要。  
   - トークンが期限切れまたは無効の場合、サーバーはハンドシェイクを拒否または後続で切断可能。

2. **セッション制御**  
   - コード内の一部メッセージには`session_id`が含まれ、独立した対話または操作を区別するために使用。サーバー側は必要に応じて異なるセッションに対して分離処理を行う可能性があり、WebSocketプロトコル自体は空。

3. **音声ペイロード**  
   - コード内では デフォルトでOpus形式を使用し、`sample_rate = 16000`、モノラルに設定。フレーム時間は`OPUS_FRAME_DURATION_MS`で制御、一般的に60ms。帯域幅またはパフォーマンスに応じて適切に調整可能。

4. **IoT指令**  
   - `"type":"iot"`のメッセージユーザー側コードは`thing_manager`と連携して具体的なコマンドを実行、デバイスカスタマイズにより異なる。サーバー側は配信フォーマットがクライアント側と一致することを確保する必要。

5. **エラーまたは異常JSON**  
   - JSONで必要フィールドが欠落している場合、例：`{"type": ...}`、クライアントはエラーログを記録（`ESP_LOGE(TAG, "Missing message type, data: %s", data);`）し、いかなるビジネスも実行しない。

---

## 8. メッセージ例

以下、典型的な双方向メッセージ例（簡略化フロー例示）：

1. **クライアント → サーバー**（ハンドシェイク）
   ```json
   {
     "type": "hello",
     "version": 1,
     "transport": "websocket",
     "audio_params": {
       "format": "opus",
       "sample_rate": 16000,
       "channels": 1,
       "frame_duration": 60
     }
   }
   ```

2. **サーバー → クライアント**（ハンドシェイク応答）
   ```json
   {
     "type": "hello",
     "transport": "websocket",
     "audio_params": {
       "sample_rate": 16000
     }
   }
   ```

3. **クライアント → サーバー**（監視開始）
   ```json
   {
     "session_id": "",
     "type": "listen",
     "state": "start",
     "mode": "auto"
   }
   ```
   同時にクライアントはバイナリフレーム（Opusデータ）の送信を開始。

4. **サーバー → クライアント**（ASR結果）
   ```json
   {
     "type": "stt",
     "text": "ユーザーが話した内容"
   }
   ```

5. **サーバー → クライアント**（TTS開始）
   ```json
   {
     "type": "tts",
     "state": "start"
   }
   ```
   続いてサーバーはバイナリ音声フレームをクライアントに送信して再生。

6. **サーバー → クライアント**（TTS終了）
   ```json
   {
     "type": "tts",
     "state": "stop"
   }
   ```
   クライアントは音声再生を停止、それ以上の指令がない場合はアイドル状態に戻る。

---

## 9. まとめ

本プロトコルはWebSocket上位層でJSONテキストとバイナリ音声フレームを伝送し、音声ストリームアップロード、TTS音声再生、音声認識と状態管理、IoT指令配信などの機能を完成させる。その核心特徴：

- **ハンドシェイク段階**：`"type":"hello"`を送信し、サーバーの返信を待機。  
- **音声チャンネル**：Opusエンコードのバイナリフレームで双方向音声ストリーム伝送を採用。  
- **JSONメッセージ**：`"type"`を核心フィールドとして異なるビジネスロジックを識別、TTS、STT、IoT、WakeWordなどを含む。  
- **拡張性**：実際のニーズに応じてJSONメッセージにフィールドを追加、またはheadersで追加認証を行うことが可能。

サーバーとクライアントは事前に各種メッセージのフィールド意味、時系列ロジック、エラー処理規則を取り決める必要があり、これによって通信を円滑にすることができる。上記情報は基礎文書として、後続の接続、開発または拡張に便利。