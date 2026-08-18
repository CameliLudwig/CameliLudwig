#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QTimer>
#include <QButtonGroup>
#include <QSet>
#include <QMap>
#include <QDateTime>
#include <QList>
#include "radarboard.h"
#include "digitaliocontroller.h"
#include "flexraythread.h"
#include "laserwarning.h"
#include "DiwoSerial.h"
#include "pdfreportgenerator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 测试模式枚举（与 TestSelectionDialog 保持一致）
    enum TestMode {
        ModeRadarBoard,   // 毫米波告警测试
        ModeDiwo,         // 敌我识别测试
        ModeLaser,        // 激光告警测试
        ModeSmoke,        // 烟幕弹测试
        ModeFireSuppress, // 灭火抑爆测试
        ModeTopComponent // 顶部组件测试
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static uint16_t SetBit(uint16 data, int bitPos, int value);

    // 设置测试模式（切换可见性，重置状态追踪）
    void setTestMode(TestMode mode);

    // 关闭所有串口设备
    void closeAllDevices();

    // 设置当前用户
    void setCurrentUser(const QString &user) { m_currentUser = user; }
    QString currentUser() const { return m_currentUser; }

    // 生成会话PDF报告
    QString generateSessionPDF();

    // 测试模式名称
    QString testModeName() const;

    TestMode testMode() const { return m_testMode; }

private slots:
    void on_pushButton_open_clicked();
    void on_pushButton_All_clicked();
    void on_pushButton_SelectAll_clicked();
    void on_pushButton_Cancel_clicked();
    void on_pushButton_WithdrawalOfAmmunition_clicked();
    void on_pushButton_close_clicked();

    void show_data(QVector<uint16> rcvData, int channel);
    void on_pushButton_SelfDestruct_clicked();
    void on_pushButton_SendSelfTestResults_2_clicked();
    void on_pushButton_diwoselfcheck_clicked();
    void on_pushButton_radiaclose_clicked();


    void on_pushButton_check_clicked();
    void on_pushButton_top_clicked();     // 顶部组件发送按钮
    void onTopSerialReadyRead();          // 顶部组件串口接收

    //void on_pushButton_2_clicked();

signals:
    void flexSend(QVector<uint16> masterData, QVector<uint16> driveData);
    void requestReturn();  // 用户点击返回，请求回到测试选择界面

private:
    void initFlexRayConnections();
    void initDevices();
    void startBridgeToggleDetect();

    // 根据测试模式控制界面可见性
    void applyVisibilityForMode();

    // PDF报告数据收集
    QList<ReportRow> collectReportRows() const;
    QList<ReportRow> collectRadarBoardRows() const;
    QList<ReportRow> collectDiwoRows() const;
    QList<ReportRow> collectLaserRows() const;
    QList<ReportRow> collectSmokeRows() const;
    QList<ReportRow> collectFireSuppressRows() const;
    QList<ReportRow> collectTopComponentRows() const;

    Ui::MainWindow *ui;
    TestBoardSerial *m_radar;
    Stm32ModbusClient *m_modbus = nullptr;
    DigitalIOController *m_dio = nullptr;
    FlexRayThread *m_flexRayThread = nullptr;
    LaserWarning *m_laserWarning = nullptr;
    LaserWarning *m_laserWarning2 = nullptr;
    QButtonGroup *m_laserSimGroup = nullptr;
    DiwoSerial *m_diwo = nullptr;
    QSerialPort *m_topSerial = nullptr;   // 顶部组件串口 (groupBox_5)
    QByteArray m_topSerialBuffer;          // 顶部组件接收缓冲

    int m_flexRayTxCounter = 0;
    bool m_abNodeAOk = true;
    bool m_abNodeBOk = true;
    int m_radarCycleIdx = 0;
    quint8 m_radarPriUs = 0;
    quint8 m_radarPwUs = 0;
    QVector<QByteArray> m_radarFrames;
    int m_cnt_SelfCheck = 0;
    bool m_bSelfDestructToggled = false;
    bool m_isReplenishing = false;
    QVector<double> m_TemprcvData1 = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    QVector<uint16> m_TemprcvData2 = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
    QVector<uint16> m_TemprcvData4 = {0xFFFF, 0xFFFF};

    // ========== 测试模式与会话信息 ==========
    TestMode m_testMode;
    QString m_currentUser;
    QDateTime m_sessionStartTime;
    QDateTime m_sessionEndTime;

    // ========== 状态追踪（用于PDF报告判定）==========

    // 雷达板卡：记录 lineEdit_2 中出现过的所有方向文字
    QSet<QString> m_radarDirectionsSeen;

    // 敌我识别：记录每个指示灯出现过的值（"0"/"1"）
    QMap<QString, QSet<QString>> m_diwoStates;

    // 激光告警：记录已点亮过的指示灯名称
    QSet<QString> m_laserIndicatorsLit;

    // 烟幕弹：记录每个烟幕弹指示灯出现过的值
    QMap<QString, QSet<QString>> m_smokeStates;

    // 灭火抑爆：记录指示灯出现过的值
    QSet<QString> m_fireSuppressValues;

    // 顶部组件：记录指示灯出现过的值
    QSet<QString> m_topComponentValues;
};

#endif // MAINWINDOW_H
