#include "signin.h"
#include "ui_signin.h"
#include "QStyleOption"
#include "QPainter"
#include <QPaintEvent>
SignIn::SignIn(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SignIn)
{
    ui->setupUi(this);
    ui->lineEdit_Password->setEchoMode(QLineEdit::Password);
    //设置ui样式

    m_styleFile  = new QFile(":/style.qss", this); //获取资源文件的对象
    m_styleFile->open(QFile::ReadOnly);//只读模式
    QString styleSheet = QString(m_styleFile->readAll());//读出的数据转成字符串
    this->setStyleSheet(styleSheet);
    m_styleFile->close();//最后关闭这个文件

    // 设置窗口标志，移除最大化按钮
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    //禁用调节大小
    setFixedSize(this->width(), this->height());

}

SignIn::~SignIn()
{
    delete ui;
}
void SignIn::closeEvent(QCloseEvent *event)
{
    int iBtn = QMessageBox::information(this, "退出", "你确定要退出吗？", "确认", "取消", 0, 1);
        switch (iBtn) {
        case 0:
            if(m_start != nullptr)
            {
                m_start->deleteLater();
                m_start = nullptr;
            }
            if(m_add != nullptr)
            {
                m_add->deleteLater();
                m_add = nullptr;
            }


            m_styleFile->deleteLater();
            this->deleteLater();
            // 关闭窗口并退出程序
            event->accept(); // 确认关闭窗口
            break;
        case 1:
            event->ignore();
            break;
        default:
            break;
        }

}


void SignIn::on_pushButton_signin_clicked()
{
    // 获取用户名和密码
    QString username = ui->lineEdit_Username->text();
    QString inputPassword = ui->lineEdit_Password->text();
    if(username == "" || inputPassword == "")
    {
        QMessageBox::warning(nullptr, "登陆失败", "用户名或密码为空", tr("确认"));
        return;
    }

    // 获取当前应用程序的目录（.exe 所在目录）
    QString appDir = QCoreApplication::applicationDirPath();
    QString fileName = appDir + "/" + username + ".dat";

    // 创建 QFile 对象，尝试打开该文件
    QFile file(fileName);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // 读取文件中的密码
        QTextStream in(&file);
        QString storedPassword = in.readAll().trimmed();  // 读取并去除多余的空白字符

        // 关闭文件
        file.close();

        // 比较输入的密码与文件中的密码
        if (inputPassword == storedPassword) {
            QMessageBox::information(nullptr, "登陆成功", "登陆成功!", tr("确认"));

            // 跳转到下一界面
            m_start = new All_Start();
            connect(this, SIGNAL(Senduser(QString,QString,QString)), m_start, SLOT(onSetUser(QString,QString,QString)));
            Senduser(ui->lineEdit_Username->text(),ui->lineEdit_Name->text(),ui->lineEdit_LicenseNumber->text());

            connect(m_start, &All_Start::closed, this, [this]() {
                this->show();
                disconnect(this, SIGNAL(Senduser(QString,QString,QString)), m_start, SLOT(onSetUser(QString,QString,QString)));
                m_start->deleteLater();
                m_start = nullptr;



            });
            m_start->show();
            this->hide();
            ui->lineEdit_Password->clear();

        } else {
            QMessageBox::warning(nullptr, "登陆失败", "无效用户名或密码", tr("确认"));
        }
    } else {
        // 如果文件不存在或无法打开
        QMessageBox::warning(nullptr, "登陆失败", "无此账户", tr("确认"));
    }
}


void SignIn::on_pushButton_AddAdmin_clicked()
{
    if (m_add.isNull() || !m_add->isVisible()) {  // 窗口为空或已经被关闭（不可见）
        m_add = new Adduser();
        connect(m_add, &Adduser::destroyed, this, [this]() {
            m_add.clear();  // 当 Adduser 窗口被销毁时，清空指针
        });
        m_add->show();  // 显示窗口
    } else {
        // 如果窗口已经打开并可见，你可以选择激活窗口
        m_add->raise();
        m_add->activateWindow();
    }
}
