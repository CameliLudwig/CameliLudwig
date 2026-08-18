#include "radarboard.h"
#include "applogger.h"
#include <QDebug>

static const quint8 FRAME_HEAD1      = 0xAA;
static const quint8 FRAME_HEAD2      = 0x55;
static const quint8 FRAME_TAIL       = 0x10;

static const int    RX_FRAME_LEN     = 8;   // 板卡 -> 上位机
static const int    TX_FRAME_LEN     = 9;   // 上位机 -> 板卡

TestBoardSerial::TestBoardSerial(const QString &portName, QObject *parent)
    : QObject(parent)
{
    m_port.setPortName(portName);
    m_port.setBaudRate(QSerialPort::Baud9600);
    m_port.setDataBits(QSerialPort::Data8);
    m_port.setParity(QSerialPort::NoParity);
    m_port.setStopBits(QSerialPort::OneStop);
    m_port.setFlowControl(QSerialPort::NoFlowControl);

    connect(&m_port, &QSerialPort::readyRead,
            this, &TestBoardSerial::onReadyRead);
    connect(&m_port, &QSerialPort::errorOccurred,
            this, [this](QSerialPort::SerialPortError e){
        if (e == QSerialPort::NoError) return;
        emit errorOccurred(QStringLiteral("Serial error: %1").arg(m_port.errorString()));
    });
}

bool TestBoardSerial::open()
{
    if (m_port.isOpen())
        return true;

    if (!m_port.open(QIODevice::ReadWrite)) {
        emit errorOccurred(QStringLiteral("Cannot open port %1: %2")
                           .arg(m_port.portName(), m_port.errorString()));
        return false;
    }
    return true;
}

void TestBoardSerial::close()
{
    if (m_port.isOpen())
        m_port.close();
}

// ---------------- RX ----------------

void TestBoardSerial::onReadyRead()
{
    m_recvBuffer.append(m_port.readAll());
    processBuffer();
}

void TestBoardSerial::processBuffer()
{
    // 简单状态机：查找头 AA 55 和尾 10，长度 8
    while (m_recvBuffer.size() >= RX_FRAME_LEN) {
        int startIndex = -1;

        // 找到 0xAA 0x55 头
        for (int i = 0; i + 1 < m_recvBuffer.size(); ++i) {
            if ((quint8)m_recvBuffer.at(i)     == FRAME_HEAD1 &&
                (quint8)m_recvBuffer.at(i + 1) == FRAME_HEAD2) {
                startIndex = i;
                break;
            }
        }

        if (startIndex < 0) {
            // 没有头，整个缓存丢弃
            m_recvBuffer.clear();
            return;
        }

        // 丢弃头之前的无效数据
        if (startIndex > 0)
            m_recvBuffer.remove(0, startIndex);

        if (m_recvBuffer.size() < RX_FRAME_LEN)
            return; // 等待更多数据

        // 检查尾字节
        if ((quint8)m_recvBuffer.at(RX_FRAME_LEN - 1) != FRAME_TAIL) {
            // 头对了尾不对，认为是错误帧，丢弃一个字节继续找
            m_recvBuffer.remove(0, 1);
            continue;
        }

        // 取出一帧
        QByteArray frame = m_recvBuffer.left(RX_FRAME_LEN);
        m_recvBuffer.remove(0, RX_FRAME_LEN);

        LOG(QString("Radar RX: %1").arg(QString::fromLatin1(frame.toHex(' '))));
        emit rawFrameReceived(frame);

        quint8 ch1Low  = (quint8)frame.at(2);
        quint8 ch1High = (quint8)frame.at(3);
        quint8 ch2Low  = (quint8)frame.at(4);
        quint8 ch2High = (quint8)frame.at(5);
        quint8 status  = (quint8)frame.at(6);

        quint16 ch1Amp = (quint16)ch1Low  | ((quint16)ch1High << 8);
        quint16 ch2Amp = (quint16)ch2Low  | ((quint16)ch2High << 8);

        emit amplitudesReceived(ch1Amp, ch2Amp, status);
    }
}
void TestBoardSerial::setPortName(const QString &name)
{
    if (m_port.isOpen()) {
        m_port.close();
    }
    m_port.setPortName(name);
}
// ---------------- TX ----------------

void TestBoardSerial::sendTriggerCommand(bool enableTrigger,
                                         quint8 priUs,
                                         quint8 pwUs,
                                         quint8 thrS1,
                                         quint8 thrS2,
                                         quint8 channelSelect)
{
    if (!m_port.isOpen()) {
        emit errorOccurred(QStringLiteral("Port %1 is not open").arg(m_port.portName()));
        return;
    }

    QByteArray frame;
    frame.resize(TX_FRAME_LEN);

    frame[0] = (char)FRAME_HEAD1;
    frame[1] = (char)FRAME_HEAD2;
    frame[2] = (char)(enableTrigger ? 0x01 : 0x00); // 触发使能
    frame[3] = (char)priUs;       // PRI, 1us 单位
    frame[4] = (char)pwUs;        // PW,  1us 单位
    frame[5] = (char)thrS1;       // THR_S1
    frame[6] = (char)thrS2;       // THR_S2
    frame[7] = (char)channelSelect; // CHANNAL_S
    frame[8] = (char)FRAME_TAIL;

    LOG(QString("Radar TX: %1").arg(QString::fromLatin1(frame.toHex(' '))));
    qint64 written = m_port.write(frame);
    if (written != frame.size()) {
        emit errorOccurred(QStringLiteral("Failed to write frame, written = %1").arg(written));
    }

    qDebug() << "TX frame:" << frame.toHex(' ');

}
