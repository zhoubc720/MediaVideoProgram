#ifndef PLAYERDIALOG_H
#define PLAYERDIALOG_H

#include <QDialog>
#include <QTimer>
#include "videoplayer.h"
QT_BEGIN_NAMESPACE
namespace Ui { class PlayerDialog; }
QT_END_NAMESPACE



class PlayerDialog : public QDialog
{
    Q_OBJECT

public:
    PlayerDialog(QWidget *parent = nullptr);
    ~PlayerDialog();

private slots:
    void on_pb_start_clicked();
    void slot_setImage(QImage);

    void on_pb_resume_clicked();

    void on_pb_pause_clicked();

    void on_pb_stop_clicked();

    void slot_PlayerStateChanged(int state);

    void slot_getTotalTime(qint64 uSec);

    void slot_TimerTimeOut();

    //事件过滤器
    bool eventFilter(QObject* obj,QEvent* event);
private:
    Ui::PlayerDialog *ui;

    VideoPlayer* m_player;

    QTimer m_timer;

    //停止的状态
    bool isStop;
};
#endif // PLAYERDIALOG_H
