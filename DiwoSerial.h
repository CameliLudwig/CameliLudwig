#ifndef DIWOSERIAL_H
#define DIWOSERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class DiwoSerial : public QObject
{
    Q_OBJECT
public:
    explicit DiwoSerial(QObject* parent = nullptr);
    ~DiwoSerial();

    bool open(const QString& portName);
    void close();
    bool isOpen() const { return m_serial && m_serial->isOpen(); }
    void sendSelfCheckCommand();
    void sendStatusFrame(quint8 statusByte);  // 发送状态帧 3E 3E 02 0D 00 [status] 01 01 00 00 00 00 [CS]

signals:
    void dataReceived();     // 收到任何数据就发
    void errorOccurred(const QString& msg);

private slots:
    void onReadyRead();

private:
    QSerialPort* m_serial = nullptr;
    QByteArray m_buffer;
    quint8 calcChecksum(const QByteArray& data);
};

#endif
