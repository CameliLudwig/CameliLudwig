#include "UserAdminDialog.h"
#include "ui_UserAdminDialog.h"

#include <QCryptographicHash>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QRegularExpression>
#include <QGraphicsDropShadowEffect>
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QFont>
#include <QTimer>
#include <QDateTime>
#include <QKeyEvent>
#include <QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
  #include <QRandomGenerator>
#endif

static const char* ORG = "Workspace";
static const char* APP = "DetectionApp";

// -----------------------------------------------------------
// 加密辅助函数
// -----------------------------------------------------------
static QByteArray APP_KEY()
{
    const QByteArray seed = QByteArray("IntegratedAlarm-2765-Auth-Key");
    return QCryptographicHash::hash(seed, QCryptographicHash::Sha256);
}
static QByteArray kdfBlock(const QByteArray& key, const QByteArray& nonce, quint32 counter)
{
    QCryptographicHash h(QCryptographicHash::Sha256);
    h.addData(key);
    h.addData(nonce);
    h.addData(reinterpret_cast<const char*>(&counter), sizeof(counter));
    return h.result();
}
static QByteArray makeNonce(int nbytes = 16)
{
    QByteArray n; n.resize(nbytes);
#if QT_VERSION >= QT_VERSION_CHECK(5,10,0)
    auto* rg = QRandomGenerator::global();
    for (int i=0;i<nbytes;++i) n[i] = char(rg->generate() & 0xFF);
#else
    qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFFFFFF));
    for (int i=0;i<nbytes;++i) n[i] = char(qrand() & 0xFF);
#endif
    return n;
}

class BigItemDelegate : public QStyledItemDelegate {
public:
    explicit BigItemDelegate(QObject* parent=nullptr): QStyledItemDelegate(parent){}
    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const override{
        QSize s = QStyledItemDelegate::sizeHint(opt, idx);
        if (s.height() < 40) s.setHeight(40); // 增加下拉项高度
        return s;
    }
};

// -----------------------------------------------------------
// 类实现
// -----------------------------------------------------------

QByteArray UserAdminDialog::toSha(const QString& plainOrShaHex)
{
    static QRegularExpression hex64(QStringLiteral("^[0-9a-fA-F]{64}$"));
    if (hex64.match(plainOrShaHex).hasMatch())
        return QByteArray::fromHex(plainOrShaHex.toLatin1());
    return QCryptographicHash::hash(plainOrShaHex.toUtf8(), QCryptographicHash::Sha256);
}
QString UserAdminDialog::shaHex(const QByteArray& sha){ return QString::fromLatin1(sha.toHex()); }

