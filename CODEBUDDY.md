# CODEBUDDY.md

This file provides guidance to CodeBuddy Code when working with code in this repository.

## Repository layout

This is a monorepo containing two independent qmake projects plus one shared SDK:

- `MediaPlayer/` — playback: demux → decode → A/V sync → render.
- `VideoRecorder/` — capture → encode → mux → RTMP push.
- `opencv-release/` — OpenCV 4.2.0, only `include/` and `lib/` (compile-time).
- The RTMP server (nginx-rtmp) is external to this repository.

Both projects are **32-bit** Windows builds: Qt 5.12.11 (MinGW 32-bit) + FFmpeg 4.2.2. `MediaPlayer` additionally uses SDL2 2.0.10; `VideoRecorder` additionally uses OpenCV 4.2.0.

## Commands

Build each project from its own directory:

```bash
# MediaPlayer
cd MediaPlayer
qmake MediaPlayer.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -j4

# VideoRecorder
cd VideoRecorder
qmake VideoRecorder.pro -spec win32-g++ "CONFIG+=release"
mingw32-make -j4
```

Qt Creator is the primary workflow: open the `.pro` and select the `Desktop Qt 5.12.11 MinGW 32-bit` kit. Toolchain locations in this environment:

- Qt: `D:\NewQtload\5.12.11\mingw73_32`
- MinGW: `D:\NewQtload\Tools\mingw730_32`

Deploy the Qt runtime next to the executable before running:

```bash
windeployqt --release <path-to-exe>
# Debug builds: windeployqt --debug <path-to-exe>
```

There is **no automated test suite and no lint/format configuration** in this repository. Verification is build + manual run.

Push (GitHub is unreachable directly from this machine; the local proxy must be running):

```bash
git -c http.proxy=http://127.0.0.1:7897 -c https.proxy=http://127.0.0.1:7897 push origin master
```

## MediaPlayer architecture

`VideoPlayer` (a `QThread`) owns the pipeline; `PlayerDialog` owns `VideoPlayer` and `MyOpenGLWidget` and connects signals to the UI.

- `VideoPlayer::run()` opens the input with `avformat_open_input`, resolves streams, opens decoders, creates `videoq`/`audioq` (`PacketQueue`), starts the video thread (`SDL_CreateThread`, or `SDL_AddTimer` for video-only input), opens the SDL audio device, then loops on `av_read_frame` and routes packets by `stream_index`.
- `video_thread()` decodes, compares `video_clock` with `audio_clock` (both microseconds) and delays while video is ahead, converts with `sws_scale` to RGB32, wraps the buffer in a `QImage`, and emits `SIG_getOneImage`.
- `PlayerDialog::slot_setImage()` forwards the image to `MyOpenGLWidget::slot_setImage()`; the widget uploads it as a texture and draws in `paintGL()`. Rendering is OpenGL, not `QLabel`.
- `audio_callback()` / `audio_decode_frame()` decode, `swr_convert` to S16, output via `SDL_MixAudioFormat`, and maintain `audio_clock`, which is the master clock for sync.
- Seek performs `av_seek_frame` and pushes a sentinel `FLUSH` packet into both queues; decode loops call `avcodec_flush_buffers` when they encounter it. Without this handshake, stale frames from the decoder produce corruption.
- Backpressure: `run()` stops reading while `audioq->size` / `videoq->size` exceed `MAX_AUDIO_SIZE` / `MAX_VIDEO_SIZE`.

Coupling to keep in mind: `VideoPlayer` owns the shared `VideoState` used by the SDL decode threads; `PacketQueue` is the only synchronization point between the read thread and the decode threads.

## VideoRecorder architecture

`SaveVideoFileThread` (a `QThread`) owns both capture objects (`PicInPic_Read`, `Audio_Read`) and the encoder/muxer.

- `RecorderDialog::on_pb_start_clicked()` fills `STRU_AF_FORMAT` and calls `slot_setInfo()`, which allocates the FLV output context, adds H.264 and AAC streams, opens codecs, calls `avio_open()`, writes the header, and starts the thread; then `slot_openVideo()` starts capture.
- `PicInPic_Read` (a `QThread`) polls the camera at 25 fps through OpenCV. Screen capture is delegated to the GUI thread via `SIG_getDeskImg` with `Qt::BlockingQueuedConnection`, because `QScreen::grabWindow` must run there. The desktop image is converted RGB888 → YUV420P and emitted as the encoded video frame.
- `Audio_Read` (a `QObject` driven by a `QTimer`) reads `QAudioInput` every 20 ms, accumulates 4096-byte chunks, and `swr_convert`s S16 → FLTP.
- The frame slots only enqueue into `m_videoDataList` / `m_audioDataList` under `QMutex`. `SaveVideoFileThread::run()` compares the next PTS of the two streams, writes whichever is due, and uses `avcodec_send_frame` / `avcodec_receive_packet` / `av_interleaved_write_frame`.
- Output uses the FLV muxer: an `rtmp://` `fileName` goes over RTMP via `avio_open`, a filesystem path writes a local `.flv`.
- Picture-in-picture is **not composited**: the camera is displayed in the always-on-top `PictureWidget`, and the full-screen capture includes that window. Window occlusion or moving it changes the recorded result.

## Constraints that are easy to get wrong

- Everything is 32-bit. Never introduce 64-bit DLLs. The MinGW runtime must be `libgcc_s_dw2-1.dll`; `libgcc_s_seh-1.dll` is the wrong architecture and causes a "missing libgcc_s_dw2-1.dll" failure at startup.
- `.gitignore` keeps only compile-time dependencies. Never commit `bin/`, `dll/`, `*.dll`, `*.exe`, `build*/`, or `*.pro.user`.
- `VideoRecorder.pro` locates OpenCV through `$$PWD/../opencv-release`; keep that relative path valid if either directory moves.
- Parts of the FFmpeg API in use are deprecated (`av_register_all`, `avcodec_decode_video2`, `av_free_packet`). Migration to the send/receive API is a known future task, not a current requirement.
- `MediaPlayer/README.md` still describes `QLabel` rendering and says live playback is not implemented. The code has since moved to `MyOpenGLWidget` and added a live-URL entry, so trust the code over that README.
