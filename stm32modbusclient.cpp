#include "stm32modbusclient.h"
#include "applogger.h"
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>

namespace
{
constexpr int kWriteResponseLen = 8;
constexpr int kReadResponseLen  = 8;

// ===== 地址映射 =====
// 这里按你当前设备真实行为：控制命令地址走完整十进制 40400
namespace RegAddr
{
    constexpr quint16 ControlCommand = 40400;

    constexpr quint16 Tim1Ch1Low  = 1011;  // 401011
    constexpr quint16 Tim1Ch1High = 1012;  // 401012
    constexpr quint16 Tim1Ch4Low  = 1041;  // 401041
    constexpr quint16 Tim1Ch4High = 1042;  // 401042

    constexpr quint16 Tim2Ch1Low  = 2011;  // 402011
    constexpr quint16 Tim2Ch1High = 2012;  // 402012
    constexpr quint16 Tim2Ch2Low  = 2021;  // 402021
    constexpr quint16 Tim2Ch2High = 2022;  // 402022
}
} // namespace

Stm32ModbusClient::Stm32ModbusClient(QObject *parent)
    : QObject(parent)
{
    m_serial.setBaudRate(QSerialPort::Baud9600);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);
}

Stm32ModbusClient::~Stm32ModbusClient()
{
    close();
}

void Stm32ModbusClient::setPortName(const QString &portName)
{
    m_serial.setPortName(portName);
}

void Stm32ModbusClient::setBaudRate(qint32 baudRate)
{
    m_serial.setBaudRate(baudRate);
}

void Stm32ModbusClient::setSlaveAddress(quint8 slaveAddress)
{
    m_slaveAddress = slaveAddress;
}

void Stm32ModbusClient::setTimeoutMs(int timeoutMs)
{
    m_timeoutMs = timeoutMs;
}

void Stm32ModbusClient::setCrcByteOrder(CrcByteOrder order)
{
    m_crcByteOrder = order;
}

bool Stm32ModbusClient::open()
{
    if (m_serial.isOpen())
    {
        return true;
    }

    if (!m_serial.open(QIODevice::ReadWrite))
    {
        setError(QStringLiteral("串口打开失败：%1").arg(m_serial.errorString()));
        return false;
    }

    m_lastError.clear();
    return true;
}

void Stm32ModbusClient::close()
{
    closeLogFile();
    if (m_serial.isOpen())
    {
        m_serial.close();
    }
}

bool Stm32ModbusClient::isOpen() const
{
    return m_serial.isOpen();
}

QString Stm32ModbusClient::lastErrorString() const
{
    return m_lastError;
}

void Stm32ModbusClient::setError(const QString &msg)
{
    m_lastError = msg;
}

// ==================== 日志 ====================

void Stm32ModbusClient::setLogFile(const QString &path)
{
    if (m_logFile.isOpen())
        m_logFile.close();

    m_logFile.setFileName(path);
    if (!m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        qWarning().noquote() << QStringLiteral("无法打开日志文件: %1").arg(path);
        return;
    }

    const QString header = QStringLiteral("\r\n========== 日志开始 %1 ==========\r\n\r\n")
                               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    m_logFile.write(header.toUtf8());
    m_logFile.flush();
}

void Stm32ModbusClient::closeLogFile()
{
    if (m_logFile.isOpen())
    {
        const QString footer = QStringLiteral("\r\n========== 日志结束 %1 ==========\r\n")
                                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
        m_logFile.write(footer.toUtf8());
        m_logFile.close();
    }
}

void Stm32ModbusClient::writeLog(const QString &line)
{
    if (!m_logFile.isOpen()) return;
    m_logFile.write(line.toUtf8());
    m_logFile.flush();
}

QString Stm32ModbusClient::describeFrame(quint8 func, quint16 addr, quint16 data)
{
    switch (func)
    {
    case 0x01: return QString("读线圈 %1").arg(addr, 5, 10, QChar('0'));
    case 0x03: return QString("读保持寄存器 %1").arg(addr, 5, 10, QChar('0'));
    case 0x05:
        return QString("写线圈 %1 = %2").arg(addr, 5, 10, QChar('0')).arg(data);
    case 0x06:
        return QString("写保持寄存器 %1 = 0x%2").arg(addr, 5, 10, QChar('0')).arg(data, 4, 16, QChar('0'));
    default:  return QString("功能码 0x%1 地址=%2").arg(func, 2, 16, QChar('0')).arg(addr);
    }
}

