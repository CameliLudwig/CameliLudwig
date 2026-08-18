#include "laserwarning.h"
#include "applogger.h"

LaserWarning::LaserWarning(QObject *parent) : QObject(parent)
{
    m_serial = new QSerialPort(this);
    // 串口配置：115200, 8数据位, 1停止位, 偶校验(Even)
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::EvenParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(m_serial, &QSerialPort::readyRead, this, &LaserWarning::onReadyRead);
}

LaserWarning::~LaserWarning()
{
    if (m_serial->isOpen()) m_serial->close();
}

bool LaserWarning::connectDevice(const QString &portName)
{
    if (m_serial->isOpen()) m_serial->close();
    m_serial->setPortName(portName);
    return m_serial->open(QIODevice::ReadWrite);
}

void LaserWarning::disconnectDevice()
{
    if (m_serial->isOpen()) m_serial->close();
}

bool LaserWarning::isConnected() const
{
    return m_serial->isOpen();
}

void LaserWarning::sendRawData(const QByteArray &data)
{
    if (!m_serial->isOpen()) {
        LOG(QString("LASER[%1] 发送失败: 串口未打开").arg(m_serial->portName()));
        return;
    }
    LOG(QString("LASER[%1] TX: %2").arg(m_serial->portName(), QString::fromLatin1(data.toHex())));
    m_serial->write(data);
}

void LaserWarning::sendSelfCheckCommand()
{
    if (!m_serial->isOpen()) return;

    QByteArray packet;
    packet.append(static_cast<char>(0xF0));
    packet.append(static_cast<char>(0xF0));
    packet.append(static_cast<char>(0x01)); // 类型：自检
    packet.append(static_cast<char>(0x06)); // 长度低位 (假设总长6)
    packet.append(static_cast<char>(0x00)); // 长度高位

    packet.append(static_cast<char>(calculateChecksum(packet, packet.size()))); // 校验和
    LOG(QString("LASER[%1] TX: %2").arg(m_serial->portName(), QString::fromLatin1(packet.toHex())));
    m_serial->write(packet);
}

// 模拟注入帧数据 (直接走解析流程, 不走串口)
void LaserWarning::injectFrame(const QByteArray &rawHex)
{
    m_buffer.append(rawHex);
    // 复用onReadyRead的解析逻辑 (不读串口, 直接处理buffer)
    while (m_buffer.size() >= 6) {
        int headIdx = m_buffer.indexOf(QByteArray::fromHex("F0F0"));
        if (headIdx == -1) { m_buffer.clear(); return; }
        if (headIdx > 0) m_buffer.remove(0, headIdx);
        if (m_buffer.size() < 6) return;

        quint8 lenLow  = static_cast<quint8>(m_buffer.at(3));
        quint8 lenHigh = static_cast<quint8>(m_buffer.at(4));
        int packetLen = (lenHigh << 8) | lenLow;
        if (m_buffer.size() < packetLen) return;

        QByteArray frame = m_buffer.mid(0, packetLen);
        m_buffer.remove(0, packetLen);

        quint8 recvSum = static_cast<quint8>(frame.at(packetLen - 1));
        quint8 calcSum = calculateChecksum(frame, packetLen - 1);
        if (recvSum == calcSum) {
            parseFrame(frame);
        }
    }
}

quint8 LaserWarning::calculateChecksum(const QByteArray &data, int length)
{
    quint8 checksum = 0;
    for (int i = 0; i < length; ++i) {
        // 使用 .at() 避免老版本 Qt 报错
        checksum ^= static_cast<quint8>(data.at(i));
    }
    return checksum;
}

// 解析第1字节的通道位定义
ChannelFlags LaserWarning::parseChannelByte(quint8 byteVal)
{
    ChannelFlags flags;
    // 7-6 bit: 01=通道1有信号 (二进制 01 = 十进制 1)
    flags.ch1_active = ((byteVal >> 6) & 0x03) == 0x01;
    // 5-4 bit: 10=通道2有信号 (二进制 10 = 十进制 2)
    flags.ch2_active = ((byteVal >> 4) & 0x03) == 0x02;
    // 3-2 bit: 11=通道3有信号 (二进制 11 = 十进制 3)
    flags.ch3_active = ((byteVal >> 2) & 0x03) == 0x03;
    // 1-0 bit: 保留
    return flags;
}

