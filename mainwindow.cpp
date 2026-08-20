#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "applogger.h"
#include "QObject"
#include "QDebug"
#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QSharedPointer>
#include <QRadioButton>
#include <QSet>
#include <QMessageBox>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QMetaObject>

uint16 MainWindow::SetBit(uint16 data, int bitPos, int value)
{
    if (value)
        return data | (1 << bitPos);
    else
        return data & ~(1 << bitPos);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_SelfDestruct->hide();

    // 默认设为雷达板卡模式（仅为了让界面有初始状态，后续 setTestMode 会覆盖）
    m_testMode = ModeRadarBoard;
    m_sessionStartTime = QDateTime::currentDateTime();
    m_sessionEndTime = m_sessionStartTime;

    // 初始化敌我识别状态追踪
    m_diwoStates["主机"] = QSet<QString>();
    m_diwoStates["询问"] = QSet<QString>();
    m_diwoStates["应答"] = QSet<QString>();

    // 初始化烟幕弹状态追踪
    const char* smokeNames[] = {"左1","左2","左3","左4","右1","右2","右3","右4"};
    for (const char* name : smokeNames) {
        m_smokeStates[QString::fromUtf8(name)] = QSet<QString>();
    }

    // 先隐藏所有，等 setTestMode() 时再按模式显示
    applyVisibilityForMode();

    LOG("=== 程序启动（等待测试模式选择）===");
    setWindowTitle("防护采集驱动装置检测软件");

    // 加载样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // 雷达板卡按钮初始状态: 打开可用, 关闭禁用, 通道可选
    ui->pushButton_open->setEnabled(true);
    ui->pushButton_radiaclose->setEnabled(false);

    // 雷达板卡 (不阻塞)
    m_radar = new TestBoardSerial("COM4");
    QObject::connect(m_radar, &TestBoardSerial::errorOccurred, [](const QString &msg){
            qDebug() << "串口发生错误:" << msg;
            LOG("雷达串口错误: " + msg);
        });
    QObject::connect(m_radar, &TestBoardSerial::rawFrameReceived,
                     this, [this](const QByteArray &frame) {
        if (m_radarCycleIdx > 0 && m_radarCycleIdx <= 8) {
            m_radarFrames.append(frame);
            LOG(QString("雷达通道%1: %2").arg(m_radarCycleIdx - 1)
                .arg(QString::fromLatin1(frame.toHex(' '))));
        }
    });
    LOG("雷达板卡 COM4 已创建");

    // 激光告警对象 (不连接, 延后)
    m_laserWarning = new LaserWarning(this);
    QObject::connect(m_laserWarning, &LaserWarning::errorOccurred, [](const QString &msg){
            qDebug() << "激光告警COM1错误:" << msg;
            LOG("激光告警COM1错误: " + msg);
        });
    m_laserWarning2 = new LaserWarning(this);
    QObject::connect(m_laserWarning2, &LaserWarning::errorOccurred, [](const QString &msg){
            qDebug() << "激光告警COM2错误:" << msg;
            LOG("激光告警com2错误: " + msg);
        });

    // FlexRay信号连接 (不启动)
    initFlexRayConnections();

    // 顶部组件发送按钮
    if (ui->pushButton) {
        connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::on_pushButton_top_clicked);
    }

    // radioButton 互斥分组
    m_laserSimGroup = new QButtonGroup(this);
    m_laserSimGroup->addButton(ui->radioButton_maichong);
    m_laserSimGroup->addButton(ui->radioButton_jiashu);
    ui->radioButton_maichong->setChecked(true);

    // 所有阻塞操作延后到事件循环, 界面先出来
    QTimer::singleShot(100, this, [this]() {
        initDevices();
    });

    LOG("=== MainWindow 构造完成（设备初始化已调度）===");
}

MainWindow::~MainWindow()
{
    closeAllDevices();
    delete ui;
}

void MainWindow::closeAllDevices()
{
    LOG("=== 开始关闭所有设备 ===");

    // 停止并停用由 MainWindow 管理的 FlexRay 定时器（仅重连定时器）
    if (m_flexray_reconnect_timer) {
        m_flexray_reconnect_timer->stop();
    }

    // 1. 停止 FlexRay
    if (m_flexRayThread) {
        LOG("停止 FlexRay...");
        // 请求停止（通过 queued 调用进入 worker 线程）
        QMetaObject::invokeMethod(m_flexRayThread, "stop", Qt::QueuedConnection);

        // 等待 worker 状态变为不运行（m_running==false），分步等待以便记录进度（总计 15s）
        const int stepMs = 1000;
        const int maxTotalMs = 15000;
        int waited = 0;
        bool exited = false;
        while (waited < maxTotalMs) {
            if (!m_flexRayThread->isRunning()) {
                exited = true;
                break;
            }
            waited += stepMs;
            LOG(QString("等待 FlexRay 退出... %1/%2 ms").arg(waited).arg(maxTotalMs));
            QCoreApplication::processEvents();
            QThread::msleep(stepMs);
        }

        if (!exited) {
            // 超时，询问用户是否强制终止
            QMessageBox::StandardButton btn = QMessageBox::warning(this,
                "FlexRay 线程未退出",
                "FlexRay 线程在 15 秒内未退出。\n强制终止可能导致资源泄露或不稳定。\n是否要强制终止？",
                QMessageBox::Abort | QMessageBox::Retry,
                QMessageBox::Retry);

            if (btn == QMessageBox::Abort) {
                LOG("用户选择强制终止 FlexRay 线程");
                // 强制终止 worker 所在的 QThread（最后手段）
                if (m_flexRayWorkerThread) {
                    m_flexRayWorkerThread->terminate();
                    if (m_flexRayWorkerThread->wait(2000)) {
                        LOG("强制终止后 FlexRay worker 线程已退出");
                    } else {
                        LOG("强制终止后 FlexRay worker 线程仍未退出");
                    }
                }
            } else {
                // 再等 10s（用户选择再等等）
                LOG("用户选择再等待 10 秒");
                int extraWait = 10000;
                int waitedExtra = 0;
                while (waitedExtra < extraWait) {
                    if (!m_flexRayThread->isRunning()) break;
                    QCoreApplication::processEvents();
                    QThread::msleep(500);
                    waitedExtra += 500;
                }
                if (m_flexRayThread->isRunning()) {
                    LOG("额外等待后仍未退出，准备强制终止 worker 线程");
                    if (m_flexRayWorkerThread) {
                        m_flexRayWorkerThread->terminate();
                        m_flexRayWorkerThread->wait(2000);
                    }
                }
            }
        }

        // 在删除 worker 对象前，若有后台启动任务在运行，短等其完成以避免 use-after-free
//        if (m_flexray_start_future.isValid()) {
//            // poll for up to 2000 ms
//            int waitedMs = 0;
//            const int step = 50;
//            while (!m_flexray_start_future.isFinished() && waitedMs < 2000) {
//                QThread::msleep(step);
//                QCoreApplication::processEvents();
//                waitedMs += step;
//            }
//            if (m_flexray_start_future.isFinished()) {
//                LOG("后台启动任务已完成");
//            } else {
//                LOG("后台启动任务仍在运行（等待 2s 后仍未完成），将继续删除 worker 对象，可能不安全");
//            }
//        }

        // 删除 worker 对象并清理线程
        if (m_flexRayWorkerThread) {
            // 停止事件循环并等待线程退出（给予短超时）
            m_flexRayWorkerThread->quit();
            if (!m_flexRayWorkerThread->wait(1000)) {
                LOG("警告: worker 线程在 quit 后仍未退出");
            }
        }

        delete m_flexRayThread;
        m_flexRayThread = nullptr;
        if (m_flexRayWorkerThread) {
            delete m_flexRayWorkerThread;
            m_flexRayWorkerThread = nullptr;
        }

        LOG("FlexRay 已停止");
    }

    // 2. 关闭敌我识别
    if (m_diwo) {
        LOG("关闭敌我识别...");
        m_diwo->close();
        delete m_diwo;
        m_diwo = nullptr;
        LOG("敌我识别已关闭");
    }

    // 3. 关闭激光告警
    if (m_laserWarning) {
        LOG("关闭激光告警 COM1...");
        m_laserWarning->disconnectDevice();
    }
    if (m_laserWarning2) {
        LOG("关闭激光告警 COM2...");
        m_laserWarning2->disconnectDevice();
    }

    // 4. 关闭雷达
    if (m_radar) {
        LOG("关闭雷达串口...");
        m_radar->close();
    }

    // 5. 关闭 DIO / MODBUS
    if (m_dio) {
        LOG("关闭 DIO...");
        delete m_dio;
        m_dio = nullptr;
    }
    if (m_modbus) {
        LOG("关闭 MODBUS...");
        m_modbus->close();
        delete m_modbus;
        m_modbus = nullptr;
    }

    // 6. 关闭顶部组件串口
    if (m_topSerial) {
        LOG("关闭顶部组件串口...");
        m_topSerial->close();
        delete m_topSerial;
        m_topSerial = nullptr;
    }

    LOG("=== 所有设备已关闭 ===");
}

