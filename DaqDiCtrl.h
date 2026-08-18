#ifndef DAQDICTRL_H
#define DAQDICTRL_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVector>

// USB-5856 DI 封装: QProcess 跑 C# Bridge.exe → 它每秒写 di_data.txt → Qt 定时读

class DaqDiCtrl : public QObject
{
    Q_OBJECT
public:
    explicit DaqDiCtrl(QObject* parent = nullptr);
    ~DaqDiCtrl();

    bool open();             // 启动 Bridge.exe 后台进程
    void close();            // 杀死进程 + 删 txt
    bool isOpen() const { return m_process && m_process->state() == QProcess::Running; }

signals:
    void diDataReady(const QVector<int>& portValues);
    void errorOccurred(const QString& msg);

private slots:
    void onPollTimeout();
    void onProcessFinished(int);

private:
    QProcess* m_process = nullptr;
    QTimer* m_timer = nullptr;
    QString m_filePath;
    bool m_hasDevice = false;
};

#endif