UserAdminDialog::UserAdminDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::UserAdminDialog)
{
    ui->setupUi(this);

    // 1. 设置全屏无边框
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setWindowState(Qt::WindowFullScreen);

    // 2. 字体设置
    {
        QFont f = this->font();
        f.setPointSize(12);
#ifdef Q_OS_WIN
        f.setFamily(QStringLiteral("Microsoft YaHei UI"));
#endif
        this->setFont(f);
    }

    // 3. 核心样式表 (蓝色科技风)
    this->setStyleSheet(R"(
        /* 全局背景：图片 */
        QDialog#UserAdminDialog {
            border-image: url(:/test.png);
        }

        /* 中间卡片：白色半透明，磨砂质感 */
        #panelFrame {
            background-color: rgba(255, 255, 255, 0.96);
            border-radius: 16px;
        }

        /* 标题 */
        #title {
            color: #0f2027;
            font-size: 26px;
            font-weight: bold;
            letter-spacing: 2px;
            margin-bottom: 10px;
        }

        /* 标签页 TabWidget */
        QTabWidget::pane {
            border: 0;
            background: transparent;
        }
        QTabWidget::tab-bar {
            alignment: center; /* Tab居中 */
        }
        QTabBar::tab {
            background: transparent;
            color: #666;
            font-size: 16px;
            font-weight: bold;
            padding: 10px 20px;
            border-bottom: 3px solid transparent;
            min-width: 80px;
        }
        QTabBar::tab:selected {
            color: #1E88E5;
            border-bottom: 3px solid #1E88E5;
        }
        QTabBar::tab:hover {
            color: #1E88E5;
        }

        /* 标签文字 */
        QLabel {
            color: #333;
            font-size: 14px;
        }
        QLabel#lblCompact {
            color: #888;
            font-size: 16px;
        }

        /* 输入框 & 下拉框 */
        QLineEdit, QComboBox {
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            padding: 0 12px;
            background: #fdfdfd;
            font-size: 15px;
            min-height: 45px; /* 增高 */
            selection-background-color: #1E88E5;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #1E88E5;
            background: #fff;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 30px;
            border-left-width: 0px;
        }
        QComboBox QAbstractItemView {
            border: 1px solid #dcdfe6;
            selection-background-color: #e6f7ff;
            selection-color: #333;
            outline: none;
            padding: 5px;
        }

        /* 按钮：主色调蓝 */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1E88E5, stop:1 #42A5F5);
            color: white;
            border: none;
            border-radius: 6px;
            font-size: 16px;
            font-weight: bold;
            min-height: 45px;
            padding: 0 20px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1976D2, stop:1 #2196F3);
        }
        QPushButton:pressed {
            background: #1565C0;
            padding-top: 2px; /* 按下效果 */
        }

        /* 红色删除按钮 */
        QPushButton#btnDelete {
            background: #FF5252;
        }
        QPushButton#btnDelete:hover {
            background: #FF1744;
        }

        /* 退出系统按钮 (右上角) */
        QPushButton#btnExitSystem {
            background: rgba(255, 255, 255, 0.1);
            color: rgba(255, 255, 255, 0.6);
            font-size: 24px;
            font-weight: normal;
            border-radius: 20px;
            min-height: 40px;
            min-width: 40px;
            padding: 0;
        }
        QPushButton#btnExitSystem:hover {
            background: #FF5252;
            color: white;
        }

        /* 分组框 */
        QGroupBox {
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            margin-top: 20px;
            padding-top: 15px;
            font-size: 14px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 15px;
            padding: 0 5px;
            color: #1E88E5;
            font-weight: bold;
        }

        /* 列表 */
        QListWidget {
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            font-size: 15px;
            padding: 5px;
            outline: none;
        }
        QListWidget::item {
            height: 36px;
            padding-left: 5px;
        }
        QListWidget::item:selected {
            background-color: #e6f7ff;
            color: #1E88E5;
            border-radius: 4px;
        }
    )");

    // 4. 卡片阴影
    if (auto *shadow = new QGraphicsDropShadowEffect(this)) {
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 10);
        shadow->setColor(QColor(0, 0, 0, 80));
        ui->panelFrame->setGraphicsEffect(shadow);
    }

    if (!m_appIcon.isNull()) setWindowIcon(m_appIcon);

    // 5. 数据初始化
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    m_storagePath = QDir(base).filePath("auth.bin");

    QString err;
    if (!loadFromDisk(&err)) {
        m_personnel.clear();
        m_personnel << QString::fromUtf8("张仁") << QString::fromUtf8("郭继霜")
                    << QString::fromUtf8("王鹏飞") << QString::fromUtf8("柴丽");

        for (const auto& u : m_personnel) m_userSha[u] = toSha("123456");
        m_adminPinSha = toSha("2765");
        saveToDisk();
    }

    // 6. 信号连接
    connect(ui->btnLogin,   &QPushButton::clicked, this, &UserAdminDialog::doLogin);
    connect(ui->btnCpApply, &QPushButton::clicked, this, &UserAdminDialog::doChangePassword);
    connect(ui->btnVerify,  &QPushButton::clicked, this, &UserAdminDialog::verifyAdmin);
    connect(ui->btnAdd,     &QPushButton::clicked, this, &UserAdminDialog::addUser);
    connect(ui->btnRename,  &QPushButton::clicked, this, &UserAdminDialog::renameUser);
    connect(ui->btnReset,   &QPushButton::clicked, this, &UserAdminDialog::resetPassword);
    connect(ui->btnDelete,  &QPushButton::clicked, this, &UserAdminDialog::deleteUser);
    connect(ui->tab,        &QTabWidget::currentChanged, this, &UserAdminDialog::onTabChanged);
    connect(ui->btnExitSystem, &QPushButton::clicked, this, &UserAdminDialog::onExitSystem); // 退出

    setupUiData();

    // 7. 控件优化
    auto tuneCombo = [](QComboBox* c){
        c->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        c->setMinimumContentsLength(10);
        c->setItemDelegate(new BigItemDelegate(c));
        if (auto v = c->view()) v->setAlternatingRowColors(true);
    };
    tuneCombo(ui->userCombo);
    tuneCombo(ui->cpUserCombo);

    ui->adminScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->adminScroll->setFrameShape(QFrame::NoFrame);

    // 记忆上次登录
    const QString last = loadLastUser();
    int idx = ui->userCombo->findText(last);
    if (idx >= 0) ui->userCombo->setCurrentIndex(idx);

    // 8. 默认状态：显示小卡片
    QTimer::singleShot(0, this, [this]{
        ui->adminStack->setCurrentIndex(0);
        ui->tab->setCurrentIndex(0);
        applySmallMode();
    });
}

