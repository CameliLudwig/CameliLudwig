/********************************************************************************
** Form generated from reading UI file 'UserAdminDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.12.9
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERADMINDIALOG_H
#define UI_USERADMINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserAdminDialog
{
public:
    QGridLayout *outerLayout;
    QHBoxLayout *topBarLayout;
    QSpacerItem *topSpacer;
    QPushButton *btnExitSystem;
    QSpacerItem *leftSpacer;
    QFrame *panelFrame;
    QVBoxLayout *panelLayout;
    QLabel *title;
    QTabWidget *tab;
    QWidget *pageLogin;
    QVBoxLayout *v_login;
    QLabel *lblUserIcon;
    QComboBox *userCombo;
    QLabel *lblPwdIcon;
    QLineEdit *loginPwd;
    QSpacerItem *loginSpacer;
    QPushButton *btnLogin;
    QWidget *pageChange;
    QFormLayout *formChange;
    QLabel *lc_u;
    QComboBox *cpUserCombo;
    QLabel *lc_o;
    QLineEdit *cpOldPwd;
    QLabel *lc_n1;
    QLineEdit *cpNewPwd;
    QLabel *lc_n2;
    QLineEdit *cpNewPwd2;
    QPushButton *btnCpApply;
    QWidget *pageAdmin;
    QVBoxLayout *v_admin_main;
    QGroupBox *grpVerify;
    QHBoxLayout *h_ver;
    QLineEdit *adminPin;
    QPushButton *btnVerify;
    QStackedWidget *adminStack;
    QWidget *pageAdminCompact;
    QVBoxLayout *v_compact;
    QSpacerItem *compactSpacer;
    QLabel *lblCompact;
    QSpacerItem *compactSpacer2;
    QWidget *pageAdminPanel;
    QVBoxLayout *v_panel;
    QScrollArea *adminScroll;
    QWidget *adminScrollContents;
    QVBoxLayout *v_scroll;
    QGroupBox *grpUsers;
    QHBoxLayout *h_users;
    QListWidget *userList;
    QVBoxLayout *v_ops;
    QGroupBox *gbAdd;
    QFormLayout *formAdd;
    QLabel *la1;
    QLineEdit *addName;
    QLabel *la2;
    QLineEdit *addPwd;
    QLabel *la3;
    QLineEdit *addPwd2;
    QPushButton *btnAdd;
    QGroupBox *gbRename;
    QVBoxLayout *v_ren;
    QLineEdit *renameEdit;
    QPushButton *btnRename;
    QGroupBox *gbReset;
    QFormLayout *formReset;
    QLabel *lr1;
    QLineEdit *resetPwd;
    QLabel *lr2;
    QLineEdit *resetPwd2;
    QPushButton *btnReset;
    QPushButton *btnDelete;
    QSpacerItem *rightSpacer;
    QSpacerItem *bottomSpacer;

    void setupUi(QDialog *UserAdminDialog)
    {
        if (UserAdminDialog->objectName().isEmpty())
            UserAdminDialog->setObjectName(QString::fromUtf8("UserAdminDialog"));
        outerLayout = new QGridLayout(UserAdminDialog);
        outerLayout->setObjectName(QString::fromUtf8("outerLayout"));
        outerLayout->setContentsMargins(0, 0, 0, 0);
        topBarLayout = new QHBoxLayout();
        topBarLayout->setObjectName(QString::fromUtf8("topBarLayout"));
        topBarLayout->setContentsMargins(-1, 20, 20, -1);
        topSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        topBarLayout->addItem(topSpacer);

        btnExitSystem = new QPushButton(UserAdminDialog);
        btnExitSystem->setObjectName(QString::fromUtf8("btnExitSystem"));
        btnExitSystem->setCursor(QCursor(Qt::PointingHandCursor));
        btnExitSystem->setFocusPolicy(Qt::NoFocus);

        topBarLayout->addWidget(btnExitSystem);


        outerLayout->addLayout(topBarLayout, 0, 0, 1, 3);

        leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        outerLayout->addItem(leftSpacer, 1, 0, 1, 1);

        panelFrame = new QFrame(UserAdminDialog);
        panelFrame->setObjectName(QString::fromUtf8("panelFrame"));
        panelFrame->setFrameShape(QFrame::NoFrame);
        panelFrame->setFrameShadow(QFrame::Raised);
        panelLayout = new QVBoxLayout(panelFrame);
        panelLayout->setSpacing(20);
        panelLayout->setObjectName(QString::fromUtf8("panelLayout"));
        panelLayout->setContentsMargins(40, 40, 40, 40);
        title = new QLabel(panelFrame);
        title->setObjectName(QString::fromUtf8("title"));
        title->setAlignment(Qt::AlignCenter);

        panelLayout->addWidget(title);

        tab = new QTabWidget(panelFrame);
        tab->setObjectName(QString::fromUtf8("tab"));
        pageLogin = new QWidget();
        pageLogin->setObjectName(QString::fromUtf8("pageLogin"));
        v_login = new QVBoxLayout(pageLogin);
        v_login->setSpacing(20);
        v_login->setObjectName(QString::fromUtf8("v_login"));
        v_login->setContentsMargins(20, 30, 20, 20);
        lblUserIcon = new QLabel(pageLogin);
        lblUserIcon->setObjectName(QString::fromUtf8("lblUserIcon"));

        v_login->addWidget(lblUserIcon);

        userCombo = new QComboBox(pageLogin);
        userCombo->setObjectName(QString::fromUtf8("userCombo"));

        v_login->addWidget(userCombo);

        lblPwdIcon = new QLabel(pageLogin);
        lblPwdIcon->setObjectName(QString::fromUtf8("lblPwdIcon"));

        v_login->addWidget(lblPwdIcon);

        loginPwd = new QLineEdit(pageLogin);
        loginPwd->setObjectName(QString::fromUtf8("loginPwd"));
        loginPwd->setEchoMode(QLineEdit::Password);

        v_login->addWidget(loginPwd);

        loginSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        v_login->addItem(loginSpacer);

        btnLogin = new QPushButton(pageLogin);
        btnLogin->setObjectName(QString::fromUtf8("btnLogin"));
        btnLogin->setCursor(QCursor(Qt::PointingHandCursor));

        v_login->addWidget(btnLogin);

        tab->addTab(pageLogin, QString());
        pageChange = new QWidget();
        pageChange->setObjectName(QString::fromUtf8("pageChange"));
        formChange = new QFormLayout(pageChange);
        formChange->setObjectName(QString::fromUtf8("formChange"));
        formChange->setHorizontalSpacing(15);
        formChange->setVerticalSpacing(15);
        formChange->setContentsMargins(-1, 20, -1, -1);
        lc_u = new QLabel(pageChange);
        lc_u->setObjectName(QString::fromUtf8("lc_u"));

        formChange->setWidget(0, QFormLayout::LabelRole, lc_u);

        cpUserCombo = new QComboBox(pageChange);
        cpUserCombo->setObjectName(QString::fromUtf8("cpUserCombo"));

        formChange->setWidget(0, QFormLayout::FieldRole, cpUserCombo);

        lc_o = new QLabel(pageChange);
        lc_o->setObjectName(QString::fromUtf8("lc_o"));

        formChange->setWidget(1, QFormLayout::LabelRole, lc_o);

        cpOldPwd = new QLineEdit(pageChange);
        cpOldPwd->setObjectName(QString::fromUtf8("cpOldPwd"));
        cpOldPwd->setEchoMode(QLineEdit::Password);

        formChange->setWidget(1, QFormLayout::FieldRole, cpOldPwd);

        lc_n1 = new QLabel(pageChange);
        lc_n1->setObjectName(QString::fromUtf8("lc_n1"));

        formChange->setWidget(2, QFormLayout::LabelRole, lc_n1);

        cpNewPwd = new QLineEdit(pageChange);
        cpNewPwd->setObjectName(QString::fromUtf8("cpNewPwd"));
        cpNewPwd->setEchoMode(QLineEdit::Password);

        formChange->setWidget(2, QFormLayout::FieldRole, cpNewPwd);

        lc_n2 = new QLabel(pageChange);
        lc_n2->setObjectName(QString::fromUtf8("lc_n2"));

        formChange->setWidget(3, QFormLayout::LabelRole, lc_n2);

        cpNewPwd2 = new QLineEdit(pageChange);
        cpNewPwd2->setObjectName(QString::fromUtf8("cpNewPwd2"));
        cpNewPwd2->setEchoMode(QLineEdit::Password);

        formChange->setWidget(3, QFormLayout::FieldRole, cpNewPwd2);

        btnCpApply = new QPushButton(pageChange);
        btnCpApply->setObjectName(QString::fromUtf8("btnCpApply"));

        formChange->setWidget(4, QFormLayout::FieldRole, btnCpApply);

        tab->addTab(pageChange, QString());
        pageAdmin = new QWidget();
        pageAdmin->setObjectName(QString::fromUtf8("pageAdmin"));
        v_admin_main = new QVBoxLayout(pageAdmin);
        v_admin_main->setObjectName(QString::fromUtf8("v_admin_main"));
        grpVerify = new QGroupBox(pageAdmin);
        grpVerify->setObjectName(QString::fromUtf8("grpVerify"));
        h_ver = new QHBoxLayout(grpVerify);
        h_ver->setObjectName(QString::fromUtf8("h_ver"));
        adminPin = new QLineEdit(grpVerify);
        adminPin->setObjectName(QString::fromUtf8("adminPin"));
        adminPin->setEchoMode(QLineEdit::Password);

        h_ver->addWidget(adminPin);

        btnVerify = new QPushButton(grpVerify);
        btnVerify->setObjectName(QString::fromUtf8("btnVerify"));

        h_ver->addWidget(btnVerify);


        v_admin_main->addWidget(grpVerify);

        adminStack = new QStackedWidget(pageAdmin);
        adminStack->setObjectName(QString::fromUtf8("adminStack"));
        pageAdminCompact = new QWidget();
        pageAdminCompact->setObjectName(QString::fromUtf8("pageAdminCompact"));
        v_compact = new QVBoxLayout(pageAdminCompact);
        v_compact->setObjectName(QString::fromUtf8("v_compact"));
        compactSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        v_compact->addItem(compactSpacer);

        lblCompact = new QLabel(pageAdminCompact);
        lblCompact->setObjectName(QString::fromUtf8("lblCompact"));
        lblCompact->setAlignment(Qt::AlignCenter);

        v_compact->addWidget(lblCompact);

        compactSpacer2 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        v_compact->addItem(compactSpacer2);

        adminStack->addWidget(pageAdminCompact);
        pageAdminPanel = new QWidget();
        pageAdminPanel->setObjectName(QString::fromUtf8("pageAdminPanel"));
        v_panel = new QVBoxLayout(pageAdminPanel);
        v_panel->setContentsMargins(0, 0, 0, 0);
        v_panel->setObjectName(QString::fromUtf8("v_panel"));
        adminScroll = new QScrollArea(pageAdminPanel);
        adminScroll->setObjectName(QString::fromUtf8("adminScroll"));
        adminScroll->setWidgetResizable(true);
        adminScrollContents = new QWidget();
        adminScrollContents->setObjectName(QString::fromUtf8("adminScrollContents"));
        v_scroll = new QVBoxLayout(adminScrollContents);
        v_scroll->setObjectName(QString::fromUtf8("v_scroll"));
        grpUsers = new QGroupBox(adminScrollContents);
        grpUsers->setObjectName(QString::fromUtf8("grpUsers"));
        h_users = new QHBoxLayout(grpUsers);
        h_users->setObjectName(QString::fromUtf8("h_users"));
        userList = new QListWidget(grpUsers);
        userList->setObjectName(QString::fromUtf8("userList"));

        h_users->addWidget(userList);

        v_ops = new QVBoxLayout();
        v_ops->setObjectName(QString::fromUtf8("v_ops"));
        gbAdd = new QGroupBox(grpUsers);
        gbAdd->setObjectName(QString::fromUtf8("gbAdd"));
        formAdd = new QFormLayout(gbAdd);
        formAdd->setObjectName(QString::fromUtf8("formAdd"));
        la1 = new QLabel(gbAdd);
        la1->setObjectName(QString::fromUtf8("la1"));

        formAdd->setWidget(0, QFormLayout::LabelRole, la1);

        addName = new QLineEdit(gbAdd);
        addName->setObjectName(QString::fromUtf8("addName"));

        formAdd->setWidget(0, QFormLayout::FieldRole, addName);

        la2 = new QLabel(gbAdd);
        la2->setObjectName(QString::fromUtf8("la2"));

        formAdd->setWidget(1, QFormLayout::LabelRole, la2);

        addPwd = new QLineEdit(gbAdd);
        addPwd->setObjectName(QString::fromUtf8("addPwd"));
        addPwd->setEchoMode(QLineEdit::Password);

        formAdd->setWidget(1, QFormLayout::FieldRole, addPwd);

        la3 = new QLabel(gbAdd);
        la3->setObjectName(QString::fromUtf8("la3"));

        formAdd->setWidget(2, QFormLayout::LabelRole, la3);

        addPwd2 = new QLineEdit(gbAdd);
        addPwd2->setObjectName(QString::fromUtf8("addPwd2"));
        addPwd2->setEchoMode(QLineEdit::Password);

        formAdd->setWidget(2, QFormLayout::FieldRole, addPwd2);

        btnAdd = new QPushButton(gbAdd);
        btnAdd->setObjectName(QString::fromUtf8("btnAdd"));

        formAdd->setWidget(3, QFormLayout::FieldRole, btnAdd);


        v_ops->addWidget(gbAdd);

        gbRename = new QGroupBox(grpUsers);
        gbRename->setObjectName(QString::fromUtf8("gbRename"));
        v_ren = new QVBoxLayout(gbRename);
        v_ren->setObjectName(QString::fromUtf8("v_ren"));
        renameEdit = new QLineEdit(gbRename);
        renameEdit->setObjectName(QString::fromUtf8("renameEdit"));

        v_ren->addWidget(renameEdit);

        btnRename = new QPushButton(gbRename);
        btnRename->setObjectName(QString::fromUtf8("btnRename"));

        v_ren->addWidget(btnRename);


        v_ops->addWidget(gbRename);

        gbReset = new QGroupBox(grpUsers);
        gbReset->setObjectName(QString::fromUtf8("gbReset"));
        formReset = new QFormLayout(gbReset);
        formReset->setObjectName(QString::fromUtf8("formReset"));
        lr1 = new QLabel(gbReset);
        lr1->setObjectName(QString::fromUtf8("lr1"));

        formReset->setWidget(0, QFormLayout::LabelRole, lr1);

        resetPwd = new QLineEdit(gbReset);
        resetPwd->setObjectName(QString::fromUtf8("resetPwd"));
        resetPwd->setEchoMode(QLineEdit::Password);

        formReset->setWidget(0, QFormLayout::FieldRole, resetPwd);

        lr2 = new QLabel(gbReset);
        lr2->setObjectName(QString::fromUtf8("lr2"));

        formReset->setWidget(1, QFormLayout::LabelRole, lr2);

        resetPwd2 = new QLineEdit(gbReset);
        resetPwd2->setObjectName(QString::fromUtf8("resetPwd2"));
        resetPwd2->setEchoMode(QLineEdit::Password);

        formReset->setWidget(1, QFormLayout::FieldRole, resetPwd2);

        btnReset = new QPushButton(gbReset);
        btnReset->setObjectName(QString::fromUtf8("btnReset"));

        formReset->setWidget(2, QFormLayout::FieldRole, btnReset);


        v_ops->addWidget(gbReset);

        btnDelete = new QPushButton(grpUsers);
        btnDelete->setObjectName(QString::fromUtf8("btnDelete"));

        v_ops->addWidget(btnDelete);


        h_users->addLayout(v_ops);


        v_scroll->addWidget(grpUsers);

        adminScroll->setWidget(adminScrollContents);

        v_panel->addWidget(adminScroll);

        adminStack->addWidget(pageAdminPanel);

        v_admin_main->addWidget(adminStack);

        tab->addTab(pageAdmin, QString());

        panelLayout->addWidget(tab);


        outerLayout->addWidget(panelFrame, 1, 1, 1, 1);

        rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

        outerLayout->addItem(rightSpacer, 1, 2, 1, 1);

        bottomSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        outerLayout->addItem(bottomSpacer, 2, 0, 1, 3);


        retranslateUi(UserAdminDialog);

        tab->setCurrentIndex(0);
        adminStack->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(UserAdminDialog);
    } // setupUi

    void retranslateUi(QDialog *UserAdminDialog)
    {
        UserAdminDialog->setWindowTitle(QApplication::translate("UserAdminDialog", "\347\224\250\346\210\267\346\235\203\351\231\220\347\256\241\347\220\206\347\263\273\347\273\237", nullptr));
        btnExitSystem->setText(QApplication::translate("UserAdminDialog", "\303\227", nullptr));
#ifndef QT_NO_TOOLTIP
        btnExitSystem->setToolTip(QApplication::translate("UserAdminDialog", "\351\200\200\345\207\272\347\263\273\347\273\237", nullptr));
#endif // QT_NO_TOOLTIP
        title->setText(QApplication::translate("UserAdminDialog", "\347\263\273\347\273\237\347\231\273\345\275\225", nullptr));
        lblUserIcon->setText(QApplication::translate("UserAdminDialog", "\351\200\211\346\213\251\347\224\250\346\210\267", nullptr));
        lblPwdIcon->setText(QApplication::translate("UserAdminDialog", "\347\231\273\345\275\225\345\257\206\347\240\201", nullptr));
        loginPwd->setPlaceholderText(QApplication::translate("UserAdminDialog", "\350\257\267\350\276\223\345\205\245\345\257\206\347\240\201", nullptr));
        btnLogin->setText(QApplication::translate("UserAdminDialog", "\347\253\213\345\215\263\347\231\273\345\275\225", nullptr));
        tab->setTabText(tab->indexOf(pageLogin), QApplication::translate("UserAdminDialog", "\350\272\253\344\273\275\351\252\214\350\257\201", nullptr));
        lc_u->setText(QApplication::translate("UserAdminDialog", "\347\224\250\346\210\267\345\220\215", nullptr));
        lc_o->setText(QApplication::translate("UserAdminDialog", "\346\227\247\345\257\206\347\240\201", nullptr));
        lc_n1->setText(QApplication::translate("UserAdminDialog", "\346\226\260\345\257\206\347\240\201", nullptr));
        lc_n2->setText(QApplication::translate("UserAdminDialog", "\347\241\256\350\256\244\346\226\260\345\257\206\347\240\201", nullptr));
        btnCpApply->setText(QApplication::translate("UserAdminDialog", "\347\241\256\350\256\244\344\277\256\346\224\271", nullptr));
        tab->setTabText(tab->indexOf(pageChange), QApplication::translate("UserAdminDialog", "\344\277\256\346\224\271\345\257\206\347\240\201", nullptr));
        grpVerify->setTitle(QApplication::translate("UserAdminDialog", "\346\235\203\351\231\220\351\252\214\350\257\201", nullptr));
        adminPin->setPlaceholderText(QApplication::translate("UserAdminDialog", "\350\257\267\350\276\223\345\205\245\347\256\241\347\220\206\345\221\230\345\217\243\344\273\244", nullptr));
        btnVerify->setText(QApplication::translate("UserAdminDialog", "\351\252\214\350\257\201", nullptr));
        lblCompact->setText(QApplication::translate("UserAdminDialog", "\350\257\267\351\252\214\350\257\201\347\256\241\347\220\206\345\221\230\350\272\253\344\273\275\344\273\245\347\256\241\347\220\206\344\272\272\345\221\230\346\225\260\346\215\256", nullptr));
        lblCompact->setStyleSheet(QApplication::translate("UserAdminDialog", "color: #888;", nullptr));
        grpUsers->setTitle(QApplication::translate("UserAdminDialog", "\344\272\272\345\221\230\346\225\260\346\215\256\345\272\223", nullptr));
        gbAdd->setTitle(QApplication::translate("UserAdminDialog", "\346\226\260\345\242\236\347\224\250\346\210\267", nullptr));
        la1->setText(QApplication::translate("UserAdminDialog", "\347\224\250\346\210\267\345\220\215", nullptr));
        la2->setText(QApplication::translate("UserAdminDialog", "\345\257\206\347\240\201", nullptr));
        la3->setText(QApplication::translate("UserAdminDialog", "\347\241\256\350\256\244", nullptr));
        btnAdd->setText(QApplication::translate("UserAdminDialog", "\346\267\273\345\212\240", nullptr));
        gbRename->setTitle(QApplication::translate("UserAdminDialog", "\347\224\250\346\210\267\347\273\264\346\212\244", nullptr));
        renameEdit->setPlaceholderText(QApplication::translate("UserAdminDialog", "\346\226\260\347\224\250\346\210\267\345\220\215", nullptr));
        btnRename->setText(QApplication::translate("UserAdminDialog", "\351\207\215\345\221\275\345\220\215\351\200\211\344\270\255\347\224\250\346\210\267", nullptr));
        gbReset->setTitle(QApplication::translate("UserAdminDialog", "\345\274\272\345\210\266\351\207\215\347\275\256\345\257\206\347\240\201", nullptr));
        lr1->setText(QApplication::translate("UserAdminDialog", "\346\226\260\345\257\206\347\240\201", nullptr));
        lr2->setText(QApplication::translate("UserAdminDialog", "\347\241\256\350\256\244", nullptr));
        btnReset->setText(QApplication::translate("UserAdminDialog", "\351\207\215\347\275\256", nullptr));
        btnDelete->setText(QApplication::translate("UserAdminDialog", "\345\210\240\351\231\244\351\200\211\344\270\255\347\224\250\346\210\267", nullptr));
        btnDelete->setStyleSheet(QApplication::translate("UserAdminDialog", "background-color: #ff5252; color: white;", nullptr));
        tab->setTabText(tab->indexOf(pageAdmin), QApplication::translate("UserAdminDialog", "\347\263\273\347\273\237\347\256\241\347\220\206", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UserAdminDialog: public Ui_UserAdminDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERADMINDIALOG_H
