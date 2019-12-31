#include "mainwindow.h"
#include "ui_mainwindow.h"

extern QString brailleCode;
char dataToBeSent[10]={64,64,64,64,64,64,0x0d,0x0a};
char sentpack[5000]={0};
char sentZero[8]={0x40,0x40,0x40,0x40,0x40,0x40,0x0d,0x0a};
extern int dotNum;
extern  QString braille;
QImage ScaleImage2Label(QImage qImage, QLabel* qLabel)
{
    QImage qScaledImage;
    QSize qImageSize = qImage.size();
    QSize qLabelSize = qLabel->size();
    double dWidthRatio = 1.0*qImageSize.width() / qLabelSize.width();
    double dHeightRatio = 1.0*qImageSize.height() / qLabelSize.height();
    if (dWidthRatio>dHeightRatio)
    {
        qScaledImage = qImage.scaledToWidth(qLabelSize.width());
    }
    else
    {
        qScaledImage = qImage.scaledToHeight(qLabelSize.height());
    }
    return qScaledImage;
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(QStringLiteral(":/new/prefix1/icon2.ico")));
    m_accessManager = new QNetworkAccessManager(this);
    camera = new QCamera();
   // QCameraInfo m_camera;
    foreach(const QCameraInfo &cameraInfo,QCameraInfo::availableCameras())
    {
        qDebug()<<"camera description"<<cameraInfo.description();
        if(cameraInfo.description()=="KingSen 8M camera")
        {
            camera=new QCamera(cameraInfo);
        }
    }
    viewfinder=new QCameraViewfinder(this);
    imageCapture=new QCameraImageCapture(camera);
    //ui->label_display->setScaledContents(true);
    ui->horizontalLayout->addWidget(viewfinder);

    camera->setViewfinder(viewfinder);
    QImage label_cam;
    if(!(label_cam.load(":/new/prefix1/camera.PNG"))) //加载图像
    {
        QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
        return;
    }//":/new/prefix1/logo.jpg"
    ui->label_camera->setPixmap(QPixmap::fromImage(label_cam));
    if(!(label_cam.load(":/new/prefix1/logo.jpg"))) //加载图像
    {
        QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
        return;
    }
    QImage qScaledImage=ScaleImage2Label(label_cam,ui->label_display);
    ui->label_display->setPixmap(QPixmap::fromImage(qScaledImage));
    ui->label_display->setAlignment(Qt::AlignCenter);
    //ui.label->resize(ui.label->pixmap()->size());
    ui->label_display->show();
     //ui->label_display->setPixmap(QPixmap::fromImage(label_cam));
    //ui->->setPixmap(QPixmap::fromImage(label_cam.scaled(ui->label_camera->size())));
 //   ui->horizontalLayout->addItem();
    //camera->start();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
      qDebug() << "Name : " << info.portName();
      qDebug() << "Description : " << info.description();
      qDebug() << "Manufacturer: " << info.manufacturer();
      qDebug() << "Serial Number: " << info.serialNumber();
      qDebug() << "System Location: " << info.systemLocation();
    }
    QVector<QString> aDeviceListO;
    QList<QAudioDeviceInfo> audioDeviceListO = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
    foreach (QAudioDeviceInfo devInfo, audioDeviceListO)
    {
        QString strName = devInfo.deviceName();
        if (devInfo.isNull()) continue;
        if (strName[0] == 65533) continue;
        bool bFound = false;
        //qDebug()<<devInfo.deviceName();
        foreach (QString dev, aDeviceListO) {
            if (strName == dev){
                bFound = true;
            }
        }
        if (bFound == true) continue;
        aDeviceListO.push_back(strName);
        qDebug()<<strName;
      //  ui->comboBoxOutput->addItem(strName);
    }
    serial = new QSerialPort;
    serial->setPortName("COM4");//待打开的串口号
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setParity(QSerialPort::NoParity);
    serial->setDataBits(QSerialPort::Data8);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->open(QIODevice::ReadWrite);
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    timer->start(100);
    connect(imageCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(displayImage(int,QImage)));
    QObject::connect(m_accessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(finishedSlot(QNetworkReply*)));
   // connect(imageCapture, &QCameraImageCapture::imageCaptured, this, &Camera::processCapturedImage);
    connect(ui->pushButton_Capture, SIGNAL(clicked()), this, SLOT(captureImage()));
    connect(ui->pushButton_Save, SIGNAL(clicked()), this, SLOT(saveImage()));
    connect(ui->pushButton_Quit, SIGNAL(clicked()), qApp, SLOT(quit()));
     //camera->start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_accessManager;

}

