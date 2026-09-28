# MediaPlayer 音视频直播项目

基于 **Qt + FFmpeg + SDL2** 的音视频播放／直播项目（开发中）。

当前阶段已完成本地媒体文件的解封装、解码、音视频同步播放，作为后续接入
**RTMP / RTSP 等直播流**的基础。项目仍在持续迭代，接口与结构会不断调整。

---

## 功能特性

### 已完成

- 打开本地媒体文件（`flv` / `rmvb` / `avi` / `mp4` / `mkv` 等）
- 播放、暂停、继续、停止
- 进度条拖动跳转（seek）
- 当前播放时间与总时长显示（定时刷新）
- **音视频同步**：以音频时钟 `audio_clock` 为基准，视频帧按
  `video_clock - audio_clock` 的差值进行延时，避免音画不同步
- 读线程与视频解码线程分离，配合线程安全的 `PacketQueue`（SDL 互斥量 + 条件变量）
- 音频经 `swresample` 重采样后通过 SDL 音频回调输出
- 视频经 `sws_scale` 转换为 RGB 后由 Qt `QLabel` 渲染显示

### 规划中

- RTMP / RTSP 等直播流拉取与播放
- 音量控制、倍速播放
- 快捷键控制（空格暂停／继续、方向键快进快退、上下调节音量）
- 音视频同步策略优化（引入更完善的时钟与丢帧策略）

---

## 技术栈

| 组件 | 版本 | 用途 |
| --- | --- | --- |
| Qt | 5.12.11（MinGW 32-bit） | 界面、信号槽与线程 |
| FFmpeg | 4.2.2（win32 shared） | 解封装、解码、重采样、像素格式转换 |
| SDL2 | 2.0.10 | 音频输出、线程与同步原语 |

---

## 目录结构

```
MediaPlayer/
├── main.cpp                 # 程序入口
├── playerdialog.h/.cpp/.ui  # 播放器界面与交互
├── videoplayer.h/.cpp       # 播放核心：解封装、解码、同步、渲染
├── PacketQueue.h/.cpp       # 线程安全的数据包队列
├── MediaPlayer.pro          # qmake 工程文件
├── .gitignore
├── README.md
├── ffmpeg-4.2.2/            # 编译期依赖：仅提交 include/ 与 lib/
│                            # （bin/ 中的运行时 dll 未纳入版本控制）
├── SDL2-2.0.10/             # 编译期依赖：仅提交 include/ 与 lib/
│                            # （bin/ 中的运行时 dll 未纳入版本控制）
└── dll/                     # 运行时 dll 收集目录（未纳入版本控制）
```

---

## 环境要求

- Windows
- Qt 5.12.11，使用 **MinGW 32-bit** 套件（与依赖库位数一致）
- FFmpeg 4.2.2 win32 开发包
- SDL2 2.0.10（MinGW 32-bit 开发包）

> 注意：工程使用 32 位依赖（`.pro` 中链接的是
> `ffmpeg-4.2.2/lib/*.lib` 与 `SDL2-2.0.10/lib/x86/SDL2.lib`），
> 请务必选择 32 位编译器套件，否则会因位数不匹配而链接失败。

---

## 依赖准备

为控制仓库体积，本项目**只提交编译期依赖（头文件与导入库）**，
**运行时 dll 未提交**。克隆后请自行准备：

1. 下载 **FFmpeg 4.2.2 win32 shared** 版本，按下述路径放置：
   - 头文件 → `ffmpeg-4.2.2/include`
   - 导入库 → `ffmpeg-4.2.2/lib`

2. 下载 **SDL2 2.0.10（MinGW 32-bit）**，按下述路径放置：
   - 头文件 → `SDL2-2.0.10/include`
   - 导入库 → `SDL2-2.0.10/lib`

3. 运行程序前，将以下 **运行时 dll 复制到可执行文件同目录**：
   - `avcodec-58.dll`、`avdevice-58.dll`、`avfilter-7.dll`、`avformat-58.dll`
   - `avutil-56.dll`、`postproc-55.dll`、`swresample-3.dll`、`swscale-5.dll`
   - `SDL2.dll`

   这些 dll 分别位于 FFmpeg 与 SDL2 的 `bin/` 目录中。

---

## 编译与运行

### 使用 Qt Creator

1. 用 Qt Creator 打开 `MediaPlayer.pro`
2. 选择 **Qt 5.12.11 MinGW 32-bit** 套件
3. 执行构建（Build）
4. 将上一步列出的运行时 dll 复制到生成的可执行文件所在目录
5. 运行

### 使用命令行（qmake）

```bash
# 生成 Makefile（请将路径替换为你的 Qt/MinGW 环境）
qmake MediaPlayer.pro

# 编译
mingw32-make

# 运行前请先复制运行时 dll 到可执行文件目录
```

---

## 说明

- 项目处于开发中，尚未完成直播相关功能，后续会持续更新。
- 仓库不包含编译产物（`build-*`）、Qt Creator 个人配置（`*.pro.user`）
  及运行时 dll，详见 `.gitignore`。
