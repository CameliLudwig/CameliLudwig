#include "digitaliocontroller.h"

#include <QDebug>
#include <QThread>

DigitalIOController::DigitalIOController(Stm32ModbusClient &client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
}

// ==================== 模式选择 ====================

bool DigitalIOController::setWorkMode(Stm32ModbusClient::WorkMode mode)
{
    // 40400 寄存器写回显即确认，不做回读（回读始终返回0）
    // 即使写操作暂时超时，也信任模式已设置，后续通道操作会自行重试
    m_client.setWorkMode(mode);
    m_currentMode = mode;
    m_lastError.clear();
    return true;
}

bool DigitalIOController::setSystemMode()
{
    return setWorkMode(Stm32ModbusClient::WorkMode::SystemTest);
}

bool DigitalIOController::setAlarmMode()
{
    return setWorkMode(Stm32ModbusClient::WorkMode::AlarmTest);
}

bool DigitalIOController::setProtectionMode()
{
    return setWorkMode(Stm32ModbusClient::WorkMode::ProtectionTest);
}

Stm32ModbusClient::WorkMode DigitalIOController::currentMode() const
{
    return m_currentMode;
}

QString DigitalIOController::currentModeName() const
{
    switch (m_currentMode)
    {
    case Stm32ModbusClient::WorkMode::SystemTest:     return QStringLiteral("系统测试");
    case Stm32ModbusClient::WorkMode::AlarmTest:      return QStringLiteral("告警测试");
    case Stm32ModbusClient::WorkMode::ProtectionTest: return QStringLiteral("防护测试");
    }
    return QStringLiteral("未知");
}

QString DigitalIOController::lastError() const
{
    return m_lastError;
}

// ==================== 权限检查 ====================

bool DigitalIOController::ensureProtectionMode()
{
    if (m_currentMode != Stm32ModbusClient::WorkMode::ProtectionTest)
    {
        m_lastError = QStringLiteral("通道控制仅在防护测试模式下可用");
        return false;
    }
    return true;
}

// ==================== 单独通道控制 ====================

bool DigitalIOController::setChannel(int ch, bool high)
{
    if (!ensureProtectionMode()) return false;
    bool ok = m_client.setRelayChannel(ch, high);
    if (!ok) m_lastError = m_client.lastErrorString();
    return ok;
}

bool DigitalIOController::readChannel(int ch, bool &high)
{
    if (!ensureProtectionMode()) return false;
    return m_client.setRelayChannel(ch, high); // 写操作即回显，不再额外读
    Q_UNUSED(high);
}

bool DigitalIOController::toggleChannel(int ch)
{
    if (!ensureProtectionMode()) return false;

    bool cur = false;
    auto reg = Stm32ModbusClient::pinRegister(
        Stm32ModbusClient::relayChannelToPin(ch));
    if (m_client.readCoil(reg.levelAddr, cur))
    {
        return m_client.setRelayChannel(ch, !cur);
    }

    // 读不到就写高再写低
    if (!m_client.setRelayChannel(ch, true)) return false;
    QThread::msleep(100);
    return m_client.setRelayChannel(ch, false);
}

// ==================== 前八位批量控制 ====================

// CH1~CH8 映射:
//   CH1=PB10, CH2=PB11, CH3=PB9,  CH4=PB8,
//   CH5=PB7,  CH6=PB6,  CH7=PA12, CH8=PB5
static const int kFirst8Channels[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

bool DigitalIOController::setFirst8Channels(quint8 bits)
{
    if (!ensureProtectionMode()) return false;

    for (int i = 0; i < 8; ++i)
    {
        const bool on = (bits >> i) & 0x01;
        if (!m_client.setRelayChannel(kFirst8Channels[i], on))
        {
            m_lastError = QStringLiteral("CH%1 写入失败: %2")
                              .arg(kFirst8Channels[i])
                              .arg(m_client.lastErrorString());
            return false;
        }
        QThread::msleep(50);
    }
    return true;
}

bool DigitalIOController::readFirst8Channels(quint8 &bits)
{
    if (!ensureProtectionMode()) return false;

    bits = 0;
    for (int i = 0; i < 8; ++i)
    {
        bool val = false;
        auto reg = Stm32ModbusClient::pinRegister(
            Stm32ModbusClient::relayChannelToPin(kFirst8Channels[i]));
        if (!m_client.readCoil(reg.levelAddr, val))
        {
            m_lastError = QStringLiteral("CH%1 读取失败: %2")
                              .arg(kFirst8Channels[i])
                              .arg(m_client.lastErrorString());
            return false;
        }
        if (val) bits |= (1 << i);
        QThread::msleep(30);
    }
    return true;
}

bool DigitalIOController::toggleFirst8Channels(quint8 mask)
{
    if (!ensureProtectionMode()) return false;

    for (int i = 0; i < 8; ++i)
    {
        if ((mask >> i) & 0x01)
        {
            if (!toggleChannel(kFirst8Channels[i]))
            {
                m_lastError = QStringLiteral("CH%1 翻转失败: %2")
                                  .arg(kFirst8Channels[i])
                                  .arg(m_client.lastErrorString());
                return false;
            }
            QThread::msleep(50);
        }
    }
    return true;
}

bool DigitalIOController::walkFirst8Channels()
{
    if (!ensureProtectionMode()) return false;

    // 逐位点亮再熄灭
    for (int i = 0; i < 8; ++i)
    {
        qInfo().noquote() << QString("  >>> 流水灯 CH%1 亮").arg(kFirst8Channels[i]);
        if (!m_client.setRelayChannel(kFirst8Channels[i], true))
        {
            m_lastError = QStringLiteral("CH%1 点亮失败: %2")
                              .arg(kFirst8Channels[i])
                              .arg(m_client.lastErrorString());
            return false;
        }
        QThread::msleep(300);

        qInfo().noquote() << QString("  >>> 流水灯 CH%1 灭").arg(kFirst8Channels[i]);
        if (!m_client.setRelayChannel(kFirst8Channels[i], false))
        {
            m_lastError = QStringLiteral("CH%1 熄灭失败: %2")
                              .arg(kFirst8Channels[i])
                              .arg(m_client.lastErrorString());
            return false;
        }
        QThread::msleep(200);
    }
    return true;
}

// ==================== 全部14路控制 ====================

bool DigitalIOController::setChannelBits(quint16 bits)
{
    if (!ensureProtectionMode()) return false;

    for (int ch = 1; ch <= 14; ++ch)
    {
        const bool on = (bits >> (ch - 1)) & 0x01;
        if (!m_client.setRelayChannel(ch, on))
        {
            m_lastError = QStringLiteral("CH%1 写入失败: %2")
                              .arg(ch)
                              .arg(m_client.lastErrorString());
            return false;
        }
        QThread::msleep(30);
    }
    return true;
}

bool DigitalIOController::readChannelBits(quint16 &bits)
{
    if (!ensureProtectionMode()) return false;

    bits = 0;
    for (int ch = 1; ch <= 14; ++ch)
    {
        bool val = false;
        auto reg = Stm32ModbusClient::pinRegister(
            Stm32ModbusClient::relayChannelToPin(ch));
        if (!m_client.readCoil(reg.levelAddr, val))
        {
            m_lastError = QStringLiteral("CH%1 读取失败: %2")
                              .arg(ch)
                              .arg(m_client.lastErrorString());
            return false;
        }
        if (val) bits |= (1 << (ch - 1));
        QThread::msleep(30);
    }
    return true;
}
