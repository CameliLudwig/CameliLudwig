#ifndef LASERWARNING_H
#define LASERWARNING_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QDebug>

// 通道信号状态 (解析第1字节)
struct ChannelFlags {
    bool ch1_active; // bit 7-6 = 01
    bool ch2_active; // bit 5-4 = 10
    bool ch3_active; // bit 3-2 = 11
};

// 0x04 脉冲告警数据
struct PulseWarningData {
    ChannelFlags flags;      // 第1字节: 通道状态
    quint16 laserParam;      // 第2-3字节: 激光参数 (高字节在前)
    quint8 channelParamReserved; // 第4字节: 激光通道参数 (可保留)
    // 第5-10字节: 保留，结构体中暂不体现，如需存储可用 QByteArray
};

// 0x06 驾束告警数据
struct BeamWarningData {
    ChannelFlags flags;      // 第1字节: 通道状态
    quint8 maxFrequency;     // 第2字节: 所有通道的信号最高频率 (kHz)
    quint16 paramCh1;        // 第3-4字节: 通道1信号参数 (高字节在前) (可保留)
    quint16 paramCh2;        // 第5-6字节: 通道2信号参数 (高字节在前) (可保留)
    quint16 paramCh3;        // 第7-8字节: 通道3信号参数 (高字节在前) (可保留)
};

// 0x02 自检结果
struct SelfCheckResult {
    quint8 statusCh1;
    quint8 statusCh2;
    quint8 statusCh3;
};

class LaserWarning : public QObject
{
    Q_OBJECT
public:
    explicit LaserWarning(QObject *parent = nullptr);
    ~LaserWarning();

    bool connectDevice(const QString &portName);
    bool isConnected() const;
    void disconnectDevice();
    void sendSelfCheckCommand();
    void sendRawData(const QByteArray &data);
    void injectFrame(const QByteArray &rawHex);  // 模拟注入帧数据

signals:
    void selfCheckResultReceived(SelfCheckResult result);
    void pulseWarningReceived(PulseWarningData data);
    void beamWarningReceived(BeamWarningData data);
    void errorOccurred(const QString &msg);

private slots:
    void onReadyRead();

private:
    QSerialPort *m_serial;
    QByteArray m_buffer;

    void parseFrame(const QByteArray &frame);
    quint8 calculateChecksum(const QByteArray &data, int length);
    ChannelFlags parseChannelByte(quint8 byteVal);
};

#endif // LASERWARNING_H
