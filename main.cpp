#include "playerdialog.h"

#include <QApplication>
#include <iostream>

using namespace std;
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
}
//由于我们建立的是 C++的工程
//编译的时候使用的 C++的编译器编译
//而 FFMPEG 是 C 的库
//因此这里需要加上 extern "C"
//否则会提示各种未定义
#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //这里简单的输出一个版本号
     cout << "Hello FFmpeg!" << endl;
     av_register_all();
     unsigned version = avcodec_version();
     cout << "version is:" << version << endl;;
    PlayerDialog w;
    w.show();
    return a.exec();
}