void MainWindow::handleTimeout()
{
    //qDebug()<<"Enter timeout processing function\n";
    static int timCount = 0;
    static int taskDoneFlag = 0;
    if(getWordsFlag == 1)
    {
        timCount++;
        if(timCount > 3)
        {
            timCount = 0;
            //qDebug() << "size:" << getBack.size();
            if(lineIdx<getBack.size())
            {
                QString temp = getBack.at(lineIdx++);
                ChineseLetterHelper::GetPinyins(temp);
//                temp = sentpack;

//                QByteArray temp2("");

//                temp2.append(temp);

//                QByteArray text = QByteArray::fromHex(temp2);

                qDebug()<<temp;
                serial->write(sentpack);
                for(int k = 0; k < 5000; k++)
                {
                    sentpack[k] = 0;
                }
                taskDoneFlag = 1;
            }
            else if(taskDoneFlag == 1)
            {
                taskDoneFlag = 0;
                getWordsFlag = 0;
            }
        }
    }
    else if(getWordsFlag == 2)
    {
        serial->write(sentpack);
        for(int k = 0; k < 5000; k++)
        {
            sentpack[k] = 0;
        }
        getWordsFlag = 0;
    }

}

void MainWindow::finishedSlot(QNetworkReply *reply)
{
    QString responWords;
    if(reply->error() == QNetworkReply::NoError) {
        QByteArray all = reply->readAll();
        QJsonParseError jsonError;
        QJsonDocument document = QJsonDocument::fromJson(all, &jsonError);

        qDebug() << document;

        if (!document.isNull() && (jsonError.error == QJsonParseError::NoError)) {
            qDebug() << "Json success";
            if (document.isObject()) {
                QJsonObject object = document.object();
                qDebug() << object.keys();
                if(object.contains("words_result")) {
                    QJsonValue object_v = object.value("words_result");
                    if(object_v.isArray()) {
                        QJsonArray words_result_a = object_v.toArray();
                        int nSize = words_result_a.size();
                        qDebug() << nSize;
                        for(int i = 0; i < nSize; i++) {
                            QJsonValue words_result_v = words_result_a[i];
                            if(words_result_v.isObject()) {
                               QJsonObject words_result_o =words_result_v.toObject();
                               if(words_result_o.contains("words")) {
                                   responWords.append(words_result_o.value("words").toString());
                                   getBack.append(words_result_o.value("words").toString());
                               }
                            }
                            responWords.append(" ");
                        }
                        //getBack.append(oneLine);
                        responWords.append("\n");
                    }
                }
            }
        }
        else {
            qDebug() << "Json failure";
        }
    }
    else {
        qDebug() << "handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        qDebug( "found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        qDebug(qPrintable(reply->errorString()));
    }

    reply->deleteLater();

    QString disString;
    for(int i=0;i<getBack.size();i++)
    {
        disString.append(getBack.at(i)+"\n");
    }
    ui->textBrowser->setText(disString);
    //qDebug()<<getBack.
    qDebug()<<"all words num"<<responWords.size();

    uploadTimes = responWords.size()/200+1;
    qDebug()<<"uploadtimes"<<uploadTimes;
    getWordsFlag = 1;
    player = new QMediaPlayer(Q_NULLPTR,QMediaPlayer::StreamPlayback);

    QTextCodec *utf8 = QTextCodec::codecForName("utf-8");
    if(uploadTimes <= 1)
    {
        QString myUrl = "http://tsn.baidu.com/text2audio?spd=4&tex=";
        QByteArray encoded = utf8->fromUnicode(responWords).toPercentEncoding();
        myUrl.append(encoded);
        myUrl.append("&lan=zh&cuid=10389036&ctp=1&per=0&tok=25.25b7ce6a71a5d84039c3f97f77fc6f45.315360000.1836217242.282335-10389036");
        player->setMedia(QUrl::fromLocalFile(myUrl));
        qDebug()<<myUrl;
//        player->setMedia(QUrl::fromLocalFile(QStringLiteral("D:\\GAME\\text2audio.mp3")));
        // player->setVolume();
        player->play();
        beginPlayFlag = 1;
    }
    else
    {
        playlist = new QMediaPlaylist;
        for(int i=0;i<uploadTimes;i++)
        {
            QString stringPart;
            stringPart = responWords.mid(i*200,200);
            qDebug()<<stringPart;
            QString myUrl = "http://tsn.baidu.com/text2audio?tex=";
            QByteArray encoded = utf8->fromUnicode(stringPart).toPercentEncoding();
            myUrl.append(encoded);
            myUrl.append("&lan=zh&cuid=10389036&ctp=1&per=0&tok=25.25b7ce6a71a5d84039c3f97f77fc6f45.315360000.1836217242.282335-10389036");
            playlist->insertMedia(i,QUrl::fromLocalFile(myUrl));
            //player->setMedia(QUrl::fromLocalFile(myUrl));
        }
        //playlist->setMediaObject(player);
        player->setPlaylist(playlist);
        player->play();
    }
    //player->setPosition(3000);
    //   player->stop();
    QString tests="你好啊";
   // tests=ui->textEdit->toPlainText();
    qDebug() << "responWords:" << responWords;
    QString pinyin = ChineseLetterHelper::GetPinyins(responWords);
   //  ChineseLetterHelper::pinYinToBraille(brailleCode);
    ui->textBrowser_2->setText(pinyin);
    ui->textBrowser_3->setText(braille);

}
char imageFlag=0;
void MainWindow::captureImage()
{
    ui->statusBar->showMessage(tr("正在捕获图片"), 1000);
    //QCoreApplication::applicationDirPath();
    imageCapture->capture(QCoreApplication::applicationDirPath()+"/pT.jpg");
    imageFlag=1;
}

