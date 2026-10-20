# VideoRecorder 推流端

> 基于 Qt + FFmpeg + OpenCV 的音视频采集与推流程序：采集摄像头/桌面画面与麦克风声音，编码为 H.264 + AAC，封装成 FLV 后推送到 RTMP 服务器（也可直接保存为本地 `.flv` 文件）。

## 一、功能

- **视频采集**：OpenCV `VideoCapture` 读摄像头；`QScreen::grabWindow` 截取主屏桌面
- **音频采集**：Qt `QAudioInput` 读取麦克风 PCM
- **视频编码**：H.264（`preset=superfast`、`tune=zerolatency`，适合实时）
- **音频编码**：AAC（44.1kHz、双声道、64kbps、FLTP）
- **封装/输出**：FLV。输出地址为 `rtmp://` 时走 RTMP 推流；为文件路径时保存为本地 `.flv`
- **画中画**：摄像头画面显示在置顶小窗中，随桌面截屏一并录入

## 二、目录结构

```
VideoRecorder/
├── VideoRecorder.pro          # qmake 工程文件
├── main.cpp
├── common.h                   # FFmpeg/OpenCV 头文件与公共宏
├── recorderdialog.h/.cpp/.ui  # 主界面：地址输入、开始/结束、预览
├── savevideofilethread.h/.cpp # 编码 + 封装 + 推流（QThread）
├── picinpic_read.h/.cpp       # 摄像头 + 桌面采集（QThread）
├── audio_read.h/.cpp          # 麦克风采集与重采样
├── picturewidget.h/.cpp/.ui   # 置顶画中画小窗
└── ffmpeg-4.2.2/              # FFmpeg 头文件与导入库（仅编译期）
```

> OpenCV 位于仓库根目录的 `opencv-release/`（仅 `include` + `lib`），`.pro` 中通过 `$$PWD/../opencv-release` 引用。

## 三、环境要求

| 组件 | 版本 / 说明 |
| --- | --- |
| 操作系统 | Windows |
| Qt | 5.12.11，MinGW 32-bit |
| FFmpeg | 4.2.2（仓库已带 `include` + `lib`） |
| OpenCV | 4.2.0（仓库已带 `include` + `lib`） |
| 编译器 | MinGW 7.3.0 32-bit（与 Qt 套件匹配） |
| 流媒体服务器 | 可选，如 nginx-rtmp |

## 四、编译

### 方式一：Qt Creator（推荐）

1. 打开 `VideoRecorder.pro`
2. 选择 `Desktop Qt 5.12.11 MinGW 32-bit` 套件
3. 选择 Debug 或 Release，点击构建

### 方式二：命令行

```bash
cd VideoRecorder

# 生成 Makefile（把路径换成你本机的 Qt 安装路径）
qmake VideoRecorder.pro -spec win32-g++ "CONFIG+=release"

# 编译（Windows Git Bash 下用 mingw32-make.exe）
mingw32-make -j4
```

> Debug 构建产物在 `debug/`，Release 产物在 `release/`。

## 五、运行前必须准备的 DLL（重要）

仓库**只提交编译期依赖**（头文件与导入库），运行时 DLL 全部被 `.gitignore` 忽略。首次运行前需要把下列文件放到 `VideoRecorder.exe` 同目录。

### 1. Qt 运行时与插件（用 windeployqt 一步完成）

```bash
windeployqt --release VideoRecorder.exe
# Debug 构建请用：windeployqt --debug VideoRecorder.exe
```

它会自动带上：

- `Qt5Core(d).dll`、`Qt5Gui(d).dll`、`Qt5Widgets(d).dll`、`Qt5Multimedia(d).dll`
- `platforms/qwindows(d).dll`
- `audio/qtaudio_windows(d).dll`（`QAudioInput` 需要）
- `mediaservice/*.dll`
- MinGW 运行时

### 2. FFmpeg 运行时

从 `ffmpeg-4.2.2/bin/` 复制：

```
avcodec-58.dll  avformat-58.dll  avutil-56.dll
swresample-3.dll  swscale-5.dll
```

（`avdevice-58.dll`、`avfilter-7.dll`、`postproc-55.dll` 当前未直接引用，可一并复制以备扩展。）

### 3. OpenCV 运行时

从 `opencv-release/bin/` 复制（最省事是把 `libopencv_*.dll` 全部拷过去）。当前 exe 直接依赖：

```
libopencv_core420.dll
libopencv_imgproc420.dll
libopencv_videoio420.dll
```

### 4. MinGW 运行时（32 位）

```
libgcc_s_dw2-1.dll
libstdc++-6.dll
libwinpthread-1.dll
```

> ⚠️ **必须是 32 位版本。** 32 位 MinGW 用的是 `libgcc_s_dw2-1.dll`；不要误拷 `libgcc_s_seh-1.dll`（通常是 64 位），否则程序启动会报缺少 `libgcc_s_dw2-1.dll`。

### Debug / Release 的 DLL 差异

| 类型 | Qt DLL | 插件 |
| --- | --- | --- |
| Debug | `Qt5Cored.dll`、`Qt5Guid.dll`、`Qt5Widgetsd.dll`、`Qt5Multimediad.dll` | `qwindowsd.dll` 等带 `d` |
| Release | `Qt5Core.dll`、`Qt5Gui.dll`、`Qt5Widgets.dll`、`Qt5Multimedia.dll` | `qwindows.dll` 等不带 `d` |

## 六、使用步骤

1. 在界面底部 `URL` 输入框填写输出地址，例如：
   - 推流：`rtmp://127.0.0.1:1935/live/test`
   - 存文件：`D:/record/test.flv`
2. 点击 **「设置直播地址」**（必须点，否则地址为空）
3. 点击 **「开始」**：主窗口最小化，画中画小窗出现，开始采集与编码
4. 点击 **「结束」**：停止采集，排空队列并写入文件尾

## 七、RTMP 服务器示例（nginx-rtmp）

```nginx
rtmp {
    server {
        listen 1935;

        application live {
            live on;
        }
    }
}
```

验证推流是否成功：

```bash
ffplay rtmp://127.0.0.1:1935/live/test
```

也可以用本仓库的 **MediaPlayer** 拉流（播放端「设置直播地址」填入同一 URL）。

## 八、编码参数

| 参数 | 值 |
| --- | --- |
| 视频编码 | H.264（`avcodec_find_encoder(AV_CODEC_ID_H264)`） |
| 帧率 | 25 fps |
| 码率 | 1 Mbps |
| 分辨率 | 主屏分辨率 |
| 像素格式 | YUV420P |
| GOP | 12 帧 |
| 音频编码 | AAC，44.1kHz / 双声道 / 64kbps / FLTP |
| 封装 | FLV |

## 九、已知限制

- **画中画依赖截屏合成**：摄像头画面显示在置顶小窗，靠桌面截图把窗口一并截入。窗口被其他窗口遮挡、移出屏幕或改变位置都会影响录制结果。
- **仅采集主屏**，多显示器场景未适配。
- **未做断线重连**：RTMP 连接断开后不会自动重连，需要手动重新开始。
- **开始前不校验地址**：未点击「设置直播地址」直接点「开始」会因地址为空而失败。
- **时间戳基于系统墙钟**（`QDateTime::currentMSecsSinceEpoch()`），长时间录制可能出现音画漂移。
- 视频编码为**软件编码**（libx264），暂未接入 NVENC/QSV 等硬件编码。

## 十、相关项目

- **MediaPlayer**：同仓库下的播放端，负责 RTMP/本地文件的解封装、解码与音视频同步播放。
