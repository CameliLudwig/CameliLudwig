#include "ADDataAcquisition.h"
#include <QSerialPortInfo>
#include <qDebug>
#include <QThread>

ADDataAcquisition::ADDataAcquisition(QObject *parent) : QObject(parent)
{
    mSerial = new QSerialPort();

}
//清理缓存
void ADDataAcquisition::clearAll()
{
    QMutexLocker locker(&mSerialMutex);
    mSerial->clear(QSerialPort::Input);  // 清空输入缓存
    mSerial->clear(QSerialPort::Output); // 清空输出缓存

}

bool ADDataAcquisition::InitSerial(QString portname)
{
    mSerial->setPortName(portname);
    if(!mSerial->open(QIODevice::ReadWrite))
    {

        return 0;
    }

    mSerial->setBaudRate(QSerialPort::Baud9600);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setStopBits(QSerialPort::OneStop);
    mSerial->setFlowControl(QSerialPort::NoFlowControl); // 流控制

    m_updataTimer = new QTimer();
    connect(m_updataTimer, &QTimer::timeout, this, &ADDataAcquisition::clearAll);
    // 读取数据
    connect(mSerial,&QSerialPort::readyRead,this,&ADDataAcquisition::ReceviceSerialData);
    m_updataTimer->setInterval(1 * 60 * 1000); // 5分钟（单位：毫秒）
    return 1;
}

void ADDataAcquisition::CloseSerial()
{
    if(mSerial->isOpen())
    {
        m_updataTimer->stop();
        m_updataTimer->deleteLater();
        mSerial->clear();
        mSerial->close();
    }

}

void ADDataAcquisition::ReceviceSerialData()
{
    QByteArray buffer = mSerial->readAll();


    emit  UpdateSerialData(buffer);
}

void ADDataAcquisition::SendSerialData(QByteArray data)
{
    QMutexLocker locker(&mSerialMutex);
    // 接收GUI数据并发送
    mSerial->write(data);
}
void ADDataAcquisition::ClearSeria()
{
    mSerial->clear();
}