void MainWindow::setTestMode(TestMode mode)
{
    m_testMode = mode;
    m_sessionStartTime = QDateTime::currentDateTime();
    m_sessionEndTime = m_sessionStartTime;

    // 重置状态追踪
    m_radarDirectionsSeen.clear();
    m_diwoStates["主机"].clear();
    m_diwoStates["询问"].clear();
    m_diwoStates["应答"].clear();
    m_laserIndicatorsLit.clear();
    const char* smokeNames[] = {"左1","左2","左3","左4","右1","右2","右3","右4"};
    for (const char* name : smokeNames) {
        m_smokeStates[QString::fromUtf8(name)].clear();
    }
    m_fireSuppressValues.clear();
    m_topComponentValues.clear();

    applyVisibilityForMode();
    LOG(QString("测试模式切换为: %1").arg(testModeName()));
}

QString MainWindow::testModeName() const
{
    switch (m_testMode) {
    case ModeRadarBoard: return QString::fromUtf8("毫米波告警测试");
    case ModeDiwo:       return QString::fromUtf8("敌我识别测试");
    case ModeLaser:      return QString::fromUtf8("激光告警测试");
    case ModeSmoke:      return QString::fromUtf8("烟幕弹测试");
    case ModeFireSuppress: return QString::fromUtf8("灭火抑爆测试");
    case ModeTopComponent:   return QString::fromUtf8("顶部组件测试");
    default:             return QString::fromUtf8("未知");
    }
}

// =================== 测试模式可见性控制 ===================
void MainWindow::applyVisibilityForMode()
{
    // 第一步：隐藏所有顶层 groupBox
    ui->groupBox_6->setVisible(false);
    ui->groupBox_2->setVisible(false);
    ui->groupBox->setVisible(false);
    ui->groupBox_10->setVisible(false);
    ui->groupBox_11->setVisible(false);

    // groupBox_3/4 — 默认隐藏
    if (ui->groupBox_3) ui->groupBox_3->setVisible(false);
    if (ui->groupBox_4) ui->groupBox_4->setVisible(false);
    if (ui->groupBox_5) ui->groupBox_5->setVisible(false);

    // groupBox_39 (FlexRay通信状态) — 始终显示
    if (ui->groupBox_39) ui->groupBox_39->setVisible(true);

    // 隐藏 groupBox_11 内的所有子组件
    // gridLayout_17 中的组件
    if (ui->label_35) ui->label_35->setVisible(false);
    if (ui->lineEdit_2) ui->lineEdit_2->setVisible(false);
    // verticalLayout 中的激光告警指示灯
    if (ui->label_2) ui->label_2->setVisible(false);
    if (ui->lineEdit_jgLeftFront) ui->lineEdit_jgLeftFront->setVisible(false);
    if (ui->label_3) ui->label_3->setVisible(false);
    if (ui->lineEdit_jgRightFront) ui->lineEdit_jgRightFront->setVisible(false);
    if (ui->label_4) ui->label_4->setVisible(false);
    if (ui->lineEdit_jgLeftRear) ui->lineEdit_jgLeftRear->setVisible(false);
    if (ui->label_5) ui->label_5->setVisible(false);
    if (ui->lineEdit_jgRightRear) ui->lineEdit_jgRightRear->setVisible(false);
    // 烟幕弹指示灯（在 groupBox_11 中）
    if (ui->label_9)  ui->label_9->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletl1) ui->lineEdit_SmokeScreenBulletl1->setVisible(false);
    if (ui->label_10) ui->label_10->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletl2) ui->lineEdit_SmokeScreenBulletl2->setVisible(false);
    if (ui->label_11) ui->label_11->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletl3) ui->lineEdit_SmokeScreenBulletl3->setVisible(false);
    if (ui->label_12) ui->label_12->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletl4) ui->lineEdit_SmokeScreenBulletl4->setVisible(false);
    if (ui->label_8)  ui->label_8->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletr1) ui->lineEdit_SmokeScreenBulletr1->setVisible(false);
    if (ui->label_13) ui->label_13->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletr2) ui->lineEdit_SmokeScreenBulletr2->setVisible(false);
    if (ui->label_15) ui->label_15->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletr3) ui->lineEdit_SmokeScreenBulletr3->setVisible(false);
    if (ui->label_18) ui->label_18->setVisible(false);
    if (ui->lineEdit_SmokeScreenBulletr4) ui->lineEdit_SmokeScreenBulletr4->setVisible(false);

    // 第二步：根据模式显示对应的组件
    switch (m_testMode) {
    case ModeRadarBoard:
        // 显示雷达板卡 + 驾驶员规避方位
        ui->groupBox_6->setVisible(true);
        ui->groupBox_11->setVisible(true);
        if (ui->label_35) ui->label_35->setVisible(true);
        if (ui->lineEdit_2) ui->lineEdit_2->setVisible(true);
        LOG("可见性: 毫米波告警");
        break;

    case ModeDiwo:
        // 只显示敌我识别，不显示 verticalLayout
        ui->groupBox_2->setVisible(true);
        ui->groupBox_11->setVisible(true);
        LOG("可见性: 敌我识别测试");
        break;

    case ModeLaser:
        // 显示激光告警控制区 + 激光告警4个指示灯
        ui->groupBox->setVisible(true);
        ui->groupBox_11->setVisible(true);
        if (ui->label_2) ui->label_2->setVisible(true);
        if (ui->lineEdit_jgLeftFront) ui->lineEdit_jgLeftFront->setVisible(true);
        if (ui->label_3) ui->label_3->setVisible(true);
        if (ui->lineEdit_jgRightFront) ui->lineEdit_jgRightFront->setVisible(true);
        if (ui->label_4) ui->label_4->setVisible(true);
        if (ui->lineEdit_jgLeftRear) ui->lineEdit_jgLeftRear->setVisible(true);
        if (ui->label_5) ui->label_5->setVisible(true);
        if (ui->lineEdit_jgRightRear) ui->lineEdit_jgRightRear->setVisible(true);
        LOG("可见性: 激光告警测试");
        break;

    case ModeSmoke:
        // 显示烟幕弹发射装置 + 雷达板卡 + 车长终端(仅烟幕弹指示灯)
        ui->groupBox_10->setVisible(true);
        ui->groupBox_6->setVisible(true);
        ui->groupBox_11->setVisible(true);
        // 注意：groupBox_39 在上方已设为 visible=true，这里不需要再设置
        // 隐藏 gridLayout_17(规避方位), verticalLayout(激光告警灯)
        if (ui->label_35) ui->label_35->setVisible(false);
        if (ui->lineEdit_2) ui->lineEdit_2->setVisible(false);
        if (ui->label_2) ui->label_2->setVisible(false);
        if (ui->lineEdit_jgLeftFront) ui->lineEdit_jgLeftFront->setVisible(false);
        if (ui->label_3) ui->label_3->setVisible(false);
        if (ui->lineEdit_jgRightFront) ui->lineEdit_jgRightFront->setVisible(false);
        if (ui->label_4) ui->label_4->setVisible(false);
        if (ui->lineEdit_jgLeftRear) ui->lineEdit_jgLeftRear->setVisible(false);
        if (ui->label_5) ui->label_5->setVisible(false);
        if (ui->lineEdit_jgRightRear) ui->lineEdit_jgRightRear->setVisible(false);
        // 显示烟幕弹指示灯
        if (ui->label_9)  ui->label_9->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletl1) ui->lineEdit_SmokeScreenBulletl1->setVisible(true);
        if (ui->label_10) ui->label_10->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletl2) ui->lineEdit_SmokeScreenBulletl2->setVisible(true);
        if (ui->label_11) ui->label_11->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletl3) ui->lineEdit_SmokeScreenBulletl3->setVisible(true);
        if (ui->label_12) ui->label_12->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletl4) ui->lineEdit_SmokeScreenBulletl4->setVisible(true);
        if (ui->label_8)  ui->label_8->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletr1) ui->lineEdit_SmokeScreenBulletr1->setVisible(true);
        if (ui->label_13) ui->label_13->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletr2) ui->lineEdit_SmokeScreenBulletr2->setVisible(true);
        if (ui->label_15) ui->label_15->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletr3) ui->lineEdit_SmokeScreenBulletr3->setVisible(true);
        if (ui->label_18) ui->label_18->setVisible(true);
        if (ui->lineEdit_SmokeScreenBulletr4) ui->lineEdit_SmokeScreenBulletr4->setVisible(true);
        LOG("可见性: 烟幕弹测试");
        break;

    case ModeFireSuppress:
        if (ui->groupBox_3) ui->groupBox_3->setVisible(true);
        LOG("可见性: 灭火抑爆测试");
        break;

    case ModeTopComponent:
        if (ui->groupBox_4) ui->groupBox_4->setVisible(true);
        if (ui->groupBox_5) ui->groupBox_5->setVisible(true);
        LOG("可见性: 顶部组件测试");
        break;
    }
}

