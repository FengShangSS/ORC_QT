#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "qwidget.h"
#include "HttpClient.h"
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QVideoWidget>
#include <QFileDialog>
#include <QStatusBar>
#include <QFile>
#include "qmessagebox.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMediaPlayer>
#include <QLatin1String>
#include <QTextCodec>
#include "chineseletterhelper.h"
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QLabel>
#include <QSize>
#include <QAudioDeviceInfo>
#include <QSoundEffect>
#include <QTimer>
#include <QMediaPlaylist>

namespace Ui {
class MainWindow;
}
class QCamera;
class QCameraViewfinder;
class QCameraImageCapture;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void writeData(const QByteArray &data);
    void readData();
    const char* get_pin(unsigned short char_zh);
    void uploadPic(void);


private slots:
     void finishedSlot(QNetworkReply *reply);
     void captureImage();
     void displayImage(int,QImage);
     void saveImage();
     void on_pushButton_convert_clicked();
     void on_pushButton_yuyin_clicked();

     void on_pushButton_image_clicked();

     void on_pushButton_camera_clicked();
     void on_pushButton_input_clicked();
     void handleTimeout();  //超时处理函数

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *m_accessManager;
    QCamera *camera;
    QCameraInfo m_camera;
    QCameraViewfinder *viewfinder;
    QCameraImageCapture *imageCapture;
    QMediaPlayer *player;
    QSerialPort *serial;
    unsigned char getWordsFlag=0,beginPlayFlag=0;
    QMediaPlaylist *playlist;
    int lineIdx=0;
    int uploadTimes=0;
    typedef struct Lines
    {

    }LNS;
    typedef struct Regions
    {
       QString boundingBox;
       LNS lines;
    }RGS;
    typedef struct ResponIsee
    {
        QString language;
        float textAngle;
        QString orientation;
        RGS regions[];
    }RSI;
    QStringList getBack;
};

#endif // MAINWINDOW_H