void MainWindow::displayImage(int , QImage image)
{
    ui->label_display->setPixmap(QPixmap::fromImage(image));
    ui->statusBar->showMessage(tr("捕获成功"), 5000);
}

void MainWindow::saveImage()
{
    QString fileName=QFileDialog::getSaveFileName(this, tr("保存到文件"),QCoreApplication::applicationDirPath()+"/pT.jpg", tr("jpeg格式文件(*.jpg)"));
    if(fileName.isEmpty()) {
        ui->statusBar->showMessage(tr("保存操作已取消"), 5000);
        return;
    }
    const QPixmap* pixmap=ui->label_display->pixmap();
    if(pixmap) {
        pixmap->save(fileName);
        ui->statusBar->showMessage(tr("保存成功"), 5000);
    }
}
void MainWindow::writeData(const QByteArray &data)
{
    serial->write(dataToBeSent);
}
void MainWindow::readData()
{
    static unsigned char pauseFlag=0;
    unsigned char instruction=0;
    static unsigned char flag=0;
    QByteArray data = serial->readAll();
    instruction=(unsigned char)data.at(0);
    if(instruction==1)
    {
        qDebug() << "left!!!!!!!!!!!!!";
        uploadPic();
    }
    else if(instruction==2)
    {
        qDebug() << "up!!!!!!!!!!!!!";
        if(flag==0)
        {
            ui->label_camera->deleteLater();
            camera->start();
            ui->pushButton_camera->setText("close_camera");
            flag++;
        }
        else if(flag==1)
        {
            captureImage();
            uploadPic();
        }
    }
    else if(instruction==3)
    {
        pauseFlag=!pauseFlag;
        if(pauseFlag)
        {
          player->pause();
        }
        else
        {
            player->play();
        }

    }
    //qDebug()<<instruction;
}
QString fileName;
void MainWindow::on_pushButton_convert_clicked()
{
    uploadPic();
}
void MainWindow::uploadPic(void)
{
    QFile *file;
    //camera->start();
    if(imageFlag==2)
    {
        file=new QFile(fileName);
    }
    else if(imageFlag==1)
    {//C:\Users\Administrator\Desktop\release3.0\release3.0
         file = new QFile(QCoreApplication::applicationDirPath()+"/pT.jpg");
        //file = new QFile("C:/Users/m2/Desktop/iseeRelease4.0/pT.jpg");
    }
    else
    {
         file = new QFile(":/new/prefix1/p2.jpg");
    }
    file->open(QIODevice::ReadOnly);
    QByteArray fdata = file->readAll();
    if(fdata.isEmpty()){
        return;
    }
    file->close();
    QByteArray hexed = fdata.toBase64();
    QByteArray urlencode = hexed.toPercentEncoding();
    qDebug() << "Successfully encode the picture";

    QString access_token = "24.d7e3e5e5a014f83668fd9e485d7a071c.2592000.1557103992.282335-14242560";
//    QFile file2("d:/iSEE2.0/BDAPI/key.txt");
//    if (file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        QTextStream in(&file2);
//        while( !in.atEnd()) {
//            access_token = in.readLine();
//            qDebug() << access_token;
//        }
//        file2.close();
//    }
    QString accessURL = "https://aip.baidubce.com/rest/2.0/ocr/v1/accurate_basic?access_token=";
    QString url_str = accessURL + access_token;
    QUrl url(url_str);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    QByteArray body = "image=" + urlencode;

    qDebug() << "Uploading......";
    QNetworkReply *reply = m_accessManager->post(request,body);
    qDebug() << reply->error();
    qDebug() << "Uploaded!";
}

