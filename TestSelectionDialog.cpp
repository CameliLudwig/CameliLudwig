#include "TestSelectionDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QDebug>

TestSelectionDialog::TestSelectionDialog(const QString &userName, QWidget *parent)
    : QDialog(parent)
    , m_selectedMode(ModeNone)
    , m_userName(userName)
{
    setupUI();
    applyStyle();
}

TestSelectionDialog::~TestSelectionDialog()
{
}

void TestSelectionDialog::setupUI()
{
    // 全屏无边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setWindowState(Qt::WindowFullScreen);
    setObjectName("TestSelectionDialog");

    // 字体
    {
        QFont f = this->font();
        f.setPointSize(12);
#ifdef Q_OS_WIN
        f.setFamily(QStringLiteral("Microsoft YaHei UI"));
#endif
        this->setFont(f);
    }

    // 外层布局：居中面板
    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setAlignment(Qt::AlignCenter);

    // 卡片面板 — 加高以容纳4个按钮
    m_panelFrame = new QFrame(this);
    m_panelFrame->setObjectName("panelFrame");
    m_panelFrame->setFixedSize(700, 680);

    QVBoxLayout *panelLayout = new QVBoxLayout(m_panelFrame);
    panelLayout->setAlignment(Qt::AlignCenter);
    panelLayout->setSpacing(12);
    panelLayout->setContentsMargins(40, 30, 40, 30);

    // 图标/标题
    m_titleLabel = new QLabel(QString::fromUtf8("防护采集驱动装置检测设备"), this);
    m_titleLabel->setObjectName("title");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    // 用户信息
    m_userLabel = new QLabel(QString::fromUtf8("当前用户：%1").arg(m_userName), this);
    m_userLabel->setObjectName("userLabel");
    m_userLabel->setAlignment(Qt::AlignCenter);

    // 分隔提示
    m_subtitleLabel = new QLabel(QString::fromUtf8("请选择测试项目"), this);
    m_subtitleLabel->setObjectName("subtitleLabel");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    // 按钮容器 — 两行两列
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(30, 5, 30, 5);

    // 第一行: 雷达板卡 | 敌我识别
    QHBoxLayout *row1 = new QHBoxLayout();
    row1->setSpacing(15);
    m_radarBoardButton = new QPushButton(QString::fromUtf8("毫米波告警测试"), this);
    m_radarBoardButton->setObjectName("radarBoardButton");
    m_radarBoardButton->setMinimumHeight(65);
    m_radarBoardButton->setCursor(Qt::PointingHandCursor);

    m_diwoButton = new QPushButton(QString::fromUtf8("敌我识别测试"), this);
    m_diwoButton->setObjectName("diwoButton");
    m_diwoButton->setMinimumHeight(65);
    m_diwoButton->setCursor(Qt::PointingHandCursor);

    row1->addWidget(m_radarBoardButton);
    row1->addWidget(m_diwoButton);

    // 第二行: 激光告警 | 烟幕弹
    QHBoxLayout *row2 = new QHBoxLayout();
    row2->setSpacing(15);
    m_laserButton = new QPushButton(QString::fromUtf8("激光告警测试"), this);
    m_laserButton->setObjectName("laserButton");
    m_laserButton->setMinimumHeight(65);
    m_laserButton->setCursor(Qt::PointingHandCursor);

    m_smokeButton = new QPushButton(QString::fromUtf8("烟幕弹测试"), this);
    m_smokeButton->setObjectName("smokeButton");
    m_smokeButton->setMinimumHeight(65);
    m_smokeButton->setCursor(Qt::PointingHandCursor);

    row2->addWidget(m_laserButton);
    row2->addWidget(m_smokeButton);

    // 第三行: 灭火抑爆 | 顶部组件
    QHBoxLayout *row3 = new QHBoxLayout();
    row3->setSpacing(15);
    m_fireSuppressButton = new QPushButton(QString::fromUtf8("灭火抑爆测试"), this);
    m_fireSuppressButton->setObjectName("fireSuppressButton");
    m_fireSuppressButton->setMinimumHeight(65);
    m_fireSuppressButton->setCursor(Qt::PointingHandCursor);

    m_topComponentButton = new QPushButton(QString::fromUtf8("顶部组件测试"), this);
    m_topComponentButton->setObjectName("topComponentButton");
    m_topComponentButton->setMinimumHeight(65);
    m_topComponentButton->setCursor(Qt::PointingHandCursor);

    row3->addWidget(m_fireSuppressButton);
    row3->addWidget(m_topComponentButton);

    buttonLayout->addLayout(row1);
    buttonLayout->addLayout(row2);
    buttonLayout->addLayout(row3);

    // 返回按钮
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(30, 8, 30, 0);
    m_returnButton = new QPushButton(QString::fromUtf8("返回登录"), this);
    m_returnButton->setObjectName("returnButton");
    m_returnButton->setMinimumHeight(42);
    m_returnButton->setFixedWidth(200);
    m_returnButton->setCursor(Qt::PointingHandCursor);
    bottomLayout->addStretch();
    bottomLayout->addWidget(m_returnButton);
    bottomLayout->addStretch();

    // 组装
    panelLayout->addSpacing(5);
    panelLayout->addWidget(m_titleLabel);
    panelLayout->addSpacing(3);
    panelLayout->addWidget(m_userLabel);
    panelLayout->addSpacing(3);
    panelLayout->addWidget(m_subtitleLabel);
    panelLayout->addSpacing(15);
    panelLayout->addLayout(buttonLayout);
    panelLayout->addSpacing(8);
    panelLayout->addLayout(bottomLayout);
    panelLayout->addSpacing(5);

    outerLayout->addWidget(m_panelFrame);

    // 信号连接
    connect(m_radarBoardButton, &QPushButton::clicked, this, &TestSelectionDialog::onRadarBoardClicked);
    connect(m_diwoButton,       &QPushButton::clicked, this, &TestSelectionDialog::onDiwoClicked);
    connect(m_laserButton,      &QPushButton::clicked, this, &TestSelectionDialog::onLaserClicked);
    connect(m_smokeButton,         &QPushButton::clicked, this, &TestSelectionDialog::onSmokeClicked);
    connect(m_fireSuppressButton,  &QPushButton::clicked, this, &TestSelectionDialog::onFireSuppressClicked);
    connect(m_topComponentButton,    &QPushButton::clicked, this, &TestSelectionDialog::onTopComponentClicked);
    connect(m_returnButton,        &QPushButton::clicked, this, &TestSelectionDialog::onReturnClicked);

    // 卡片阴影
    if (auto *shadow = new QGraphicsDropShadowEffect(this)) {
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 10);
        shadow->setColor(QColor(0, 0, 0, 80));
        m_panelFrame->setGraphicsEffect(shadow);
    }
}