void MainWindow::initDevices()
{
    LOG("initDevices 开始...");

    // COM10 Modbus (DIO)
    m_modbus = new Stm32ModbusClient();
    m_modbus->setPortName("COM10");
    m_modbus->setBaudRate(QSerialPort::Baud9600);
    m_modbus->setSlaveAddress(0x01);
    if (!m_modbus->open()) {
        QString err = "COM10 打开失败! 撤弹/补弹/自毁不可用";
        qDebug() << err;
        LOG(err);
        m_dio = nullptr;
    } else {
        m_dio = new DigitalIOController(*m_modbus, this);
        m_dio->setProtectionMode();
        LOG("COM10 打开成功, DIO 已初始化");
        for (int ch = 1; ch <= 8; ch++) {
            m_dio->setChannel(ch, false);
        }

        LOG("DIO 全部通道已初始化为OFF");
    }

    // COM1 激光告警
    if (m_laserWarning->connectDevice("COM1")) {
        LOG("激光告警 COM1 连接成功");
        QByteArray initCmd = QByteArray::fromHex("F0F0020A000101010B02");
        m_laserWarning->sendRawData(initCmd);
    } else {
        QString err = "COM1 打开失败! 激光告警1不可用";
        LOG(err);
    }

    // com2 激光告警
    if (m_laserWarning2->connectDevice("COM2")) {
        LOG("激光告警 com2 连接成功");
        QByteArray initCmd = QByteArray::fromHex("F0F0020A000101010B02");
        m_laserWarning2->sendRawData(initCmd);
    } else {
        QString err = "com2 打开失败! 激光告警2不可用";
        LOG(err);
    }

    // 敌我识别自检
    m_diwo = new DiwoSerial(this);
    connect(m_diwo, &DiwoSerial::dataReceived, this, [this]() {
        // 每次收到数据后重发当前状态帧
        quint8 st = 0;
        st |= (ui->comboBox_diwoHost->currentIndex() & 0x01);       // bit0=主机
        st |= (ui->comboBox_diwoInquiry->currentIndex() & 0x01) << 1; // bit1=询问
        st |= (ui->comboBox_diwoResponse->currentIndex() & 0x01) << 2; // bit2=应答
        m_diwo->sendStatusFrame(st);
    });
    connect(m_diwo, &DiwoSerial::errorOccurred, this, [](const QString& e) {
        LOG(QString("敌我识别错误: %1").arg(e));
    });
    // 下拉框切换即发送
    auto sendDiwoStatus = [this]() {
        if (!m_diwo || !m_diwo->isOpen()) return;
        quint8 st = 0;
        st |= (ui->comboBox_diwoHost->currentIndex() & 0x01);
        st |= (ui->comboBox_diwoInquiry->currentIndex() & 0x01) << 1;
        st |= (ui->comboBox_diwoResponse->currentIndex() & 0x01) << 2;
        m_diwo->sendStatusFrame(st);
    };
    connect(ui->comboBox_diwoHost, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sendDiwoStatus);
    connect(ui->comboBox_diwoInquiry, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sendDiwoStatus);
    connect(ui->comboBox_diwoResponse, QOverload<int>::of(&QComboBox::currentIndexChanged), this, sendDiwoStatus);
    if (m_diwo->open("COM9")) {
        LOG("敌我识别 COM9 打开成功");
    } else {
        LOG("敌我识别 COM9 打开失败");
    }

    // 顶部组件串口 (groupBox_5)
    m_topSerial = new QSerialPort(this);
    m_topSerial->setPortName("COM11");
    m_topSerial->setBaudRate(QSerialPort::Baud115200);
    m_topSerial->setDataBits(QSerialPort::Data8);
    m_topSerial->setStopBits(QSerialPort::OneStop);
    m_topSerial->setParity(QSerialPort::EvenParity);
    m_topSerial->setFlowControl(QSerialPort::NoFlowControl);
    if (m_topSerial->open(QIODevice::ReadWrite)) {
        LOG("顶部组件 COM11 打开成功");
        connect(m_topSerial, &QSerialPort::readyRead, this, &MainWindow::onTopSerialReadyRead);
    } else {
        LOG("顶部组件 COM11 打开失败");
    }

    // Flex/Ray 启动 — 单独延后, 不阻塞
    {
        QPointer<MainWindow> safeThis(this);
        QTimer::singleShot(500, this, [safeThis]() {
            if (!safeThis) return;
            // 防止重入：如果已有启动进行中则跳过
            if (safeThis->m_flexStartInProgress.load()) {
                LOG("FlexRay 启动请求被忽略：已有启动进行中");
                return;
            }
            LOG("FlexRay 开始启动 (queued)...");
            // 如果没有 worker 或 worker 已被删除则不调用
            if (!safeThis->m_flexRayThread) return;
            // 标记启动中，避免定时器重复触发造成重入
            safeThis->m_flexStartInProgress.store(true);
            // 停止重连定时器在启动期间避免重复排队
            if (safeThis->m_flexray_reconnect_timer) safeThis->m_flexray_reconnect_timer->stop();
            // 将 start 请求排入 FlexRay worker 的事件队列（安全，非阻塞 UI）
            QMetaObject::invokeMethod(safeThis->m_flexRayThread, "startFlexRayOperation", Qt::QueuedConnection);
            // 启动一个超时看门器：若在超时内未能建立连接，则清理 in-progress 标记并启动重连定时器
            QTimer::singleShot(15000, safeThis.data(), [safeThis]() {
                if (!safeThis) return;
                if (safeThis->m_flexStartInProgress.load()) {
                    LOG("FlexRay 启动超时：清除启动标志并启动重连定时器");
                    safeThis->m_flexStartInProgress.store(false);
                    if (safeThis->m_flexray_reconnect_timer) safeThis->m_flexray_reconnect_timer->start();
                }
            });
        });
    }
    LOG("initDevices 完成");
}

