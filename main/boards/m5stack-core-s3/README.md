# 使用方法


1. コンパイルターゲットをesp32s3に設定

```shell
idf.py set-target esp32s3
```

2. 設定を変更

```shell
cp main/boards/m5stack-core-s3/sdkconfig.cores3 sdkconfig
```

3. プログラムをコンパイル・フラッシュ

```shell
idf.py build flash monitor
```

> [!NOTE]
> ダウンロードモードに入る方法：リセットボタンを長押し（約3秒）し、内部インジケータライトが緑色に点灯するまで待ってから、ボタンを離してください。


 
