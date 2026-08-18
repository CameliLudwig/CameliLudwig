#ifndef STM32MODBUSCLIENT_H
#define STM32MODBUSCLIENT_H

#include <QObject>
#include <QSerialPort>
#include <QFile>
#include <QDateTime>

/*
 * Stm32ModbusClient
 * -----------------
 * STM32 数字量接口板 Qt 串口通信封装类
 *
 * 当前按设备真实行为适配：
 * 1. 写命令（05 / 06）响应：8 字节回显，可能尾随多余 00 00
 * 2. 读命令（01 / 03）响应：7 字节
 * 3. 模式控制地址：40400（十进制），即 0x9DD0
 * 4. GPIO / TIM 地址仍按你原始协议中的十进制地址处理
 */
class Stm32ModbusClient : public QObject
{
    Q_OBJECT

public:
    explicit Stm32ModbusClient(QObject *parent = nullptr);
    ~Stm32ModbusClient();

    enum class FunctionCode : quint8
    {
        ReadCoil             = 0x01,
        ReadHoldingRegister  = 0x03,
        WriteCoil            = 0x05,
        WriteHoldingRegister = 0x06
    };
    Q_ENUM(FunctionCode)

    enum class WorkMode : quint16
    {
        SystemTest     = 0x0000,
        AlarmTest      = 0x0001,
        ProtectionTest = 0x0002
    };
    Q_ENUM(WorkMode)

    enum class GpioMode : quint16
    {
        Input  = 0,
        Output = 1
    };
    Q_ENUM(GpioMode)

    enum class LogicLevel : quint16
    {
        Low  = 0,
        High = 1
    };
    Q_ENUM(LogicLevel)

    enum class CrcByteOrder
    {
        HighLow,  // CRC高字节在前
        LowHigh   // CRC低字节在前（标准 Modbus 常见）
    };

    enum class PinId
    {
        PA0,
        PA1,
        PA4,
        PA5,
        PA6,
        PA7,
        PA12,
        PB0,
        PB1,
        PB4,
        PB5,
        PB6,
        PB7,
        PB8,
        PB9,
        PB10,
        PB11,
        PB12,
        PB13,
        PB14,
        PB15,
        PC14,
        PC15
    };
    Q_ENUM(PinId)

    struct PinRegister
    {
        quint16 modeAddr;
        quint16 levelAddr;
    };

public:
    // ----------- 基本配置 -----------
    void setPortName(const QString &portName);
    void setBaudRate(qint32 baudRate = QSerialPort::Baud9600);
    void setSlaveAddress(quint8 slaveAddress);
    void setTimeoutMs(int timeoutMs);
    void setCrcByteOrder(CrcByteOrder order);

    bool open();
    void close();
    bool isOpen() const;

    QString lastErrorString() const;

    // ----------- 日志 -----------
    void setLogFile(const QString &path);
    void closeLogFile();

    // ----------- 协议层基础接口 -----------
    bool readCoil(quint16 coilAddr, bool &value);
    bool writeCoil(quint16 coilAddr, bool value);

    bool readHoldingRegister(quint16 regAddr, quint16 &value);
    bool writeHoldingRegister(quint16 regAddr, quint16 value);

    bool writeUInt32ToHolding(quint16 lowAddr, quint16 highAddr, quint32 value);
    bool readUInt32FromHolding(quint16 lowAddr, quint16 highAddr, quint32 &value);

    // ----------- 模式相关 -----------
    bool setWorkMode(WorkMode mode);
    bool setSystemMode();
    bool setAlarmMode();
    bool setProtectionMode();

    // 先写模式控制地址，再等待固定时间，再回读确认
    bool switchModeAndWait(WorkMode mode, int settleMs = 20000);

    // ----------- 业务层高级接口 -----------
    bool setGpioMode(PinId pin, GpioMode mode);
    bool readGpioMode(PinId pin, GpioMode &mode);

    bool writeGpioLevel(PinId pin, LogicLevel level);
    bool readGpioLevel(PinId pin, LogicLevel &level);

    bool readAlarmTtlInput(int index, bool &value);
    bool setProtectionTtlOutput(int index, bool value);
    bool readProtectionTtlInput(int index, bool &value);
    bool setRelayChannel(int channel, bool on);

    bool setTim1Ch1Pwm(quint32 value);
    bool setTim1Ch4Pwm(quint32 value);

    bool readTim2Ch1Capture(quint32 &value);
    bool readTim2Ch2Capture(quint32 &value);

    // ----------- 静态辅助函数 -----------
    static PinRegister pinRegister(PinId pin);
    static QString pinName(PinId pin);
    static quint16 calculateModbusCrc16(const QByteArray &data);

private:
    QByteArray buildRequestFrame(FunctionCode function,
                                 quint16 regAddr,
                                 quint16 payload) const;

    bool transact(FunctionCode function,
                  quint16 regAddr,
                  quint16 payload,
                  quint16 *responseValue = nullptr);

    bool parseWriteResponse(const QByteArray &frame,
                            FunctionCode expectedFunction,
                            quint16 expectedAddr,
                            quint16 *valueOut) const;

    bool parseReadResponse(const QByteArray &frame,
                           FunctionCode expectedFunction,
                           quint16 *valueOut) const;

    bool isFrameCrcValid(const QByteArray &frame) const;
    void drainInputBuffer(int maxWaitMs = 20);

public:
    void setError(const QString &msg);

    static PinId ttlIoIndexToPin(int index);
    static PinId protectionInputIndexToPin(int index);
    static PinId relayChannelToPin(int channel);

private:
    QSerialPort m_serial;
    QFile m_logFile;
    quint8 m_slaveAddress {0x01};
    int m_timeoutMs {500};
    CrcByteOrder m_crcByteOrder {CrcByteOrder::LowHigh};
    QString m_lastError;

    void writeLog(const QString &line);
    static QString describeFrame(quint8 func, quint16 addr, quint16 data);
};

#endif // STM32MODBUSCLIENT_H