void LaserWarning::onReadyRead()
{
    QByteArray raw = m_serial->readAll();
    LOG(QString("LASER[%1] RX(raw): %2").arg(m_serial->portName(), QString::fromLatin1(raw.toHex())));
    m_buffer.append(raw);

    while (m_buffer.size() >= 6) {
        int headIdx = m_buffer.indexOf(QByteArray::fromHex("F0F0"));
        if (headIdx == -1) {
            m_buffer.clear();
            return;
        }
        if (headIdx > 0) m_buffer.remove(0, headIdx);

        if (m_buffer.size() < 6) return;

        quint8 lenLow = static_cast<quint8>(m_buffer.at(3));
        quint8 lenHigh = static_cast<quint8>(m_buffer.at(4));
        int packetLen = (lenHigh << 8) | lenLow;

        if (m_buffer.size() < packetLen) return;

        QByteArray frame = m_buffer.mid(0, packetLen);
        m_buffer.remove(0, packetLen);

        quint8 recvSum = static_cast<quint8>(frame.at(packetLen - 1));
        quint8 calcSum = calculateChecksum(frame, packetLen - 1);

        if (recvSum == calcSum) {
            LOG(QString("LASER[%1] 完整帧: %2").arg(m_serial->portName(), QString::fromLatin1(frame.toHex())));
            parseFrame(frame);
        } else {
            LOG(QString("LASER[%1] 校验失败: recv=%2 calc=%3").arg(m_serial->portName()).arg(recvSum).arg(calcSum));
        }
    }
}

void LaserWarning::parseFrame(const QByteArray &frame)
{
    quint8 type = static_cast<quint8>(frame.at(2));
    // 数据区从第5个字节开始 (Frame Header 2 + Type 1 + Len 2 = 5)
    const int DATA_OFFSET = 5;

    // 安全检查：防止访问越界
    // frame.size() 包含了最后的校验位，所以实际有效数据长度应减去 Header(5) 和 Checksum(1)

    switch (type) {
    case 0x02: // 自检结果
    {
        if (frame.size() < DATA_OFFSET + 3) break;
        SelfCheckResult res;
        res.statusCh1 = static_cast<quint8>(frame.at(DATA_OFFSET + 0));
        res.statusCh2 = static_cast<quint8>(frame.at(DATA_OFFSET + 1));
        res.statusCh3 = static_cast<quint8>(frame.at(DATA_OFFSET + 2));
        emit selfCheckResultReceived(res);
        break;
    }
    case 0x04: // 脉冲激光告警信息
    {
        // 根据描述：第1字节通道，第2-3字节参数，第4字节参数，第5-10字节参数
        // 至少需要 10 字节的数据区
        if (frame.size() < DATA_OFFSET + 10) break;

        PulseWarningData data;

        // 第1字节：通道状态位
        data.flags = parseChannelByte(static_cast<quint8>(frame.at(DATA_OFFSET + 0)));

        // 第2-3字节：激光参数（高字节在前，低字节在后）
        data.laserParam = (static_cast<quint8>(frame.at(DATA_OFFSET + 1)) << 8) |
                           static_cast<quint8>(frame.at(DATA_OFFSET + 2));

        // 第4字节：激光通道参数（可保留）
        data.channelParamReserved = static_cast<quint8>(frame.at(DATA_OFFSET + 3));

        // 第5-10字节：激光通道参数（保留），这里未解析

        emit pulseWarningReceived(data);
        break;
    }
    case 0x06: // 驾束激光告警信息
    {
        // 根据描述：第1字节通道，第2字节频率，第3-4、5-6、7-8参数
        // 至少需要 8 字节的数据区
        if (frame.size() < DATA_OFFSET + 8) break;

        BeamWarningData data;

        // 第1字节：通道编号
        data.flags = parseChannelByte(static_cast<quint8>(frame.at(DATA_OFFSET + 0)));

        // 第2字节：所有通道的信号最高频率（kHZ）
        data.maxFrequency = static_cast<quint8>(frame.at(DATA_OFFSET + 1));

        // 第3-4字节：通道1信号参数（高字节在前）
        data.paramCh1 = (static_cast<quint8>(frame.at(DATA_OFFSET + 2)) << 8) |
                         static_cast<quint8>(frame.at(DATA_OFFSET + 3));

        // 第5-6字节：通道2信号参数（高字节在前）
        data.paramCh2 = (static_cast<quint8>(frame.at(DATA_OFFSET + 4)) << 8) |
                         static_cast<quint8>(frame.at(DATA_OFFSET + 5));

        // 第7-8字节：通道3信号参数（高字节在前）
        data.paramCh3 = (static_cast<quint8>(frame.at(DATA_OFFSET + 6)) << 8) |
                         static_cast<quint8>(frame.at(DATA_OFFSET + 7));

        emit beamWarningReceived(data);
        break;
    }
    default:
        // qDebug() << "Unknown type:" << type;
        break;
    }
}
