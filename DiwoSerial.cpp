#include "DiwoSerial.h"
#include "applogger.h"
#include <QDebug>

DiwoSerial::DiwoSerial(QObject* parent) : QObject(parent) {}
DiwoSerial::~DiwoSerial() { close(); }

bool DiwoSerial::open(const QString& portName) {
    m_serial = new QSerialPort(portName, this);
    m_serial->setBaudRate(QSerialPort::Baud115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred("DiwoSerial open failed: " + m_serial->errorString());
        delete m_serial; m_serial = nullptr;
        return false;
    }
    connect(m_serial, &QSerialPort::readyRead, this, &DiwoSerial::onReadyRead);
    return true;
}

void DiwoSerial::close() {
    if (m_serial) { m_serial->close(); delete m_serial; m_serial = nullptr; }
    m_buffer.clear();
}

quint8 DiwoSerial::calcChecksum(const QByteArray& data) {
    quint8 cs = 0;
    for (auto b : data) cs ^= (quint8)b;
    return cs;
}

void DiwoSerial::sendSelfCheckCommand() {
    if (!isOpen()) { emit errorOccurred("port not open"); return; }
    // 协议修正: 发送用0x02, 接收用0x01
    QByteArray cmd;
    cmd.append((char)0x3E);
    cmd.append((char)0x3E);
    cmd.append((char)0x02);  // type: self-check send
    cmd.append((char)0x0D);  // len low
    cmd.append((char)0x00);  // len high


    cmd.append((char)0x00);
    cmd.append((char)0x00);
    cmd.append((char)0x00);
    cmd.append((char)0x00);
    cmd.append((char)0x00);

    cmd.append((char)calcChecksum(cmd));
    LOG(QString("Diwo TX: %1").arg(QString::fromLatin1(cmd.toHex(' '))));
    m_serial->write(cmd);
}

void DiwoSerial::sendStatusFrame(quint8 statusByte) {
    if (!isOpen()) { emit errorOccurred("port not open"); return; }
    // 帧: 3E 3E 02 0D 00 [status] 01 01 00 00 00 00 [CS]
    QByteArray frame;
    frame.append((char)0x3E);
    frame.append((char)0x3E);
    frame.append((char)0x02);
    frame.append((char)0x0D);
    frame.append((char)0x00);
    frame.append((char)statusByte);
    frame.append((char)0x01);
    frame.append((char)0x01);
    frame.append((char)0x00);
    frame.append((char)0x00);
    frame.append((char)0x00);
    frame.append((char)0x00);
    frame.append((char)calcChecksum(frame));
    LOG(QString("Diwo TX: %1").arg(QString::fromLatin1(frame.toHex(' '))));
    m_serial->write(frame);
}

void DiwoSerial::onReadyRead() {
    QByteArray raw = m_serial->readAll();
    LOG(QString("Diwo RX: %1").arg(QString::fromLatin1(raw.toHex(' '))));
    m_buffer.append(raw);
    emit dataReceived();  // 收到任何数据就通知
}
