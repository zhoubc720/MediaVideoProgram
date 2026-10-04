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
//    // OpenGL 后端选择：必须在 QApplication 构造之前设置
//    // 依次可试（改这一行即可）：
//    //   Qt::AA_UseOpenGLES      -> ANGLE，走 D3D11，通常最快
//    //   Qt::AA_UseSoftwareOpenGL-> 软件渲染(opengl32sw.dll)，最稳但慢
//    //   Qt::AA_UseDesktopOpenGL -> 桌面 OpenGL
//    //QApplication::setAttribute(Qt::AA_UseOpenGLES);
//    QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
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
