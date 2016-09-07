#ifndef CWINFLASH_H
#define CWINFLASH_H

#include <QWidget>
#include <QThread>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QTimer>
#include <QDebug>
#include <QWidget>
#include <QCloseEvent>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <QDataStream>
#include <QButtonGroup>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#define FREE 0x00
#define SYNC 0x01
#define READ 0x02
#define SEND 0x03
#define FAIL 0x04
#define EXIT 0x05
#define QUIT 0x06
namespace Ui {
class CWinFlash;
}

class CWinFlash : public QWidget
{
    Q_OBJECT

public:
    explicit CWinFlash(QWidget *parent = 0);
    ~CWinFlash();

private:
    Ui::CWinFlash *ui;

private slots:
    void WinInit(void);
    void KeyInit(void);
    void KeyJudge(int id);
    void DatInit(void);
    void DatSave(void);
    void ComInit(void);
    void ComRead(void);
    void SendMsg(QByteArray msg);

    void FileRead(void);
    void FileOpen(void);
    void FileAdd(QString fileName);

    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void closeEvent(QCloseEvent *e);

private:
    QButtonGroup *btnGroup;
    QSettings *setting;
    QSerialPort *com;
    QByteArray text;
    QTimer *timer;
    QFile *file;
    quint8 step;
    quint8 page;
};

#endif // CWINFLASH_H