void MainWindow::initFlexRayConnections()
{
    qRegisterMetaType<QVector<uint16>>("QVector<uint16>");
    LOG("FlexRay 初始化连接...");
    m_flexRayThread = new FlexRayThread();
    LOG("FlexRayThread 已创建");

    // 创建并启动用于承载 FlexRay worker 的 QThread，并将 worker 移到该线程
    m_flexRayWorkerThread = new QThread(this);
    m_flexRayThread->moveToThread(m_flexRayWorkerThread);
    // 确保线程退出时自动删除 worker 对象由 MainWindow 负责删除
    m_flexRayWorkerThread->start();
    LOG(QString("moveToThread done: workerThread=%1 isRunning=%2 workerAffinity=%3")
        .arg((quintptr)m_flexRayWorkerThread,0,16).arg(m_flexRayWorkerThread->isRunning()).arg((quintptr)m_flexRayThread->thread(),0,16));

    // 创建并管理由 MainWindow 负责的定时器（仅重连定时器）
    m_flexray_reconnect_timer = new QTimer(this);
    m_flexray_reconnect_timer->setInterval(3000);
    m_flexray_reconnect_timer->setSingleShot(false);
    {
        QPointer<MainWindow> safeThis(this);
        connect(m_flexray_reconnect_timer, &QTimer::timeout, this, [safeThis]() {
            if (!safeThis) return;
            // 如果线程正在运行则不需要重连
            if (!safeThis->m_flexRayThread) return;
            if (safeThis->m_flexRayThread->isRunning()) return;
            // 如果已有启动进行中则跳过本次重连触发
            if (safeThis->m_flexStartInProgress.load()) {
                LOG("FlexRay reconnect timer: 忽略，已有启动进行中");
                return;
            }
            LOG("FlexRay reconnect timer: 尝试启动 (queued)...");
            // 标记正在启动并停止重连定时器，避免重复排队
            safeThis->m_flexStartInProgress.store(true);
            if (safeThis->m_flexray_reconnect_timer) safeThis->m_flexray_reconnect_timer->stop();
            // 将 start 请求排入 FlexRay worker 的事件队列（安全，非阻塞 UI）
            QMetaObject::invokeMethod(safeThis->m_flexRayThread, "startFlexRayOperation", Qt::QueuedConnection);
            // 超时保护：若15s内未完成，则清除 in-progress 并重启重连定时器
            QTimer::singleShot(15000, safeThis.data(), [safeThis]() {
                if (!safeThis) return;
                if (safeThis->m_flexStartInProgress.load()) {
                    LOG("FlexRay reconnect 超时：清除启动标志并重启重连定时器");
                    safeThis->m_flexStartInProgress.store(false);
                    if (safeThis->m_flexray_reconnect_timer) safeThis->m_flexray_reconnect_timer->start();
                }
            });
        });
    }

    bool connected = QObject::connect(m_flexRayThread, &FlexRayThread::flexRayDataReceived,
                     this, &MainWindow::show_data, Qt::QueuedConnection);
    if (connected) {
        LOG("FlexRay data信号连接: 成功");
    } else {
        LOG("FlexRay data信号连接: 失败!!! 类型不匹配!");
        QMessageBox::critical(nullptr, "FlexRay 错误", "信号连接失败! 类型不匹配");
    }
    QObject::connect(m_flexRayThread, &FlexRayThread::error, this, [this](const QString &err){
            qDebug() << "FlexRay错误:" << err;
            LOG("FlexRay错误: " + err);
            // 清理启动中标志（启动失败）
            m_flexStartInProgress.store(false);
            // 确保在 GUI 线程创建对话框: 使用 queued connection 或者再次通过 invokeMethod 到主线程执行
            QMessageBox *msgBox = new QMessageBox(QMessageBox::Warning, "FlexRay错误", err, QMessageBox::Ok, this);
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->show();
            // 启动重连定时器以便稍后重试
            if (m_flexray_reconnect_timer) m_flexray_reconnect_timer->start();
        }, Qt::QueuedConnection);
    QObject::connect(m_flexRayThread, &FlexRayThread::connectionLost, this, [this](){
            qDebug() << "FlexRay连接丢失";
            LOG("FlexRay连接丢失");
            // 清理启动中标志（若在尝试启动时发生断开）
            m_flexStartInProgress.store(false);
            ui->lineEdit_16->setText("0");
            ui->lineEdit_15->setText("0");

            QMessageBox *msgBox = new QMessageBox(QMessageBox::Critical, "FlexRay", "连接丢失，正在尝试重连...", QMessageBox::Ok, this);
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->show();

            // 启动重连定时器
            if (m_flexray_reconnect_timer) m_flexray_reconnect_timer->start();
        }, Qt::QueuedConnection);
    QObject::connect(m_flexRayThread, &FlexRayThread::connectionRestored, this, [this](){
            qDebug() << "FlexRay连接恢复";
            LOG("FlexRay连接恢复");

            QMessageBox *msgBox = new QMessageBox(QMessageBox::Information, "FlexRay", "连接已恢复", QMessageBox::Ok, this);
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->show();

            // 停止重连定时器
            if (m_flexray_reconnect_timer) m_flexray_reconnect_timer->stop();
        }, Qt::QueuedConnection);

    // AB灯: 节点状态做门控 + 超时强制灭
    QObject::connect(m_flexRayThread, &FlexRayThread::abChannelChanged,
                     this, [this](int chA, int chB) {
        m_abNodeAOk = chA;
        m_abNodeBOk = chB;
        if (!chA) ui->lineEdit_16->setText("0");
        if (!chB) ui->lineEdit_15->setText("0");
    }, Qt::QueuedConnection);
    QObject::connect(m_flexRayThread, &FlexRayThread::abFrameTimeout,
                     this, [this]() {
        ui->lineEdit_16->setText("0");
        ui->lineEdit_15->setText("0");
    }, Qt::QueuedConnection);

    QObject::connect(this, &MainWindow::flexSend,
                     m_flexRayThread, &FlexRayThread::flexRayDataSend);
    LOG("FlexRay 信号连接完成");
}

void MainWindow::on_pushButton_close_clicked()
{
    LOG("用户点击了「返回」按钮");

    // 记录会话结束时间
    m_sessionEndTime = QDateTime::currentDateTime();

    // 生成会话PDF报告
    generateSessionPDF();

    // 不关闭设备，只隐藏窗口，通知返回测试选择界面
    hide();
    emit requestReturn();
}

