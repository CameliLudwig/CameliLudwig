#ifndef COMSERVICE_H
#define COMSERVICE_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

class ADDataAcquisition : public QObject
{
    Q_OBJECT
public:
    ADDataAcquisition(QObject *parent = nullptr);
    bool InitSerial(QString portname);
    void CloseSerial();
    void ClearSeria();

signals:
    void UpdateSerialData(QByteArray data);

public slots:
    void SendSerialData(QByteArray data);
private slots:
    void ReceviceSerialData();
    void clearAll();
private:
    QSerialPort *mSerial;
    QMutex mSerialMutex;  // 创建一个互斥锁
    QTimer *m_updataTimer;
};

#endif // COMSERVICE_H