void Stm32ModbusClient::drainInputBuffer(int maxWaitMs)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray junk;

    // 只清“已经到达”的残留，不用 clear() 粗暴清口
    while (timer.elapsed() < maxWaitMs)
    {
        if (m_serial.bytesAvailable() > 0 || m_serial.waitForReadyRead(5))
        {
            junk.append(m_serial.readAll());
        }
        else
        {
            break;
        }
    }

    if (!junk.isEmpty())
    {
        qInfo().noquote() << "Drop(stale) =" << junk.toHex(' ');
    }
}

quint16 Stm32ModbusClient::calculateModbusCrc16(const QByteArray &data)
{
    quint16 crc = 0xFFFF;

    for (const char byte : data)
    {
        crc ^= static_cast<quint8>(byte);

        for (int i = 0; i < 8; ++i)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

QByteArray Stm32ModbusClient::buildRequestFrame(FunctionCode function,
                                                quint16 regAddr,
                                                quint16 payload) const
{
    QByteArray frame;
    frame.reserve(8);

    frame.append(static_cast<char>(m_slaveAddress));
    frame.append(static_cast<char>(function));
    frame.append(static_cast<char>((regAddr >> 8) & 0xFF));
    frame.append(static_cast<char>(regAddr & 0xFF));
    frame.append(static_cast<char>((payload >> 8) & 0xFF));
    frame.append(static_cast<char>(payload & 0xFF));

    const quint16 crc = calculateModbusCrc16(frame);
    const quint8 crcHigh = static_cast<quint8>((crc >> 8) & 0xFF);
    const quint8 crcLow  = static_cast<quint8>(crc & 0xFF);

    if (m_crcByteOrder == CrcByteOrder::HighLow)
    {
        frame.append(static_cast<char>(crcHigh));
        frame.append(static_cast<char>(crcLow));
    }
    else
    {
        frame.append(static_cast<char>(crcLow));
        frame.append(static_cast<char>(crcHigh));
    }

    return frame;
}

bool Stm32ModbusClient::isFrameCrcValid(const QByteArray &frame) const
{
    if (frame.size() < 4)
    {
        return false;
    }

    const QByteArray body = frame.left(frame.size() - 2);
    const quint16 crcCalc = calculateModbusCrc16(body);

    quint8 crcHighRecv = 0;
    quint8 crcLowRecv = 0;

    if (m_crcByteOrder == CrcByteOrder::HighLow)
    {
        crcHighRecv = static_cast<quint8>(frame.at(frame.size() - 2));
        crcLowRecv  = static_cast<quint8>(frame.at(frame.size() - 1));
    }
    else
    {
        crcLowRecv  = static_cast<quint8>(frame.at(frame.size() - 2));
        crcHighRecv = static_cast<quint8>(frame.at(frame.size() - 1));
    }

    const quint16 crcRecv =
            (static_cast<quint16>(crcHighRecv) << 8)
            | static_cast<quint16>(crcLowRecv);

    return crcRecv == crcCalc;
}

bool Stm32ModbusClient::parseWriteResponse(const QByteArray &frame,
                                           FunctionCode expectedFunction,
                                           quint16 expectedAddr,
                                           quint16 *valueOut) const
{
    if (frame.size() != kWriteResponseLen)
    {
        return false;
    }

    if (!isFrameCrcValid(frame))
    {
        return false;
    }

    const quint8 slave = static_cast<quint8>(frame.at(0));
    if (slave != m_slaveAddress)
    {
        return false;
    }

    const quint8 functionValue = static_cast<quint8>(frame.at(1));
    if (functionValue != static_cast<quint8>(expectedFunction))
    {
        return false;
    }

    const quint16 addr =
            (static_cast<quint16>(static_cast<quint8>(frame.at(2))) << 8)
            | static_cast<quint8>(frame.at(3));

    if (addr != expectedAddr)
    {
        return false;
    }

    if (valueOut)
    {
        const quint16 data =
                (static_cast<quint16>(static_cast<quint8>(frame.at(4))) << 8)
                | static_cast<quint8>(frame.at(5));
        *valueOut = data;
    }

    return true;
}

bool Stm32ModbusClient::parseReadResponse(const QByteArray &frame,
                                          FunctionCode expectedFunction,
                                          quint16 *valueOut) const
{
    if (frame.size() != kReadResponseLen)
    {
        return false;
    }

    if (!isFrameCrcValid(frame))
    {
        return false;
    }

    const quint8 slave = static_cast<quint8>(frame.at(0));
    if (slave != m_slaveAddress)
    {
        return false;
    }

    const quint8 functionValue = static_cast<quint8>(frame.at(1));
    if (functionValue != static_cast<quint8>(expectedFunction))
    {
        return false;
    }

    // 设备实际返回 8 字节回显：01 01 00 01 00 00 CRC CRC
    // frame[2:3] = 地址回显, frame[4:5] = 16位数据
    if (valueOut)
    {
        const quint16 data =
                (static_cast<quint16>(static_cast<quint8>(frame.at(4))) << 8)
                | static_cast<quint8>(frame.at(5));
        *valueOut = data;
    }

    return true;
}

bool Stm32ModbusClient::transact(FunctionCode function,
                                 quint16 regAddr,
                                 quint16 payload,
                                 quint16 *responseValue)
{
    if (!m_serial.isOpen())
    {
        setError(QStringLiteral("串口未打开"));
        return false;
    }

    // 发新命令前先把旧残留读掉
    drainInputBuffer();

    const QByteArray request = buildRequestFrame(function, regAddr, payload);
    LOG(QString("Modbus TX: %1 [Fn=0x%2 Addr=%3 Payload=%4]").arg(QString::fromLatin1(request.toHex(' '))).arg(static_cast<quint8>(function), 2, 16, QChar('0')).arg(regAddr, 4, 16, QChar('0')).arg(payload, 4, 16, QChar('0')));

    // 写日志：TX
    const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    writeLog(QString("%1 TX %2  [%3]\r\n")
                 .arg(ts, request.toHex(' ').constData(),
                      describeFrame(static_cast<quint8>(function), regAddr, payload)));

    const qint64 written = m_serial.write(request);
    if (written != request.size())
    {
        setError(QStringLiteral("串口写入失败：已写入 %1 字节，期望 %2 字节")
                 .arg(written)
                 .arg(request.size()));
        return false;
    }

    if (!m_serial.waitForBytesWritten(m_timeoutMs))
    {
        setError(QStringLiteral("串口写超时：%1").arg(m_serial.errorString()));
        return false;
    }

    QByteArray buffer;
    QElapsedTimer timer;
    timer.start();

    const bool isRead = (function == FunctionCode::ReadCoil ||
                         function == FunctionCode::ReadHoldingRegister);
    const int expectedLen = isRead ? kReadResponseLen : kWriteResponseLen;

    while (timer.elapsed() < m_timeoutMs)
    {
        const int remain = m_timeoutMs - static_cast<int>(timer.elapsed());
        if (remain <= 0)
        {
            break;
        }

        if (!m_serial.waitForReadyRead(remain))
        {
            continue;
        }

        buffer.append(m_serial.readAll());
        LOG(QString("Modbus RX raw: %1").arg(QString::fromLatin1(buffer.toHex(' '))));

        // 在缓冲区里滑动找帧，兼容前导 00、尾随 00 00
        for (int i = 0; i + expectedLen <= buffer.size(); ++i)
        {
            const QByteArray frame = buffer.mid(i, expectedLen);
            quint16 value = 0;
            bool ok = false;

            if (isRead)
            {
                ok = parseReadResponse(frame, function, &value);
            }
            else
            {
                ok = parseWriteResponse(frame, function, regAddr, &value);
            }

            if (ok)
            {
                if (responseValue)
                {
                    *responseValue = value;
                }

                LOG(QString("Modbus RX frame: %1").arg(QString::fromLatin1(frame.toHex(' '))));

                // 写日志：RX
                const QString ts2 = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
                const quint16 rxAddr =
                        (static_cast<quint16>(static_cast<quint8>(frame.at(2))) << 8)
                        | static_cast<quint8>(frame.at(3));
                writeLog(QString("%1 RX %2  [回显: %3]\r\n")
                             .arg(ts2, frame.toHex(' ').constData(),
                                  describeFrame(static_cast<quint8>(function), rxAddr, value)));
                writeLog("\r\n");

                return true;
            }
        }

        // 防止 buffer 无限制增长
        if (buffer.size() > 128)
        {
            buffer = buffer.right(32);
        }
    }

    setError(QStringLiteral("响应超时或数据不足：收到 %1 字节，期望 %2 字节")
             .arg(buffer.size())
             .arg(expectedLen));
    LOG(QString("Modbus 超时: %1").arg(m_lastError));
    writeLog(QString("%1 !! 超时 (%2) [%3]\r\n\r\n")
                 .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"),
                      m_lastError,
                      describeFrame(static_cast<quint8>(function), regAddr, payload)));
    return false;
}

// ==================== 基础接口 ====================

bool Stm32ModbusClient::readCoil(quint16 coilAddr, bool &value)
{
    quint16 responseValue = 0;
    if (!transact(FunctionCode::ReadCoil, coilAddr, 0x0000, &responseValue))
    {
        return false;
    }

    value = (responseValue != 0);
    return true;
}

bool Stm32ModbusClient::writeCoil(quint16 coilAddr, bool value)
{
    const quint16 payload = value ? 0x0001 : 0x0000;
    return transact(FunctionCode::WriteCoil, coilAddr, payload, nullptr);
}

bool Stm32ModbusClient::readHoldingRegister(quint16 regAddr, quint16 &value)
{
    quint16 responseValue = 0;
    if (!transact(FunctionCode::ReadHoldingRegister, regAddr, 0x0000, &responseValue))
    {
        return false;
    }

    value = responseValue;
    return true;
}

bool Stm32ModbusClient::writeHoldingRegister(quint16 regAddr, quint16 value)
{
    return transact(FunctionCode::WriteHoldingRegister, regAddr, value, nullptr);
}

bool Stm32ModbusClient::writeUInt32ToHolding(quint16 lowAddr, quint16 highAddr, quint32 value)
{
    const quint16 lowWord  = static_cast<quint16>(value & 0xFFFFu);
    const quint16 highWord = static_cast<quint16>((value >> 16) & 0xFFFFu);

    if (!writeHoldingRegister(lowAddr, lowWord))
    {
        return false;
    }

    if (!writeHoldingRegister(highAddr, highWord))
    {
        return false;
    }

    return true;
}

bool Stm32ModbusClient::readUInt32FromHolding(quint16 lowAddr, quint16 highAddr, quint32 &value)
{
    quint16 lowWord = 0;
    quint16 highWord = 0;

    if (!readHoldingRegister(lowAddr, lowWord))
    {
        return false;
    }

    if (!readHoldingRegister(highAddr, highWord))
    {
        return false;
    }

    value = (static_cast<quint32>(highWord) << 16) | static_cast<quint32>(lowWord);
    return true;
}

// ==================== 模式接口 ====================

bool Stm32ModbusClient::setWorkMode(WorkMode mode)
{
    return writeHoldingRegister(RegAddr::ControlCommand, static_cast<quint16>(mode));
}

bool Stm32ModbusClient::switchModeAndWait(WorkMode mode, int settleMs)
{
    if (!setWorkMode(mode))
    {
        return false;
    }

    qInfo().noquote() << QStringLiteral("[等待] 等待模式生效，等待 %1 秒...")
                         .arg(settleMs / 1000);
    QThread::msleep(static_cast<unsigned long>(settleMs));

    quint16 readBack = 0;
    if (!readHoldingRegister(RegAddr::ControlCommand, readBack))
    {
        setError(QStringLiteral("回读模式地址失败：%1").arg(lastErrorString()));
        return false;
    }

    if (readBack != static_cast<quint16>(mode))
    {
        setError(QStringLiteral("模式确认失败，期望=%1，实际=%2")
                 .arg(static_cast<quint16>(mode))
                 .arg(readBack));
        return false;
    }

    return true;
}

bool Stm32ModbusClient::setSystemMode()
{
    return switchModeAndWait(WorkMode::SystemTest, 20000);
}

bool Stm32ModbusClient::setAlarmMode()
{
    return switchModeAndWait(WorkMode::AlarmTest, 20000);
}

bool Stm32ModbusClient::setProtectionMode()
{
    return switchModeAndWait(WorkMode::ProtectionTest, 20000);
}

// ==================== 业务接口 ====================

bool Stm32ModbusClient::setGpioMode(PinId pin, GpioMode mode)
{
    const PinRegister reg = pinRegister(pin);
    if (reg.modeAddr == 0)
    {
        setError(QStringLiteral("无效 GPIO 模式地址：%1").arg(pinName(pin)));
        return false;
    }

    return writeHoldingRegister(reg.modeAddr, static_cast<quint16>(mode));
}

bool Stm32ModbusClient::readGpioMode(PinId pin, GpioMode &mode)
{
    const PinRegister reg = pinRegister(pin);
    if (reg.modeAddr == 0)
    {
        setError(QStringLiteral("无效 GPIO 模式地址：%1").arg(pinName(pin)));
        return false;
    }

    quint16 value = 0;
    if (!readHoldingRegister(reg.modeAddr, value))
    {
        return false;
    }

    mode = (value == 0) ? GpioMode::Input : GpioMode::Output;
    return true;
}

bool Stm32ModbusClient::writeGpioLevel(PinId pin, LogicLevel level)
{
    const PinRegister reg = pinRegister(pin);
    if (reg.levelAddr == 0)
    {
        setError(QStringLiteral("无效 GPIO 电平地址：%1").arg(pinName(pin)));
        return false;
    }

    return writeCoil(reg.levelAddr, level == LogicLevel::High);
}

bool Stm32ModbusClient::readGpioLevel(PinId pin, LogicLevel &level)
{
    const PinRegister reg = pinRegister(pin);
    if (reg.levelAddr == 0)
    {
        setError(QStringLiteral("无效 GPIO 电平地址：%1").arg(pinName(pin)));
        return false;
    }

    bool value = false;
    if (!readCoil(reg.levelAddr, value))
    {
        return false;
    }

    level = value ? LogicLevel::High : LogicLevel::Low;
    return true;
}

bool Stm32ModbusClient::readAlarmTtlInput(int index, bool &value)
{
    const PinId pin = ttlIoIndexToPin(index);
    LogicLevel level = LogicLevel::Low;

    if (!readGpioLevel(pin, level))
    {
        return false;
    }

    value = (level == LogicLevel::High);
    return true;
}

bool Stm32ModbusClient::setProtectionTtlOutput(int index, bool value)
{
    const PinId pin = ttlIoIndexToPin(index);
    return writeGpioLevel(pin, value ? LogicLevel::High : LogicLevel::Low);
}

bool Stm32ModbusClient::readProtectionTtlInput(int index, bool &value)
{
    const PinId pin = protectionInputIndexToPin(index);
    LogicLevel level = LogicLevel::Low;

    if (!readGpioLevel(pin, level))
    {
        return false;
    }

    value = (level == LogicLevel::High);
    return true;
}

bool Stm32ModbusClient::setRelayChannel(int channel, bool on)
{
    if (channel < 1 || channel > 14)
    {
        setError(QStringLiteral("继电器通道号越界，合法范围为 1~14"));
        return false;
    }

    const PinId pin = relayChannelToPin(channel);
    return writeGpioLevel(pin, on ? LogicLevel::High : LogicLevel::Low);
}

bool Stm32ModbusClient::setTim1Ch1Pwm(quint32 value)
{
    return writeUInt32ToHolding(RegAddr::Tim1Ch1Low, RegAddr::Tim1Ch1High, value);
}

bool Stm32ModbusClient::setTim1Ch4Pwm(quint32 value)
{
    return writeUInt32ToHolding(RegAddr::Tim1Ch4Low, RegAddr::Tim1Ch4High, value);
}

bool Stm32ModbusClient::readTim2Ch1Capture(quint32 &value)
{
    return readUInt32FromHolding(RegAddr::Tim2Ch1Low, RegAddr::Tim2Ch1High, value);
}

bool Stm32ModbusClient::readTim2Ch2Capture(quint32 &value)
{
    return readUInt32FromHolding(RegAddr::Tim2Ch2Low, RegAddr::Tim2Ch2High, value);
}

// ==================== 地址映射 ====================

Stm32ModbusClient::PinRegister Stm32ModbusClient::pinRegister(PinId pin)
{
    switch (pin)
    {
    case PinId::PA0:  return { 40001,   1 };
    case PinId::PA1:  return { 40002,   2 };
    case PinId::PA4:  return { 40005,   5 };
    case PinId::PA5:  return { 40006,   6 };
    case PinId::PA6:  return { 40007,   7 };
    case PinId::PA7:  return { 40008,   8 };
    case PinId::PA12: return { 40013,  13 };

    case PinId::PB0:  return { 40101, 101 };
    case PinId::PB1:  return { 40102, 102 };
    case PinId::PB4:  return { 40105, 105 };
    case PinId::PB5:  return { 40106, 106 };
    case PinId::PB6:  return { 40107, 107 };
    case PinId::PB7:  return { 40108, 108 };
    case PinId::PB8:  return { 40109, 109 };
    case PinId::PB9:  return { 40110, 110 };
    case PinId::PB10: return { 40111, 111 };
    case PinId::PB11: return { 40112, 112 };
    case PinId::PB12: return { 40113, 113 };
    case PinId::PB13: return { 40114, 114 };
    case PinId::PB14: return { 40115, 115 };
    case PinId::PB15: return { 40116, 116 };

    case PinId::PC14: return { 40115, 115 };
    case PinId::PC15: return { 40116, 116 };
    }

    return {0, 0};
}

QString Stm32ModbusClient::pinName(PinId pin)
{
    switch (pin)
    {
    case PinId::PA0:  return QStringLiteral("PA0");
    case PinId::PA1:  return QStringLiteral("PA1");
    case PinId::PA4:  return QStringLiteral("PA4");
    case PinId::PA5:  return QStringLiteral("PA5");
    case PinId::PA6:  return QStringLiteral("PA6");
    case PinId::PA7:  return QStringLiteral("PA7");
    case PinId::PA12: return QStringLiteral("PA12");
    case PinId::PB0:  return QStringLiteral("PB0");
    case PinId::PB1:  return QStringLiteral("PB1");
    case PinId::PB4:  return QStringLiteral("PB4");
    case PinId::PB5:  return QStringLiteral("PB5");
    case PinId::PB6:  return QStringLiteral("PB6");
    case PinId::PB7:  return QStringLiteral("PB7");
    case PinId::PB8:  return QStringLiteral("PB8");
    case PinId::PB9:  return QStringLiteral("PB9");
    case PinId::PB10: return QStringLiteral("PB10");
    case PinId::PB11: return QStringLiteral("PB11");
    case PinId::PB12: return QStringLiteral("PB12");
    case PinId::PB13: return QStringLiteral("PB13");
    case PinId::PB14: return QStringLiteral("PB14");
    case PinId::PB15: return QStringLiteral("PB15");
    case PinId::PC14: return QStringLiteral("PC14");
    case PinId::PC15: return QStringLiteral("PC15");
    }

    return QStringLiteral("UNKNOWN");
}

// ==================== 索引辅助 ====================

Stm32ModbusClient::PinId Stm32ModbusClient::ttlIoIndexToPin(int index)
{
    switch (index)
    {
    case 1: return PinId::PA4;
    case 2: return PinId::PA5;
    case 3: return PinId::PA6;
    case 4: return PinId::PA7;
    default: return PinId::PA4;
    }
}

Stm32ModbusClient::PinId Stm32ModbusClient::protectionInputIndexToPin(int index)
{
    switch (index)
    {
    case 1: return PinId::PC14;
    case 2: return PinId::PC15;
    default: return PinId::PC14;
    }
}

Stm32ModbusClient::PinId Stm32ModbusClient::relayChannelToPin(int channel)
{
    switch (channel)
    {
    case 1:  return PinId::PB10;
    case 2:  return PinId::PB11;
    case 3:  return PinId::PB9;
    case 4:  return PinId::PB8;
    case 5:  return PinId::PB7;
    case 6:  return PinId::PB6;
    case 7:  return PinId::PA12;
    case 8:  return PinId::PB5;
    case 9:  return PinId::PB4;
    case 10: return PinId::PB15;
    case 11: return PinId::PB14;
    case 12: return PinId::PB13;
    case 13: return PinId::PB12;
    case 14: return PinId::PB1;
    default: return PinId::PA12;
    }
}