// =================== 雷达板卡 ===================
void MainWindow::on_pushButton_open_clicked()
{
    if (m_radar->open()) {
        quint8 channelSelect = 0;
        if (ui->radioButton_Passageway1->isChecked()) channelSelect = 0;
        else if (ui->radioButton_Passageway2->isChecked()) channelSelect = 1;
        else if (ui->radioButton_Passageway3->isChecked()) channelSelect = 2;
        else if (ui->radioButton_Passageway4->isChecked()) channelSelect = 3;

        quint8 priUs = static_cast<quint8>(ui->lineEdit_PulseRepetitionPeriod->text().toUInt());
        quint8 pwUs  = static_cast<quint8>(ui->lineEdit_PulseWidth->text().toUInt());

        m_radar->sendTriggerCommand(true, priUs, pwUs, 55, 60, 7);
        QTimer::singleShot(7000, this, [this, priUs, pwUs, channelSelect]() {
            m_radar->sendTriggerCommand(true, priUs, pwUs, 55, 60, channelSelect);
        });

        // 锁定: 打开禁用, 关闭启用, 通道不可切换
        ui->pushButton_open->setEnabled(false);
        ui->pushButton_radiaclose->setEnabled(true);
        ui->radioButton_Passageway1->setEnabled(false);
        ui->radioButton_Passageway2->setEnabled(false);
        ui->radioButton_Passageway3->setEnabled(false);
        ui->radioButton_Passageway4->setEnabled(false);
    } else {
        QMessageBox::warning(this, "错误", "雷达串口打开失败");
    }
}
//雷达板卡关闭
void MainWindow::on_pushButton_radiaclose_clicked()
{
    quint8 priUs = static_cast<quint8>(ui->lineEdit_PulseRepetitionPeriod->text().toUInt());
    quint8 pwUs  = static_cast<quint8>(ui->lineEdit_PulseWidth->text().toUInt());

    m_radar->sendTriggerCommand(false, priUs, pwUs, 55, 60, 7);

    // 解锁: 关闭禁用, 打开启用, 通道可切换
    ui->pushButton_radiaclose->setEnabled(false);
    ui->pushButton_open->setEnabled(true);
    ui->radioButton_Passageway1->setEnabled(true);
    ui->radioButton_Passageway2->setEnabled(true);
    ui->radioButton_Passageway3->setEnabled(true);
    ui->radioButton_Passageway4->setEnabled(true);
}

// =================== 烟幕弹控制 (groupBox_10) ===================
void MainWindow::on_pushButton_All_clicked()
{
    QList<QCheckBox*> boxes = {
        ui->checkBox_l1, ui->checkBox_r1,
        ui->checkBox_l2, ui->checkBox_r2,
        ui->checkBox_l3, ui->checkBox_r3,
        ui->checkBox_l4, ui->checkBox_r4
    };
    for (auto *cb : boxes) {
        cb->setChecked(true);
    }
}

// =================== 激光告警自检 (COM1 + com2) ===================
void MainWindow::on_pushButton_SelectAll_clicked()
{
    LOG("激光告警自检 发送");
    m_laserWarning->sendSelfCheckCommand();
    m_laserWarning2->sendSelfCheckCommand();
}

// =================== 敌我识别自检 ===================
void MainWindow::on_pushButton_diwoselfcheck_clicked() {
    if (!m_diwo || !m_diwo->isOpen()) {
        LOG("敌我识别自检失败: 串口未打开");
        return;
    }
    // 根据下拉框组合状态字节: bit0=主机, bit1=询问, bit2=应答 (0=正常,1=故障)
    quint8 st = 0;
    st |= (ui->comboBox_diwoHost->currentIndex() & 0x01);
    st |= (ui->comboBox_diwoInquiry->currentIndex() & 0x01) << 1;
    st |= (ui->comboBox_diwoResponse->currentIndex() & 0x01) << 2;
    m_diwo->sendStatusFrame(st);
    LOG(QString("敌我识别自检 发送 status=0x%1").arg(st, 2, 16, QChar('0')));
}

// =================== 顶部组件组件 FlexRay发送 (FR_MP_5, 槽ID=42) ===================
void MainWindow::on_pushButton_SendSelfTestResults_2_clicked()
{
    m_flexRayTxCounter++;

    quint8 mode = 0x00;

    QVector<uint16> frameData(16, 0x0000);
    frameData[0] = (quint16)m_flexRayTxCounter;
    frameData[1] = 0x0100;
    frameData[2] = (quint16)mode;
    frameData[3] = 0x0000;
    frameData[4] = 0x0011;

    m_flexRayThread->flexRayDataSend_MP5(frameData);

    QString logMsg = QString("FR_MP_5 TX: counter=%1 mode=0x%2")
        .arg(m_flexRayTxCounter).arg(mode, 2, 16, QLatin1Char('0'));
    LOG(logMsg);
}

//取消选择
void MainWindow::on_pushButton_Cancel_clicked()
{
    QList<QCheckBox*> boxes = {
        ui->checkBox_l1, ui->checkBox_r1,
        ui->checkBox_l2, ui->checkBox_r2,
        ui->checkBox_l3, ui->checkBox_r3,
        ui->checkBox_l4, ui->checkBox_r4
    };
    for (auto *cb : boxes) {
        cb->setChecked(false);
    }
}
//补弹/撤弹 切换按钮
void MainWindow::on_pushButton_WithdrawalOfAmmunition_clicked()
{
    if (!m_dio) {
        LOG("补弹失败: m_dio为空(COM10未连接)");
        QMessageBox::warning(this, "错误", "数字量接口板未连接(COM10)，补弹不可用");
        return;
    }
    struct CbCh { QCheckBox *cb; int ch; };
    const CbCh mapping[] = {
        { ui->checkBox_l1, 1 }, { ui->checkBox_r1, 5 },
        { ui->checkBox_l2, 2 }, { ui->checkBox_r2, 6 },
        { ui->checkBox_l3, 3 }, { ui->checkBox_r3, 7 },
        { ui->checkBox_l4, 4 }, { ui->checkBox_r4, 8 },
    };

    if (!m_isReplenishing) {
        // ====== 补弹 ======
        LOG("补弹");
        QList<int> selectedChannels;
        for (const auto &m : mapping) {
            if (m.cb->isChecked()) {
                bool ok = m_dio->setChannel(m.ch, true);
                LOG(QString("补弹 CH%1=ON %2%3").arg(m.ch).arg(ok ? "OK" : "FAIL").arg(ok ? "" : " " + m_dio->lastError()));
                if (ok) selectedChannels.append(m.ch);
            }
        }

        m_isReplenishing = true;
        ui->pushButton_WithdrawalOfAmmunition->setText("撤弹");
    } else {
        // ====== 撤弹 ======
        LOG("撤弹");
        for (const auto &m : mapping) {
            if (m.cb->isChecked()) {
                bool ok = m_dio->setChannel(m.ch, false);
                LOG(QString("撤弹 CH%1=OFF %2%3").arg(m.ch).arg(ok ? "OK" : "FAIL").arg(ok ? "" : " " + m_dio->lastError()));
            }
        }
        m_isReplenishing = false;
        ui->pushButton_WithdrawalOfAmmunition->setText("补弹");
    }
}

