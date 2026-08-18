#ifndef SIGNIN_H
#define SIGNIN_H

#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFile>
#include "adduser.h"
#include "all_start.h"
namespace Ui {
class SignIn;
}

class SignIn : public QMainWindow
{
    Q_OBJECT

public:
    explicit SignIn(QWidget *parent = nullptr);
    ~SignIn();


signals:
    void Senduser(QString user,QString name,QString carnumber);

private slots:
    void on_pushButton_signin_clicked();
    void on_pushButton_AddAdmin_clicked();


protected:
    void closeEvent(QCloseEvent *event) override ;
private:
    Ui::SignIn *ui;
    All_Start *m_start = nullptr;
    QFile *m_styleFile = nullptr;
    QPointer<Adduser> m_add;
};

#endif // SIGNIN_H