UserAdminDialog::~UserAdminDialog() { delete ui; }

void UserAdminDialog::keyPressEvent(QKeyEvent *event)
{
    // 防止误按 ESC 退出全屏导致程序关闭（如果是主窗口的话）
    if (event->key() == Qt::Key_Escape) {
        return;
    }
    QDialog::keyPressEvent(event);
}

void UserAdminDialog::showTestSelectionDialog()
{
    auto *dlg = new TestSelectionDialog(m_currentUser, this);

    // 用户选择了某个测试模式
    connect(dlg, &TestSelectionDialog::testModeSelected, this, [this, dlg](TestSelectionDialog::TestMode mode) {
        dlg->deleteLater();
        if (!m_main) return;

        MainWindow::TestMode mainMode;
        switch (mode) {
        case TestSelectionDialog::ModeRadarBoard: mainMode = MainWindow::ModeRadarBoard; break;
        case TestSelectionDialog::ModeDiwo:       mainMode = MainWindow::ModeDiwo;       break;
        case TestSelectionDialog::ModeLaser:      mainMode = MainWindow::ModeLaser;      break;
        case TestSelectionDialog::ModeSmoke:         mainMode = MainWindow::ModeSmoke;         break;
        case TestSelectionDialog::ModeFireSuppress:  mainMode = MainWindow::ModeFireSuppress;  break;
        case TestSelectionDialog::ModeTopComponent:    mainMode = MainWindow::ModeTopComponent;    break;
        default: return;
        }

        m_main->setTestMode(mainMode);
        m_main->showMaximized();

        // 连接 MainWindow 的返回信号
        connect(m_main, &MainWindow::requestReturn, this, [this]() {
            m_main->hide();
            showTestSelectionDialog();
        }, Qt::UniqueConnection);
    });

    // 用户点击了「返回登录」— 关闭所有设备并退出
    connect(dlg, &TestSelectionDialog::requestReturnToLogin, this, [this, dlg]() {
        dlg->deleteLater();
        if (m_main) {
            m_main->closeAllDevices();
            m_main->deleteLater();
            m_main = nullptr;
        }
        QApplication::quit();
    });

    dlg->show();
}

