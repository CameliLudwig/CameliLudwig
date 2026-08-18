#ifndef RADARBOARD_H
#define RADARBOARD_H

#include <QObject>
#include <QtSerialPort/QSerialPort>

class TestBoardSerial : public QObject
{
    Q_OBJECT
public:
    explicit TestBoardSerial(const QString &portName,
                             QObject *parent = nullptr);

    bool open();
    void close();
    void setPortName(const QString &name);

    bool isOpen() const { return m_port.isOpen(); }

    // 发送一帧“触发配置命令”
    // priUs / pwUs 单位都是 1us，对应协议里的 8bit 数值
    void sendTriggerCommand(bool enableTrigger,
                            quint8 priUs,
                            quint8 pwUs,
                            quint8 thrS1,
                            quint8 thrS2,
                            quint8 channelSelect);

signals:
    // 解析到一帧板卡数据
    void amplitudesReceived(quint16 ch1Amplitude,
                            quint16 ch2Amplitude,
                            quint8  statusByte);  // 0x11 正常，其他故障

    // 收到完整原始帧（8 字节），方便调试
    void rawFrameReceived(const QByteArray &frame);

    // 串口或协议错误
    void errorOccurred(const QString &message);

private slots:
    void onReadyRead();

private:
    QSerialPort m_port;
    QByteArray  m_recvBuffer;

    void processBuffer();
};


#endif // RADARBOARD_H