void MainWindow::startBridgeToggleDetect()
{
    LOG("启动Bridge检测(最长10秒, 50ms轮询, 连续3次=1即触发)");

    QProcess* proc = new QProcess(this);
    QTimer* pollTimer = new QTimer(this);
    QSharedPointer<bool> done(new bool(false));
    QSharedPointer<QVector<int>> consecutiveOnes(new QVector<int>(8, 0));
    QSharedPointer<int> readCount(new int(0));

    connect(proc, QOverload<int>::of(&QProcess::finished),
            this, [proc, pollTimer, this](int) {
        pollTimer->stop();
        pollTimer->deleteLater();
        proc->deleteLater();
        LOG("Bridge 已退出");
    });

    // 10秒超时兜底
    QTimer::singleShot(10000, this, [this, proc, done, readCount]() {
        if (*done) return;
        *done = true;
        LOG(QString("Bridge超时结束: 共读%1次, 无稳定置1位").arg(*readCount));
        proc->kill();
    });

    // 每50ms读一次
    connect(pollTimer, &QTimer::timeout, this, [this, proc, pollTimer, done, consecutiveOnes, readCount]() {
        if (*done) return;

        QFile f(QDir::currentPath() + "/di_data.txt");
        if (!f.open(QIODevice::ReadOnly)) return;
        QByteArray d = f.readAll().trimmed();
        f.close();
        if (d.isEmpty()) return;

        QJsonDocument doc = QJsonDocument::fromJson(d);
        if (doc.isNull() || !doc.isObject()) return;
        QJsonObject obj = doc.object();
        if (!obj.contains("ports")) return;
        QJsonArray arr = obj["ports"].toArray();
        if (arr.size() < 16) return;

        (*readCount)++;
        const int chByTxtIdx[] = {1, 5, 2, 6, 3, 7, 4, 8};
        QStringList allVals;
        for (int i = 0; i < 16; i++)
            allVals.append(QString::number(arr[i].toInt()));

        for (int i = 0; i < 8; i++) {
            int txtPos = 8 + i;
            int val = arr[txtPos].toInt();

            if (val == 1) {
                (*consecutiveOnes)[i]++;
                if ((*consecutiveOnes)[i] >= 3) {
                    *done = true;
                    pollTimer->stop();
                    int ch = chByTxtIdx[i];
                    bool ok = m_dio ? m_dio->setChannel(ch, false) : false;
                    LOG(QString("Bridge触发: txt[%1]连续3次=1 → CH%2=%3 (共读%4次)")
                        .arg(txtPos).arg(ch).arg(ok ? "OFF" : "FAIL").arg(*readCount));
                    proc->kill();
                    return;
                }
            } else {
                (*consecutiveOnes)[i] = 0;
            }
        }

        if (*readCount <= 3 || *readCount % 20 == 0) {
            LOG(QString("Bridge: 第%1次读 txt[0-15]=%2").arg(*readCount).arg(allVals.join(" ")));
        }
    });

    connect(proc, &QProcess::started, this, [proc, pollTimer, this]() {
        LOG(QString("Bridge PID=%1").arg(proc->processId()));
        pollTimer->start(50);
    });
    connect(proc, &QProcess::errorOccurred, this, [proc, pollTimer, done, this](QProcess::ProcessError) {
        if (*done) return;
        *done = true;
        LOG("Bridge 启动失败");
        pollTimer->deleteLater();
        proc->deleteLater();
    });

    proc->start("Bridge.exe");
}

