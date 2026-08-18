#ifndef USERADMINDIALOG_H
#define USERADMINDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QHash>
#include <QIcon>
#include <QSize>
#include "mainwindow.h"
#include "TestSelectionDialog.h"

namespace Ui { class UserAdminDialog; }

/**
 * UserAdminDialog
 * - 全屏沉浸式登录/管理界面
 * - 蓝色科技风主题
 * - 卡片式交互，防止视觉分散
 */
class UserAdminDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserAdminDialog(QWidget* parent = nullptr);
    ~UserAdminDialog();

    void setAppIcon(const QIcon& icon);
    QString currentUser() const { return m_currentUser; }

protected:
    // 允许通过 ESC 退出全屏或系统吗？这里拦截一下防止意外关闭
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void doLogin();
    void doChangePassword();
    void verifyAdmin();
    void addUser();
    void renameUser();
    void resetPassword();
    void deleteUser();
    void onTabChanged(int index);
    void onExitSystem(); // 新增：退出系统

private:
    Ui::UserAdminDialog* ui{};
    QStringList                 m_personnel;
    QHash<QString, QByteArray>  m_userSha;
    QByteArray                  m_adminPinSha;
    QString                     m_currentUser;
    QIcon                       m_appIcon;

    // 持久化
    QString m_storagePath;
    bool loadFromDisk(QString* error = nullptr);
    bool saveToDisk(QString* error = nullptr) const;

    // 加解密工具
    QByteArray packPlain() const;
    bool       unpackPlain(const QByteArray& p);
    bool       encrypt(const QByteArray& plain, QByteArray& cipher) const;
    bool       decrypt(const QByteArray& cipher, QByteArray& plain) const;
    static QByteArray toSha(const QString& plainOrShaHex);
    static QString    shaHex(const QByteArray& sha);

    void setupUiData();
    void rememberLastUser(const QString& u) const;
    QString loadLastUser() const;

    // 界面尺寸控制 (控制的是中间 Panel 的大小，而不是 Window)
    const QSize kSmallPanelSize = QSize(520, 500); // 登录/改密时的卡片大小
    const QSize kLargePanelSize = QSize(1000, 750); // 管理界面时的卡片大小

    int  adminTabIndex() const;
    void applySmallMode();   // 切换到小卡片模式
    void applyLargeMode();   // 切换到大卡片模式
    void showTestSelectionDialog(); // 显示测试选择对话框

    MainWindow* m_main = nullptr;
};

#endif // USERADMINDIALOG_H
