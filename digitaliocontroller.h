#ifndef DIGITALIOCONTROLLER_H
#define DIGITALIOCONTROLLER_H

#include <QObject>
#include "stm32modbusclient.h"

/*
 * ============================================================================
 * DigitalIOController  ---  数字量接口板上层控制类
 * ============================================================================
 *
 * 功能概述:
 *   1. 工作模式切换  ---  通过 MODBUS 40400 寄存器设置系统/告警/防护三种模式
 *   2. 通道电平控制  ---  14路继电器通道(CH1~CH14)的读写操作（仅防护模式可用）
 *   3. 前八位批量控制 ---  CH1~CH8 整字节批量设置/读取/翻转/流水灯测试
 *
 * 设计约束:
 *   - 通道控制操作（setChannel / setFirst8Channels 等）仅在防护测试模式下可用，
 *     非防护模式自动拒绝并返回 false，lastError() 可查看原因。
 *   - 模式切换通过写 40400 命令实现，写回显即确认，不做额外回读验证。
 *
 * 通道映射（前八位 bit0=CH1 ... bit7=CH8）:
 *   CH1=PB10, CH2=PB11, CH3=PB9,  CH4=PB8,
 *   CH5=PB7,  CH6=PB6,  CH7=PA12, CH8=PB5
 *
 * 后六路映射:
 *   CH9=PB4,   CH10=PB15, CH11=PB14, CH12=PB13,
 *   CH13=PB12, CH14=PB1
 *
 * 依赖:
 *   - Stm32ModbusClient  (底层 MODBUS RTU 通信)
 *   - 必须先打开串口:  client.setPortName("COM5"); client.open();
 *
 * ============================================================================
 * 使用示例:
 * ============================================================================
 *
 * // --- 示例1: 基本初始化 ---
 * Stm32ModbusClient client;
 * client.setPortName("COM5");
 * client.setBaudRate(QSerialPort::Baud9600);
 * client.setSlaveAddress(0x01);
 * client.open();
 *
 * DigitalIOController io(client);
 *
 * // --- 示例2: 切换到防护测试模式 ---
 * if (io.setProtectionMode()) {
 *     qInfo() << "当前模式:" << io.currentModeName();  // "防护测试"
 * }
 *
 * // --- 示例3: 单独设置通道 ---
 * io.setChannel(1, true);   // CH1 置高
 * io.setChannel(3, false);  // CH3 置低
 *
 * // --- 示例4: 前八位批量控制 ---
 * // 设置 CH1~CH8 = 10101010 (交替)
 * io.setFirst8Channels(0xAA);
 *
 * // 读取前八位状态
 * quint8 bits = 0;
 * io.readFirst8Channels(bits);
 * qInfo() << "八位:" << bits;  // 0xAA = 170
 *
 * // --- 示例5: 翻转指定通道 ---
 * io.toggleChannel(5);          // CH5 翻转
 * io.toggleFirst8Channels(0x0F); // CH1~CH4 同时翻转
 *
 * // --- 示例6: 流水灯测试 ---
 * io.walkFirst8Channels();  // CH1~CH8 逐一点亮再熄灭
 *
 * // --- 示例7: 全部14路控制 ---
 * io.setChannelBits(0x3FFF);  // 14路全高
 * io.setChannelBits(0x0000);  // 14路全低
 * io.setChannelBits(0x0055);  // CH1/CH3/CH5/CH7 高，其余低
 *
 * // --- 示例8: 错误处理 ---
 * if (!io.setChannel(1, true)) {
 *     qCritical() << "失败:" << io.lastError();
 *     // 可能原因: 未切换到防护模式 / 串口通信异常
 * }
 *
 * // --- 示例9: 非防护模式拒绝 ---
 * io.setSystemMode();
 * bool ok = io.setChannel(1, true);  // ok == false
 * // lastError() == "通道控制仅在防护测试模式下可用"
 *
 * ============================================================================
 */

class DigitalIOController : public QObject
{
    Q_OBJECT

public:
    // 构造，传入已打开串口的 Stm32ModbusClient 引用
    explicit DigitalIOController(Stm32ModbusClient &client, QObject *parent = nullptr);

    // ========================= 模式选择 =========================
    // 通过 MODBUS 40400 寄存器切换工作模式
    // 40400 = 0x0000 → 系统测试
    // 40400 = 0x0001 → 告警测试
    // 40400 = 0x0002 → 防护测试
    bool setWorkMode(Stm32ModbusClient::WorkMode mode);
    bool setSystemMode();          // 便捷方法：切换系统测试模式
    bool setAlarmMode();           // 便捷方法：切换告警测试模式
    bool setProtectionMode();      // 便捷方法：切换防护测试模式

    Stm32ModbusClient::WorkMode currentMode() const;   // 获取当前模式枚举
    QString currentModeName() const;                     // 获取当前模式中文名

    // ========================= 单独通道控制 =========================
    // 仅防护模式下可用，否则返回 false
    // ch: 通道号 1~14
    // high: true=高电平, false=低电平
    bool setChannel(int ch, bool high);
    bool readChannel(int ch, bool &high);
    bool toggleChannel(int ch);    // 翻转指定通道电平

    // ========================= 前八位批量控制 =========================
    // 仅防护模式下可用，否则返回 false
    // bits: 单字节，bit0=CH1, bit1=CH2 ... bit7=CH8
    // 例: 0xAA = 10101010 → CH1/3/5/7高, CH2/4/6/8低
    bool setFirst8Channels(quint8 bits);
    bool readFirst8Channels(quint8 &bits);
    bool toggleFirst8Channels(quint8 mask);  // mask 中置1的位翻转

    // ========================= 前八位流水灯测试 =========================
    // CH1→CH8 逐个点亮再熄灭，仅防护模式
    bool walkFirst8Channels();

    // ========================= 全部14路控制 =========================
    // 仅防护模式下可用，否则返回 false
    // bits: bit0=CH1, bit1=CH2 ... bit13=CH14
    bool setChannelBits(quint16 bits);
    bool readChannelBits(quint16 &bits);

    // ========================= 错误信息 =========================
    QString lastError() const;

private:
    bool ensureProtectionMode();   // 内部权限检查：非防护模式拒绝并设 m_lastError

    Stm32ModbusClient &m_client;
    Stm32ModbusClient::WorkMode m_currentMode {Stm32ModbusClient::WorkMode::SystemTest};
    QString m_lastError;
};

#endif // DIGITALIOCONTROLLER_H
