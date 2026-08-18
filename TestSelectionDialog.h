#ifndef TESTSELECTIONDIALOG_H
#define TESTSELECTIONDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QFrame>

/**
 * TestSelectionDialog
 * - 登录后显示的测试选择界面
 * - 四个大按钮：雷达板卡测试 / 敌我识别测试 / 激光告警测试 / 烟幕弹测试
 * - 全屏蓝色科技风，与 UserAdminDialog 风格一致
 */
class TestSelectionDialog : public QDialog
{
    Q_OBJECT
public:
    enum TestMode {
        ModeNone = 0,
        ModeRadarBoard,   // 毫米波告警测试
        ModeDiwo,         // 敌我识别测试
        ModeLaser,        // 激光告警测试
        ModeSmoke,        // 烟幕弹测试
        ModeFireSuppress, // 灭火抑爆测试
        ModeTopComponent // 顶部组件测试
    };

    explicit TestSelectionDialog(const QString &userName, QWidget *parent = nullptr);
    ~TestSelectionDialog();

    TestMode selectedMode() const { return m_selectedMode; }

signals:
    void testModeSelected(TestMode mode);
    void requestReturnToLogin();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onRadarBoardClicked();
    void onDiwoClicked();
    void onLaserClicked();
    void onSmokeClicked();
    void onFireSuppressClicked();
    void onTopComponentClicked();
    void onReturnClicked();

private:
    void setupUI();
    void applyStyle();

    TestMode m_selectedMode;
    QString  m_userName;

    // UI 组件
    QFrame     *m_panelFrame;
    QLabel     *m_titleLabel;
    QLabel     *m_userLabel;
    QLabel     *m_subtitleLabel;
    QPushButton *m_radarBoardButton;
    QPushButton *m_diwoButton;
    QPushButton *m_laserButton;
    QPushButton *m_smokeButton;
    QPushButton *m_fireSuppressButton;
    QPushButton *m_topComponentButton;
    QPushButton *m_returnButton;
};

#endif // TESTSELECTIONDIALOG_H