void UserAdminDialog::onExitSystem()
{
    if (QMessageBox::question(this, tr("退出"), tr("确定要退出系统吗？")) == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void UserAdminDialog::setAppIcon(const QIcon& icon)
{
    m_appIcon = icon;
    if (!m_appIcon.isNull()) setWindowIcon(m_appIcon);
}

// -----------------------------------------------------------
// 模式切换
// -----------------------------------------------------------
int UserAdminDialog::adminTabIndex() const { return ui->tab->indexOf(ui->pageAdmin); }

void UserAdminDialog::applySmallMode() {
    ui->panelFrame->setFixedSize(kSmallPanelSize);
}

void UserAdminDialog::applyLargeMode() {
    ui->panelFrame->setFixedSize(kLargePanelSize);
}

void UserAdminDialog::onTabChanged(int index) {
    const bool isAdminTab = (index == adminTabIndex());
    const bool verified   = (ui->adminStack->currentIndex() == 1);

    // 如果是管理员且已验证，显示大面板，否则显示小面板
    if (isAdminTab && verified) {
        applyLargeMode();
    } else {
        applySmallMode();
    }
}

// -----------------------------------------------------------
// 业务逻辑 (基本保持不变，适配新UI)
// -----------------------------------------------------------
void UserAdminDialog::setupUiData()
{
    ui->userCombo->clear();
    ui->userCombo->addItems(m_personnel);
    ui->cpUserCombo->clear();
    ui->cpUserCombo->addItems(m_personnel);
    ui->userList->clear();
    ui->userList->addItems(m_personnel);

    // 管理员页面的默认状态
    ui->grpUsers->setEnabled(false);
}

void UserAdminDialog::doLogin()
{
    const QString user = ui->userCombo->currentText().trimmed();
    const QString pwd  = ui->loginPwd->text();

    if (user.isEmpty() || pwd.isEmpty()){
        QMessageBox::warning(this, tr("提示"), tr("请输入完整的登录信息。"));
        return;
    }
    if (!m_userSha.contains(user) ||
        QCryptographicHash::hash(pwd.toUtf8(), QCryptographicHash::Sha256) != m_userSha.value(user)) {
        QMessageBox::critical(this, tr("登录失败"), tr("用户名或密码不正确。"));
        return;
    }

    m_currentUser = user;
    rememberLastUser(user);

    // 登录成功，先创建 MainWindow（隐藏）以初始化所有串口/FlexRay设备
    m_main = new MainWindow();
    m_main->setCurrentUser(m_currentUser);
    m_main->hide();  // 后台初始化设备，不显示

    // 显示测试选择对话框
    showTestSelectionDialog();
}

void UserAdminDialog::doChangePassword()
{
    const QString user = ui->cpUserCombo->currentText().trimmed();
    const QString oldp = ui->cpOldPwd->text();
    const QString np1  = ui->cpNewPwd->text();
    const QString np2  = ui->cpNewPwd2->text();

    if (user.isEmpty() || oldp.isEmpty() || np1.isEmpty() || np2.isEmpty()){
        QMessageBox::warning(this, tr("提示"), tr("请填写完整。"));
        return;
    }
    if (np1 != np2){
        QMessageBox::warning(this, tr("提示"), tr("两次新密码不一致。"));
        return;
    }
    if (!m_userSha.contains(user) ||
        QCryptographicHash::hash(oldp.toUtf8(), QCryptographicHash::Sha256) != m_userSha.value(user)){
        QMessageBox::critical(this, tr("失败"), tr("旧密码不正确。"));
        return;
    }
    m_userSha[user] = QCryptographicHash::hash(np1.toUtf8(), QCryptographicHash::Sha256);
    if (!saveToDisk()){
        QMessageBox::warning(this, tr("提示"), tr("修改成功，但保存到磁盘失败。"));
    }else{
        QMessageBox::information(this, tr("成功"), tr("密码已修改。"));
        ui->cpOldPwd->clear(); ui->cpNewPwd->clear(); ui->cpNewPwd2->clear();
    }
}

void UserAdminDialog::verifyAdmin()
{
    const QString pin = ui->adminPin->text();
    if (pin.isEmpty()){
        QMessageBox::warning(this, tr("提示"), tr("请输入管理员口令。"));
        return;
    }
    const QByteArray sha = QCryptographicHash::hash(pin.toUtf8(), QCryptographicHash::Sha256);
    if (sha != m_adminPinSha){
        QMessageBox::critical(this, tr("失败"), tr("管理员口令不正确。"));
        ui->grpUsers->setEnabled(false);
        ui->adminStack->setCurrentIndex(0);
        return;
    }

    ui->grpUsers->setEnabled(true);
    ui->adminStack->setCurrentIndex(1);
    applyLargeMode(); // 切换到大面板
}

// ... (Add, Rename, Reset, Delete, Persistence logic remains same as original but using new UI pointers) ...

void UserAdminDialog::addUser()
{
    // 逻辑同前，省略部分重复代码，核心是使用 ui->addName 等新对象
    // ...
    const QString name = ui->addName->text().trimmed();
    const QString p1   = ui->addPwd->text();
    const QString p2   = ui->addPwd2->text();
    if (name.isEmpty() || p1.isEmpty() || p2.isEmpty()) return;
    if (p1 != p2) { QMessageBox::warning(this, "提示", "密码不一致"); return; }
    if (m_userSha.contains(name)) { QMessageBox::warning(this, "提示", "用户已存在"); return; }

    m_personnel.append(name);
    m_userSha.insert(name, QCryptographicHash::hash(p1.toUtf8(), QCryptographicHash::Sha256));
    saveToDisk();
    setupUiData();
    ui->addName->clear(); ui->addPwd->clear(); ui->addPwd2->clear();
}

void UserAdminDialog::renameUser()
{
    auto items = ui->userList->selectedItems();
    if (items.isEmpty()) return;
    QString old = items.first()->text();
    QString nu = ui->renameEdit->text().trimmed();
    if (nu.isEmpty() || m_userSha.contains(nu)) return;

    QByteArray sha = m_userSha.take(old);
    m_userSha.insert(nu, sha);
    int idx = m_personnel.indexOf(old);
    if (idx>=0) m_personnel[idx] = nu;

    if (m_currentUser == old) m_currentUser = nu;
    saveToDisk();
    setupUiData();
    ui->renameEdit->clear();
}

void UserAdminDialog::resetPassword()
{
    auto items = ui->userList->selectedItems();
    if (items.isEmpty()) return;
    QString user = items.first()->text();
    QString p1 = ui->resetPwd->text();
    QString p2 = ui->resetPwd2->text();
    if (p1.isEmpty() || p1 != p2) return;

    m_userSha[user] = QCryptographicHash::hash(p1.toUtf8(), QCryptographicHash::Sha256);
    saveToDisk();
    ui->resetPwd->clear(); ui->resetPwd2->clear();
    QMessageBox::information(this, "成功", "重置成功");
}

void UserAdminDialog::deleteUser()
{
    auto items = ui->userList->selectedItems();
    if (items.isEmpty()) return;
    QString user = items.first()->text();
    if (QMessageBox::question(this, "确认", "确认删除？") != QMessageBox::Yes) return;

    m_userSha.remove(user);
    m_personnel.removeAll(user);
    saveToDisk();
    setupUiData();
}

// -----------------------------------------------------------
// 存储相关 (与原版保持一致)
// -----------------------------------------------------------
void UserAdminDialog::rememberLastUser(const QString& u) const
{
    QSettings s(ORG, APP);
    s.setValue("lastUser", u);
}
QString UserAdminDialog::loadLastUser() const
{
    QSettings s(ORG, APP);
    return s.value("lastUser").toString();
}

QByteArray UserAdminDialog::packPlain() const
{
    QJsonObject root;
    root["version"] = 1;
    QJsonArray arr;
    for (const auto& u : m_personnel) arr.append(u);
    root["personnel"] = arr;
    QJsonObject pw;
    for (auto it = m_userSha.begin(); it != m_userSha.end(); ++it)
        pw[it.key()] = shaHex(it.value());
    root["passwords"] = pw;
    root["adminPin"] = shaHex(m_adminPinSha.isEmpty() ? toSha("2765") : m_adminPinSha);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

bool UserAdminDialog::unpackPlain(const QByteArray& p)
{
    const auto doc = QJsonDocument::fromJson(p);
    if (!doc.isObject()) return false;
    const auto o = doc.object();

    QStringList per;
    for (const auto& v : o["personnel"].toArray()) per.append(v.toString());

    QHash<QString,QByteArray> pw;
    const auto pwo = o["passwords"].toObject();
    for (auto it = pwo.begin(); it != pwo.end(); ++it)
        pw[it.key()] = QByteArray::fromHex(it.value().toString().toLatin1());

    QByteArray adminSha = QByteArray::fromHex(o["adminPin"].toString().toLatin1());
    if (adminSha.isEmpty()) adminSha = toSha("2765");

    m_personnel = per;
    m_userSha = pw;
    m_adminPinSha = adminSha;
    return true;
}

bool UserAdminDialog::saveToDisk(QString* error) const
{
    if (m_storagePath.isEmpty()) return false;
    const QByteArray plain = packPlain();
    QByteArray enc;
    if (!encrypt(plain, enc)) return false;
    QSaveFile f(m_storagePath);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(enc);
    return f.commit();
}

bool UserAdminDialog::loadFromDisk(QString* error)
{
    QFile f(m_storagePath);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return false;
    const QByteArray enc = f.readAll();
    QByteArray plain;
    if (!decrypt(enc, plain)) return false;
    return unpackPlain(plain);
}

bool UserAdminDialog::encrypt(const QByteArray& plain, QByteArray& out) const
{
    const QByteArray key = APP_KEY();
    const QByteArray nonce = makeNonce(16);
    QByteArray cipher; cipher.resize(plain.size());
    quint32 ctr = 0;
    int off = 0;
    while (off < plain.size()){
        const QByteArray block = kdfBlock(key, nonce, ctr++);
        const int n = qMin(block.size(), plain.size()-off);
        for (int i=0;i<n;++i) cipher[off+i] = plain[off+i] ^ block[i];
        off += n;
    }
    out.clear();
    out.append("UA1", 3);
    out.append(nonce);
    out.append(cipher);
    return true;
}

bool UserAdminDialog::decrypt(const QByteArray& in, QByteArray& plain) const
{
    if (in.size() < 19 || in.left(3) != "UA1") return false;
    const QByteArray nonce = in.mid(3, 16);
    const QByteArray cipher = in.mid(19);
    const QByteArray key = APP_KEY();
    plain.resize(cipher.size());
    quint32 ctr = 0;
    int off = 0;
    while (off < cipher.size()){
        const QByteArray block = kdfBlock(key, nonce, ctr++);
        const int n = qMin(block.size(), cipher.size()-off);
        for (int i=0;i<n;++i) plain[off+i] = cipher[off+i] ^ block[i];
        off += n;
    }
    return true;
}
