#ifndef SAVEVIDEOFILETHREAD_H
#define SAVEVIDEOFILETHREAD_H

#include <QThread>
#include "picinpic_read.h"
#include "audio_read.h"
#include <QMutex>

typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    uint8_t* frameBuffer;
    int frameBufferSize;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;


struct STRU_AF_FORMAT
{
    //    void clear()
    //    {
    //        memset(this,0,sizeof(STRU_AF_FORMAT));
    //    }
    bool hasCamera;
    bool hasDesk;
    bool hasAudio;

    //////////视频信息/////////
    int width;
    int height;
    int frame_rate;
    int videoBitRate;
    QString fileName; //URL或name
    //int codec_id; //H.264 默认
    //int pix_fmt; //YUV420P 默认

    /////////音频信息/////////
    /// ...TODO
};

struct BufferDataNode
{
    uint8_t * buffer;
    int bufferSize;
    int64_t time; //视频帧用于稳帧, 比较时间
};


class SaveVideoFileThread : public QThread
{
    Q_OBJECT
signals:
    void SIG_sendVideoFrame( QImage img ); // 用于预览
    void SIG_sendPicInPic( QImage img ); //用于显示画中画
public:
    explicit SaveVideoFileThread(QObject *parent = nullptr);

    const STRU_AF_FORMAT &avFormat() const;

public slots:
    void slot_writeVideoFrameData( uint8_t* picture_buf, int buffer_size ); //采集的数据格式 YUV420P

    void slot_writeAudioFrameData ( uint8_t* picture_buf, int buffer_size );
    void slot_setInfo(STRU_AF_FORMAT& avFormat); //可以用于设置视频的格式和选项
    void slot_openVideo();
    void slot_closeVideo();
private:
    void run();

    //采集音频视频 归写线程来管理
    PicInPic_Read* m_picInPicRead;
    Audio_Read * m_audioRead;

    STRU_AF_FORMAT m_avFormat;

    OutputStream video_st = { 0 }, audio_st = { 0 };
    const char *filename;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *audio_codec, *video_codec;
    int ret;
    int have_video = 0, have_audio = 0;
    int encode_video = 0, encode_audio = 0;

    int mAudioOneFrameSize;

    QList<BufferDataNode*> m_videoDataList;
    QList<BufferDataNode*> m_audioDataList;
    BufferDataNode* lastVideoNode;
    bool m_videoBeginFlag;
    qint64 m_videoBeginTime; //int64_t 就是long long
    QMutex m_videoMutex;
    QMutex m_audioMutex;
    bool isStop;
    int64_t video_pts;
    int64_t audio_pts;

    void videoDataQuene_Input(uint8_t *buffer, int size, int64_t time);
    BufferDataNode *videoDataQuene_get(int64_t time);
    void audioDataQuene_Input(const uint8_t *buffer, const int &size);
    BufferDataNode *audioDataQuene_get();

    void add_video_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, AVCodecID codec_id);
    void add_audio_stream(OutputStream *ost, AVFormatContext *oc, AVCodec **codec, AVCodecID codec_id);
    void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
    void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
    void close_stream(AVFormatContext *oc, OutputStream *ost);
    int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c, AVStream *st, AVFrame *frame);
    int write_frame(AVFormatContext *fmt_ctx, AVCodecContext *c, AVStream *st, AVFrame *frame, int64_t &pts, OutputStream *ost);
    bool write_video_frame(AVFormatContext *oc, OutputStream *ost, double time);
    bool write_audio_frame(AVFormatContext *oc, OutputStream *ost);
};

#endif // SAVEVIDEOFILETHREAD_H