void TestSelectionDialog::applyStyle()
{
    this->setStyleSheet(R"(
        /* 全局背景：图片 */
        QDialog#TestSelectionDialog {
            border-image: url(:/test.png);
        }

        /* 中间卡片 */
        #panelFrame {
            background-color: rgba(255, 255, 255, 0.96);
            border-radius: 16px;
        }

        /* 标题 */
        #title {
            color: #0f2027;
            font-size: 24px;
            font-weight: bold;
            letter-spacing: 2px;
            background: transparent;
        }

        /* 用户名 */
        #userLabel {
            color: #555;
            font-size: 15px;
            background: transparent;
            font-weight: normal;
        }

        /* 子标题 */
        #subtitleLabel {
            color: #888;
            font-size: 17px;
            background: transparent;
            font-weight: normal;
            letter-spacing: 1px;
        }

        /* 雷达板卡按钮 — 绿色调 */
        #radarBoardButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #2E7D32, stop:1 #43A047);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        #radarBoardButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #1B5E20, stop:1 #2E7D32);
        }
        #radarBoardButton:pressed {
            background: #1B5E20;
            padding-top: 4px;
        }

        /* 敌我识别按钮 — 深蓝色调 */
        #diwoButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #1565C0, stop:1 #1E88E5);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        #diwoButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #0D47A1, stop:1 #1565C0);
        }
        #diwoButton:pressed {
            background: #0D47A1;
            padding-top: 4px;
        }

        /* 激光告警按钮 — 蓝色调 */
        #laserButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #1E88E5, stop:1 #42A5F5);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        #laserButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #1565C0, stop:1 #1E88E5);
        }
        #laserButton:pressed {
            background: #0D47A1;
            padding-top: 4px;
        }

        /* 烟幕弹按钮 — 橙色调 */
        #smokeButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #E65100, stop:1 #FF9800);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        #smokeButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #BF360C, stop:1 #E65100);
        }
        #smokeButton:pressed {
            background: #BF360C;
            padding-top: 4px;
        }

        /* 灭火抑爆按钮 — 红色调 */
        #fireSuppressButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #C62828, stop:1 #EF5350);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        #fireSuppressButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #B71C1C, stop:1 #E53935);
        }
        #fireSuppressButton:pressed {
            background: #B71C1C;
            padding-top: 4px;
        }

        /* 顶部组件按钮 — 紫色调 */
        #topComponentButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #6A1B9A, stop:1 #AB47BC);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 2px;
        }
        #topComponentButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                        stop:0 #4A148C, stop:1 #8E24AA);
        }
        #topComponentButton:pressed {
            background: #4A148C;
            padding-top: 4px;
        }

        /* 返回按钮 */
        #returnButton {
            background: rgba(0, 0, 0, 0.08);
            color: #666;
            border: 1px solid #ddd;
            border-radius: 8px;
            font-size: 15px;
            font-weight: bold;
        }
        #returnButton:hover {
            background: rgba(0, 0, 0, 0.15);
            color: #333;
        }
        #returnButton:pressed {
            background: rgba(0, 0, 0, 0.25);
        }
    )");
}

void TestSelectionDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        return; // 拦截 ESC，防止意外退出
    }
    QDialog::keyPressEvent(event);
}

void TestSelectionDialog::onRadarBoardClicked()
{
    qDebug() << "[TestSelection] 用户点击了「雷达板卡测试」";
    m_selectedMode = ModeRadarBoard;
    emit testModeSelected(ModeRadarBoard);
    accept();
}

void TestSelectionDialog::onDiwoClicked()
{
    qDebug() << "[TestSelection] 用户点击了「敌我识别测试」";
    m_selectedMode = ModeDiwo;
    emit testModeSelected(ModeDiwo);
    accept();
}

void TestSelectionDialog::onLaserClicked()
{
    qDebug() << "[TestSelection] 用户点击了「激光告警测试」";
    m_selectedMode = ModeLaser;
    emit testModeSelected(ModeLaser);
    accept();
}

void TestSelectionDialog::onSmokeClicked()
{
    qDebug() << "[TestSelection] 用户点击了「烟幕弹测试」";
    m_selectedMode = ModeSmoke;
    emit testModeSelected(ModeSmoke);
    accept();
}

void TestSelectionDialog::onFireSuppressClicked()
{
    qDebug() << "[TestSelection] 用户点击了「灭火抑爆测试」";
    m_selectedMode = ModeFireSuppress;
    emit testModeSelected(ModeFireSuppress);
    accept();
}

void TestSelectionDialog::onTopComponentClicked()
{
    qDebug() << "[TestSelection] 用户点击了「顶部组件测试」";
    m_selectedMode = ModeTopComponent;
    emit testModeSelected(ModeTopComponent);
    accept();
}

void TestSelectionDialog::onReturnClicked()
{
    qDebug() << "[TestSelection] 用户点击了「返回登录」";
    m_selectedMode = ModeNone;
    emit requestReturnToLogin();
    reject();
}
