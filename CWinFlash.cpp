#include "CWinFlash.h"
#include "ui_CWinFlash.h"
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      构造
******************************************************************************/
CWinFlash::CWinFlash(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CWinFlash)
{
    ui->setupUi(this);
    WinInit();
    KeyInit();
    DatInit();
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(ComRead()));
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      析构
******************************************************************************/
CWinFlash::~CWinFlash()
{
    delete ui;
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      界面初始化
******************************************************************************/
void CWinFlash::WinInit()
{
    QStringList com;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        com << info.portName();
    }
    ui->Box1->addItems(com);

    QFile file;
    QString qss;
    file.setFileName(":/css/aip001.css");
    file.open(QFile::ReadOnly);
    qss = QLatin1String(file.readAll());
    this->setStyleSheet(qss);
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      按键初始化
******************************************************************************/
void CWinFlash::KeyInit()
{
    btnGroup = new QButtonGroup;
    btnGroup->addButton(ui->KeyOpen, Qt::Key_A);
    btnGroup->addButton(ui->KeyClear,Qt::Key_B);
    btnGroup->addButton(ui->KeyFile, Qt::Key_E);
    connect(btnGroup,SIGNAL(buttonClicked(int)),this,SLOT(KeyJudge(int)));
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      按键功能
******************************************************************************/
void CWinFlash::KeyJudge(int id)
{
    switch (id) {
    case Qt::Key_A:
        if (ui->KeyOpen->text().contains("开始下载")) {
            ComInit();
        } else {
            ComQuit();
        }
        break;
    case Qt::Key_B:
        ui->textBrowser->clear();
        break;
    case Qt::Key_E:
        FileOpen();
        break;
    default:
        break;
    }
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      读取设置
******************************************************************************/
void CWinFlash::DatInit()
{
    setting = new QSettings("default",QSettings::IniFormat);
    ui->Box1->setCurrentText(setting->value("/Default/COM").toString());
    QStringList t = (setting->value("/Default/FILE").toString()).split("@@");
    ui->comboBox->addItems(t);
    step = 0;
    page = 0;
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      保存设置
******************************************************************************/
void CWinFlash::DatSave()
{
    QStringList t;
    for (int i=0; i<ui->comboBox->count(); i++)
        t.append(ui->comboBox->itemText(i));
    setting->setValue("/Default/COM",ui->Box1->currentText());
    setting->setValue("/Default/FILE",t.join("@@"));
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      打开串口
******************************************************************************/
void CWinFlash::ComInit()
{
    com = new QSerialPort(ui->Box1->currentText(),this);
    if (com->open(QIODevice::ReadWrite)) {
        com->setBaudRate(ui->Box2->currentText().toInt());    //波特率
        if (ui->Box3->currentText() == "None")
            com->setParity(QSerialPort::NoParity);
        if (ui->Box3->currentText() == "Odd")
            com->setParity(QSerialPort::OddParity);
        if (ui->Box3->currentText() == "Even")
            com->setParity(QSerialPort::EvenParity);

        if (ui->Box4->currentText() == "8")
            com->setDataBits(QSerialPort::Data8);
        if (ui->Box4->currentText() == "7")
            com->setDataBits(QSerialPort::Data7);
        if (ui->Box4->currentText() == "6")
            com->setDataBits(QSerialPort::Data6);

        if (ui->Box5->currentText() == "1")
            com->setStopBits(QSerialPort::OneStop);
        if (ui->Box5->currentText() == "1.5")
            com->setStopBits(QSerialPort::OneAndHalfStop);
        if (ui->Box5->currentText() == "2")
            com->setStopBits(QSerialPort::TwoStop);

        com->setFlowControl(QSerialPort::NoFlowControl);
        com->setDataTerminalReady(true);
        com->setRequestToSend(false);
        ui->KeyOpen->setText("中断下载");
        SendMsg("请重启单片机\n");

        timer->start(50);
        return;
    }
    SendMsg("串口打开失败\n");
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      读取串口数据
******************************************************************************/
void CWinFlash::ComRead()
{
    QByteArray ack;
    if (com->bytesAvailable() > 0) {
        ack = com->readAll();

        switch (ack.at(0)) {
        case 'R':
            file = new QFile(ui->comboBox->currentText());
            if (!file->open(QFile::ReadOnly)) {
                SendMsg("打开文件失败\n");
                step = QUIT;
                break;
            }
            SendMsg("正在同步设备......   ");
            step = SYNC;
            break;
        case 'C':
            SendMsg("同步成功\n");
            step = SEND;
            break;
        case 'A':
            SendMsg("写入成功\n");
            page++;
            if (file->atEnd())
                step = OVER;
            else
                step = SEND;
            break;
        case 'N':
            SendMsg("写入失败\n");
            step = FAIL;
            break;
        case 'E':
            SendMsg("写入完成\n");
            step = QUIT;
            page = 0;
            break;
        case 'V':
            ack = ack.mid(0,4);
            ack.insert(0,"Bootloader版本");
            ack.append("\n");
            SendMsg(ack);
            step = FREE;
            break;
        default:
            break;
        }
    }

    switch (step) {
    case FREE:
        com->write("R");
        break;
    case SYNC:
        com->write("C");
        break;
    case SEND:
        if (ack.isEmpty())
            break;
        FileRead();
        SendMsg(QString("正在写入第%1帧数据......   ").arg(page+1, 2, 10, QChar('0')).toUtf8());
        com->write(text);
        break;
    case FAIL:
        SendMsg(QString("重新写入第%1帧数据......   ").arg(page+1, 2, 10, QChar('0')).toUtf8());
        com->write(text);
        break;
    case OVER:
        SendMsg("正在完成写入.....   ");
        com->write("E");
        break;
    case QUIT:
        ComQuit();
        step = FREE;
    default:
        break;
    }
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      关闭串口
******************************************************************************/
void CWinFlash::ComQuit()
{
    timer->stop();
    com->close();
    ui->KeyOpen->setText("开始下载");
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      显示数据
******************************************************************************/
void CWinFlash::SendMsg(QByteArray msg)
{
    ui->textBrowser->insertPlainText(msg);
    ui->textBrowser->moveCursor(QTextCursor::End);
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      读取一页数据
******************************************************************************/
void CWinFlash::FileRead(void)
{
    text = file->read(1024);
    if (file->atEnd()) {
        for (int i=text.size(); i<1024; i++)
            text.append(0xff);
    }
    quint8 crc = 0;
    for (int i=0; i<1024; i++)
        crc += text.at(i);
    text.insert(0,'S');
    text.insert(1,page);
    text.insert(2,!page);
    text.insert(3,crc);
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      打开文件
******************************************************************************/
void CWinFlash::FileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开文件"),
                                                    "/",
                                                    tr("bin文件(*.bin)"";;任何文件(*.*)"));
    if (fileName.length() != 0)
        FileAdd(fileName);
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      添加文件
******************************************************************************/
void CWinFlash::FileAdd(QString fileName)
{
    QStringList t;
    for (int i=0; i<ui->comboBox->count(); i++)
        t.append(ui->comboBox->itemText(i));
    ui->comboBox->clear();
    for (int i=0; i<t.size(); i++) {
        if (t.at(i) == fileName) {
            t.move(i,0);
            ui->comboBox->addItems(t);
            ui->comboBox->setCurrentText(fileName);
            return;
        }
    }
    t.insert(0,fileName);
    if (t.size()>5)
        t.removeLast();
    ui->comboBox->addItems(t);
    ui->comboBox->setCurrentText(fileName);
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      拖放效果
******************************************************************************/
void CWinFlash::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty())
        return;
    QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return;
    FileAdd(fileName);
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      拖放效果
******************************************************************************/
void CWinFlash::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasFormat("text/uri-list"))
        e->acceptProposedAction();
}
/******************************************************************************
  * version:    1.0
  * author:     link
  * date:       2016.08.18
  * brief:      退出保存
******************************************************************************/
void CWinFlash::closeEvent(QCloseEvent *e)
{
    DatSave();
    e->accept();
}