// =================== FlexRay show_data（含状态追踪）===================
void MainWindow::show_data(QVector<uint16> rcvData, int channel)
{
    if (rcvData.isEmpty()) return;

    static int showCount = 0;
    static int lastShowCh = -1;
    if (++showCount <= 10 || showCount % 50 == 0 || channel != lastShowCh) {
        QString logMsg = QString("show_data ch=%1 len=%2 count=%3").arg(channel).arg(rcvData.size()).arg(showCount);
        LOG(logMsg);
    }
    lastShowCh = channel;

    if (channel == 2)
    {
        bool changed = false;
        for (int i = 0; i < qMin(rcvData.size(), m_TemprcvData2.size()); i++) {
            if (rcvData.at(i) != m_TemprcvData2.at(i)) { changed = true; break; }
        }
        if (!changed) return;
        LOG("show_data ch=2 数据变化!");

        if (rcvData.size() >= 10) {
            quint8 dir = (rcvData.at(9) >> 8) & 0xFF;
            const char* dirText = "";
            switch (dir) {
                case 0x00: dirText = "正常驾驶"; break;
                case 0x01: dirText = "正前方"; break;
                case 0x02: dirText = "右前方"; break;
                case 0x03: dirText = "右方"; break;
                case 0x04: dirText = "右后方"; break;
                case 0x05: dirText = "正后方"; break;
                case 0x06: dirText = "左后方"; break;
                case 0x07: dirText = "左方"; break;
                case 0x08: dirText = "左前方"; break;
                case 0xFF: dirText = "无法提示"; break;
                default: dirText = "未知"; break;
            }
            ui->lineEdit_2->setText(dirText);

            // ====== 雷达方向状态追踪 ======
            if (m_testMode == ModeRadarBoard) {
                QString dirStr = QString::fromUtf8(dirText);
                // 只记录有效的8个方向 (排除"正常驾驶"、"无法提示"、"未知")
                if (dir >= 0x01 && dir <= 0x08) {
                    m_radarDirectionsSeen.insert(dirStr);
                }
            }
        }
        for (int i = 0; i < qMin(rcvData.size(), m_TemprcvData2.size()); i++)
            m_TemprcvData2[i] = rcvData.at(i);
    }
    else if (channel == 1)
    {
        bool changed = false;
        for (int i = 0; i < qMin(rcvData.size(), m_TemprcvData1.size()); i++) {
            if ((double)rcvData.at(i) != m_TemprcvData1.at(i)) { changed = true; break; }
        }
        if (!changed) return;
        for (int i = 0; i < qMin(rcvData.size(), m_TemprcvData1.size()); i++)
            m_TemprcvData1[i] = rcvData.at(i);

        // 烟幕弹指示灯
        quint8 leftJamming  = rcvData.at(5) & 0xFF;
        quint8 rightJamming = (rcvData.at(5) >> 8) & 0xFF;

        auto setSmokeAndTrack = [this](CircleLineEdit* w, bool isOne, const QString& name) {
            QString val = isOne ? "1" : "0";
            w->setText(val);
            if (m_testMode == ModeSmoke) {
                m_smokeStates[name].insert(val);
            }
        };

        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletl1, leftJamming & 0x01,       QString::fromUtf8("左1"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletl2, (leftJamming >> 1) & 0x01, QString::fromUtf8("左2"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletl3, (leftJamming >> 2) & 0x01, QString::fromUtf8("左3"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletl4, (leftJamming >> 3) & 0x01, QString::fromUtf8("左4"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletr1, rightJamming & 0x01,       QString::fromUtf8("右1"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletr2, (rightJamming >> 1) & 0x01, QString::fromUtf8("右2"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletr3, (rightJamming >> 2) & 0x01, QString::fromUtf8("右3"));
        setSmokeAndTrack(ui->lineEdit_SmokeScreenBulletr4, (rightJamming >> 3) & 0x01, QString::fromUtf8("右4"));

        // 激光告警指示灯
        if (rcvData.size() >= 7) {
            quint8 b12 = rcvData.at(6) & 0xFF;
            quint8 b13 = (rcvData.at(6) >> 8) & 0xFF;

            auto setJgAndTrack = [this](CircleLineEdit* w, int val, const QString& name) {
                w->setText(QString::number(val));
                if (val > 0) {
                    m_laserIndicatorsLit.insert(name);
                }
            };

            setJgAndTrack(ui->lineEdit_jgLeftFront,  (b12 >> 4) & 0x03, QString::fromUtf8("左前"));
            setJgAndTrack(ui->lineEdit_jgLeftRear,   b12 & 0x03,        QString::fromUtf8("左后"));
            setJgAndTrack(ui->lineEdit_jgRightFront, (b13 >> 4) & 0x03, QString::fromUtf8("右前"));
            setJgAndTrack(ui->lineEdit_jgRightRear,  b13 & 0x03,        QString::fromUtf8("右后"));
        }

        // 敌我识别 + 毫米波状态 (byte16-17 在 word8 里)
        if (rcvData.size() >= 9) {
            quint8 byte16 = rcvData.at(8) & 0xFF;
            quint8 byte17 = (rcvData.at(8) >> 8) & 0xFF;

            auto showStatusAndTrack = [this](CircleLineEdit* w, quint8 v, const QString& name) {
                QString val;
                if      (v == 0x01) val = "1";
                else if (v == 0x02) val = "2";
                else                val = "0";
                w->setText(val);
                // 追踪0和1两种状态（"2"也算作1的一种异常状态）
                if (m_testMode == ModeDiwo) {
                    if (val == "0") m_diwoStates[name].insert("0");
                    else m_diwoStates[name].insert("1");  // "1" 或 "2" 都算已触发
                }
            };
            showStatusAndTrack(ui->lineEdit_diwoHost,      (byte16 >> 4) & 0x0F, QString::fromUtf8("主机"));
            showStatusAndTrack(ui->lineEdit_mmwaveLF,      byte16 & 0x0F,        QString::fromUtf8("询问"));
            showStatusAndTrack(ui->lineEdit_diwoResponder, (byte17 >> 4) & 0x0F, QString::fromUtf8("应答"));
        }

        // 灭火抑爆 (byte14.7-4) + 顶部组件 (byte15.7-4)
        if (rcvData.size() >= 8) {
            quint8 byte14 = rcvData.at(7) & 0xFF;
            quint8 byte15 = (rcvData.at(7) >> 8) & 0xFF;

            auto setNibbleAndTrack = [this](CircleLineEdit* w, quint8 v, QSet<QString>& trackSet) {
                QString val;
                if      (v == 0x01) val = "1";
                else if (v == 0x02) val = "2";
                else                val = "0";
                w->setText(val);
                trackSet.insert(val);
            };

            if (m_testMode == ModeFireSuppress && ui->lineEdit_17) {
                setNibbleAndTrack(ui->lineEdit_17, (byte14 >> 4) & 0x0F, m_fireSuppressValues);
            }
            if (m_testMode == ModeTopComponent && ui->lineEdit_18) {
                setNibbleAndTrack(ui->lineEdit_18, (byte15 >> 4) & 0x0F, m_topComponentValues);
            }
        }
    }
    else if (channel == 4)
    {
        if (rcvData.size() < 2) return;
        if (rcvData.at(0) == m_TemprcvData4.at(0) && rcvData.at(1) == m_TemprcvData4.at(1))
            return;
        QString logMsg = QString("show_data ch=4 AB变化! ch=%1 slot=%2").arg(rcvData.at(0)).arg(rcvData.at(1));
        LOG(logMsg);
        m_TemprcvData4[0] = rcvData.at(0);
        m_TemprcvData4[1] = rcvData.at(1);

        int abChannel = rcvData.at(0);
        ui->lineEdit_16->setText(m_abNodeAOk && (abChannel & 0x01) ? "1" : "0");  // A
        ui->lineEdit_15->setText(m_abNodeBOk && (abChannel & 0x02) ? "1" : "0");  // B
    }
}

//毁钥
void MainWindow::on_pushButton_SelfDestruct_clicked()
{
    LOG("自毁按钮点击");
    m_bSelfDestructToggled = !m_bSelfDestructToggled;
    m_dio->setChannel(13, m_bSelfDestructToggled);
    QString logMsg = QString("自毁 PB12(CH13) = %1").arg(m_bSelfDestructToggled ? "ON" : "OFF");
    LOG(logMsg);
    qDebug() << "自毁 PB12(CH13) =" << (m_bSelfDestructToggled ? "ON" : "OFF");
}

//告警发射
void MainWindow::on_pushButton_check_clicked()
{
    QByteArray initCmd = QByteArray::fromHex("F0F0020A000101010B02");
    m_laserWarning->sendRawData(initCmd);
    m_laserWarning2->sendRawData(initCmd);
}

// 顶部组件发送 (groupBox_5)
void MainWindow::on_pushButton_top_clicked()
{
    if (!m_topSerial || !m_topSerial->isOpen()) {
        LOG("顶部组件: 串口未打开");
        return;
    }

    QByteArray frame;
    frame.append((char)0x2C);
    frame.append((char)0x2C);
    frame.append((char)0x02);
    frame.append((char)0x09);
    frame.append((char)0x00);
    frame.append((char)0x01);
    frame.append((char)0x01);
    frame.append((char)0x04);

    quint8 cs = 0;
    for (int i = 0; i < frame.size(); i++)
        cs ^= (quint8)frame.at(i);
    frame.append((char)cs);

    LOG(QString("顶部组件 TX: %1").arg(QString::fromLatin1(frame.toHex(' '))));
    m_topSerial->write(frame);
}
////关闭
//void MainWindow::on_pushButton_2_clicked()
//{
//    if (!m_topSerial || !m_topSerial->isOpen()) {
//        LOG("顶部组件: 串口未打开");
//        return;
//    }

//    QByteArray frame;
//    frame.append((char)0x2C);
//    frame.append((char)0x2C);
//    frame.append((char)0x02);
//    frame.append((char)0x09);
//    frame.append((char)0x00);
//    frame.append((char)0x01);
//    frame.append((char)0x01);
//    frame.append((char)0x04);

//    quint8 cs = 0;
//    for (int i = 0; i < frame.size(); i++)
//        cs ^= (quint8)frame.at(i);
//    frame.append((char)cs);

//    LOG(QString("顶部组件 TX: %1").arg(QString::fromLatin1(frame.toHex(' '))));
//    m_topSerial->write(frame);
//}
void MainWindow::onTopSerialReadyRead()
{
    QByteArray raw = m_topSerial->readAll();
    LOG(QString("顶部组件 RX(raw): %1").arg(QString::fromLatin1(raw.toHex(' '))));
    m_topSerialBuffer.append(raw);

    // 帧头 2C 2C 02 09 00, 总长8字节: 头5 + 无效2 + 校验1
    while (m_topSerialBuffer.size() >= 6) {
        int headIdx = m_topSerialBuffer.indexOf(QByteArray::fromHex("2C2C010700"));
        if (headIdx == -1) { m_topSerialBuffer.clear(); return; }
        if (headIdx > 0) m_topSerialBuffer.remove(0, headIdx);
        if (m_topSerialBuffer.size() < 6) return;

        QByteArray frame = m_topSerialBuffer.left(6);
        m_topSerialBuffer.remove(0, 6);

        quint8 recvCs = (quint8)frame.at(5);
        quint8 calcCs = 0;
        for (int i = 0; i < 5; i++)
            calcCs ^= (quint8)frame.at(i);

        LOG(QString("顶部组件 RX: %1 (CS calc=0x%2 recv=0x%3 %4)")
            .arg(QString::fromLatin1(frame.toHex(' ')))
            .arg(calcCs, 2, 16, QChar('0'))
            .arg(recvCs, 2, 16, QChar('0'))
            .arg(calcCs == recvCs ? "OK" : "FAIL"));

        if (calcCs != recvCs) continue;

//        // 校验通过, 点亮指示灯
//        if (ui->lineEdit_19) ui->lineEdit_19->setText("1");
//        if (m_testMode == ModeTopComponent) {
//            m_topComponentValues.insert("1");
//        }
//        LOG("顶部组件: 灯已点亮");
    }
}

// =================== PDF报告生成 ===================

QString MainWindow::generateSessionPDF()
{
    QString reportDir = QApplication::applicationDirPath() + "/reports";
    QDir().mkpath(reportDir);

    QString timestamp = m_sessionStartTime.toString("yyyyMMdd_hhmmss");
    QString fileName = QString("%1/%2_%3_%4.pdf")
                       .arg(reportDir)
                       .arg(timestamp)
                       .arg(testModeName())
                       .arg(m_currentUser);

    qDebug() << "[MainWindow] 正在生成会话PDF报告:" << fileName;
    LOG(QString("正在生成PDF报告: %1").arg(fileName));

    PDFReportGenerator generator;
    generator.setCompanyInfo(QString::fromUtf8("中国人民解放军第二七六五厂"));
    generator.setReportTitle(QString::fromUtf8("防护采集驱动装置检测设备  检测报告"));
    generator.setDeviceName(QString::fromUtf8("防护采集驱动装置"));
    generator.setPartNumber(QString(""));
    generator.setInspector(m_currentUser);
    generator.setTestDate(m_sessionStartTime.toString("yyyy-MM-dd"));

    // 收集 ReportRow
    QList<ReportRow> rows = collectReportRows();
    for (const auto &r : rows) generator.addReportRow(r);

    bool ok = generator.generateFormalReport(fileName);
    if (ok) {
        LOG(QString("PDF报告已保存: %1").arg(fileName));
        qDebug() << "[MainWindow] PDF报告生成成功:" << fileName;
    } else {
        LOG("PDF报告生成失败!");
        qWarning() << "[MainWindow] PDF报告生成失败!";
    }

    return ok ? fileName : QString();
}

QList<ReportRow> MainWindow::collectReportRows() const
{
    switch (m_testMode) {
    case ModeRadarBoard: return collectRadarBoardRows();
    case ModeDiwo:       return collectDiwoRows();
    case ModeLaser:      return collectLaserRows();
    case ModeSmoke:      return collectSmokeRows();
    case ModeFireSuppress: return collectFireSuppressRows();
    case ModeTopComponent:   return collectTopComponentRows();
    default:             return QList<ReportRow>();
    }
}

QList<ReportRow> MainWindow::collectRadarBoardRows() const
{
    QList<ReportRow> rows;

    struct ChannelInfo {
        QString name;
        QString dir1, dir2;
    };
    const ChannelInfo channels[] = {
        { QString::fromUtf8("通道1"), QString::fromUtf8("正前方"), QString::fromUtf8("右方") },
        { QString::fromUtf8("通道2"), QString::fromUtf8("右前方"), QString::fromUtf8("右后方") },
        { QString::fromUtf8("通道3"), QString::fromUtf8("左方"), QString::fromUtf8("正后方") },
        { QString::fromUtf8("通道4"), QString::fromUtf8("左前方"), QString::fromUtf8("左后方") },
    };

    for (const auto &ch : channels) {
        bool dir1Ok = m_radarDirectionsSeen.contains(ch.dir1);
        bool dir2Ok = m_radarDirectionsSeen.contains(ch.dir2);
        bool allOk = dir1Ok && dir2Ok;

        QString result;
        if (allOk) {
            result = QString::fromUtf8("检测到: %1, %2").arg(ch.dir1, ch.dir2);
        } else {
            QStringList missing;
            if (!dir1Ok) missing.append(ch.dir1);
            if (!dir2Ok) missing.append(ch.dir2);
            QStringList seenList;
            for (const auto &d : m_radarDirectionsSeen) seenList.append(d);
            result = QString::fromUtf8("缺失: %1 (已检测到: %2)")
                     .arg(missing.join(", "))
                     .arg(seenList.join(", "));
        }

        rows.append(ReportRow(
            QString::fromUtf8("毫米波告警-%1-%2/%3").arg(ch.name, ch.dir1, ch.dir2),
            QString::fromUtf8("%1, %2").arg(ch.dir1, ch.dir2),
            result,
            allOk ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));
    }

    return rows;
}

QList<ReportRow> MainWindow::collectDiwoRows() const
{
    QList<ReportRow> rows;

    struct DiwoInfo {
        QString key;
        QString displayName;
    };
    const DiwoInfo indicators[] = {
        { QString::fromUtf8("主机"), QString::fromUtf8("主机指示灯") },
        { QString::fromUtf8("询问"), QString::fromUtf8("询问指示灯(毫米波)") },
        { QString::fromUtf8("应答"), QString::fromUtf8("应答指示灯") },
    };

    for (const auto &ind : indicators) {
        QSet<QString> values = m_diwoStates.value(ind.key);
        bool hasZero = values.contains("0");
        bool hasOne  = values.contains("1");
        bool allOk = hasZero && hasOne;

        QString detectedStr;
        if (values.isEmpty()) {
            detectedStr = QString::fromUtf8("未检测到数据");
        } else {
            QStringList vals;
            if (hasZero) vals.append("0");
            if (hasOne)  vals.append("1");
            detectedStr = QString::fromUtf8("检测到: %1").arg(vals.join(", "));
        }

        rows.append(ReportRow(
            QString::fromUtf8("敌我识别-%1").arg(ind.displayName),
            QString::fromUtf8("0和1状态均出现"),
            detectedStr,
            allOk ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));
    }

    return rows;
}

QList<ReportRow> MainWindow::collectLaserRows() const
{
    QList<ReportRow> rows;

    const QString indicatorNames[] = {
        QString::fromUtf8("左前"), QString::fromUtf8("右前"),
        QString::fromUtf8("左后"), QString::fromUtf8("右后")
    };

    for (const auto &name : indicatorNames) {
        bool lit = m_laserIndicatorsLit.contains(name);
        rows.append(ReportRow(
            QString::fromUtf8("激光告警-%1指示灯").arg(name),
            QString::fromUtf8("点亮"),
            lit ? QString::fromUtf8("已点亮") : QString::fromUtf8("未点亮"),
            lit ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));
    }

    return rows;
}

QList<ReportRow> MainWindow::collectSmokeRows() const
{
    QList<ReportRow> rows;

    const char* smokeNames[] = {"左1","左2","左3","左4","右1","右2","右3","右4"};

    for (const char* name : smokeNames) {
        QString key = QString::fromUtf8(name);
        QSet<QString> values = m_smokeStates.value(key);
        bool hasZero = values.contains("0");
        bool hasOne  = values.contains("1");
        bool allOk = hasZero && hasOne;

        QString detectedStr;
        if (values.isEmpty()) {
            detectedStr = QString::fromUtf8("未检测到数据");
        } else {
            QStringList vals;
            if (hasZero) vals.append("0");
            if (hasOne)  vals.append("1");
            detectedStr = QString::fromUtf8("检测到: %1").arg(vals.join(", "));
        }

        rows.append(ReportRow(
            QString::fromUtf8("烟幕弹%1").arg(key),
            QString::fromUtf8("0和1状态均出现"),
            detectedStr,
            allOk ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));
    }

    return rows;
}

QList<ReportRow> MainWindow::collectFireSuppressRows() const
{
    QList<ReportRow> rows;
    //bool hasZero = m_fireSuppressValues.contains("0");
    bool hasOne  = m_fireSuppressValues.contains("1") || m_fireSuppressValues.contains("2");
    bool allOk   = hasOne;

    QString detectedStr;
    if (m_fireSuppressValues.isEmpty()) {
        detectedStr = QString::fromUtf8("未检测到数据");
    } else {
        QStringList vals;
        //if (hasZero) vals.append("0");
        if (hasOne)  vals.append("1");
        detectedStr = QString::fromUtf8("检测到: %1").arg(vals.join(", "));
    }

    rows.append(ReportRow(
        QString::fromUtf8("灭火抑爆-指示灯"),
        QString::fromUtf8("状态正常"),
        detectedStr,
        allOk ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));

    return rows;
}

QList<ReportRow> MainWindow::collectTopComponentRows() const
{
    QList<ReportRow> rows;
    bool hasZero = m_topComponentValues.contains("1");
    bool hasOne  = m_topComponentValues.contains("2");

    QString detectedStr;
    if (m_topComponentValues.isEmpty()) {
        detectedStr = QString::fromUtf8("未检测到数据");
    } else {
        QStringList vals;
        if (hasZero) vals.append("1");
        if (hasOne)  vals.append("2");
        detectedStr = QString::fromUtf8("检测到: %1").arg(vals.join(", "));
    }

    rows.append(ReportRow(
        QString::fromUtf8("顶部组件-FlexRay指示灯"),
        QString::fromUtf8("1和2状态均出现"),
        detectedStr,
        hasZero && hasOne ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));

    rows.append(ReportRow(
        QString::fromUtf8("顶部组件-串口通信"),
        QString::fromUtf8("收到正确帧"),
        m_topComponentValues.isEmpty() ? QString::fromUtf8("未收到") : QString::fromUtf8("已收到"),
        !m_topComponentValues.isEmpty() ? QString::fromUtf8("合格") : QString::fromUtf8("不合格")));

    return rows;
}