void MainWindow::on_pushButton_yuyin_clicked()
{
    // appKey = 5HIBVpQ3bxo1Y1X72MKDa8gb
    // appSecret = 934b5999cea2b332351e537918219dcb
    // appid=10389036
 //   HttpClient("http://xtuer.github.io/img/dog.png").debug(true).download("G:/dog-1.png");
/*
    QNetworkRequest request;
    request.setUrl(QUrl("https://openapi.baidu.com/oauth/2.0/token?grant_type=client_credentials&client_id=5HIBVpQ3bxo1Y1X72MKDa8gb&client_secret=934b5999cea2b332351e537918219dcb"));
  //  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/octet-stream");
  //  request.setRawHeader("Ocp-Apim-Subscription-Key","257198d391b045abad53e97c893dd3ba");
   // QByteArray postData;
  //  postData.append(fdata);
  //  postData.append("{\"url\": \"https://upload.wikimedia.org/wikipedia/commons/thumb/a/af/Atomist_quote_from_Democritus.png/338px-Atomist_quote_from_Democritus.png\"}");
    QNetworkReply* reply =m_accessManager->get(request);
*/

    QNetworkRequest request;
    request.setUrl(QUrl("http://tsn.baidu.com/text2audio?tex=%e7%99%be%ba%a6%e4%bd%a0%e5%a5%bd&lan=zh&cuid=10389036&ctp=1&tok=24.4462b2847d89c403671ed42070fd175d.2592000.1514987657.282335-10389036"));
  //  request.setHeader(QNetworkRequest::ContentTypeHeader,"application/octet-stream");
  //  request.setRawHeader("Ocp-Apim-Subscription-Key","257198d391b045abad53e97c893dd3ba");
   // QByteArray postData;
  //  postData.append(fdata);
  //  postData.append("{\"url\": \"https://upload.wikimedia.org/wikipedia/commons/thumb/a/af/Atomist_quote_from_Democritus.png/338px-Atomist_quote_from_Democritus.png\"}");
    QNetworkReply* reply =m_accessManager->get(request);
    //24.2a7283b914938d9dc394c0dccfc94c6d.2592000.1514989289.282335-10389036
  //  url为 http://tsn.baidu.com/text2audio?tex=%e7%99%be%e5%ba%a6%e4%bd%a0%e5%a5%bd&lan=zh&cuid=***&ctp=1&tok=***
}

void MainWindow::on_pushButton_image_clicked()
{
       QString filename=QFileDialog::getOpenFileName(this,tr("选择图像"),"",tr("Images (*.png *.bmp *.jpg)"));
       if(filename.isEmpty())
           return;
       else
       {
           QImage img;
           if(!(img.load(filename))) //加载图像
           {
               QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
               return;
           }
           fileName=filename;
           QImage qScaledImage=ScaleImage2Label(img,ui->label_display);
           ui->label_display->setPixmap(QPixmap::fromImage(qScaledImage));
           ui->label_display->setAlignment(Qt::AlignCenter);
           //ui.label->resize(ui.label->pixmap()->size());
           ui->label_display->show();
         //  ui->label_display->setPixmap(QPixmap::fromImage(img.scaled(ui->label_display->size())));
       }
       imageFlag=2;
}

void MainWindow::on_pushButton_camera_clicked()
{
    //qDebug()<<camera->status();

    if(camera->status()==QCamera::ActiveStatus)
    {
        QImage label_cam;
        if(!(label_cam.load(":/new/prefix1/camera.PNG"))) //加载图像
        {
            QMessageBox::information(this, tr("打开图像失败"),tr("打开图像失败!"));
            return;
        }
        ui->label_camera->setPixmap(QPixmap::fromImage(label_cam));
        camera->stop();
        ui->pushButton_camera->setText("open_camera");
    }
    else
    {
        //ui->label_camera->deleteLater();
        ui->label_camera->clear();
        camera->start();
        ui->pushButton_camera->setText("close_camera");
    }

}

void MainWindow::on_pushButton_input_clicked()
{
    QString words_input_manually = ui->textEdit->toPlainText();
    QString pinyin = ChineseLetterHelper::GetPinyins(words_input_manually);
    ui->textBrowser_4->setText(pinyin);
    ui->textBrowser_5->setText(braille);
    getWordsFlag = 2;
}
