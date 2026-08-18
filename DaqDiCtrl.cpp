#include "DaqDiCtrl.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QDebug>

DaqDiCtrl::DaqDiCtrl(QObject* parent) : QObject(parent) {
    m_filePath = QDir::currentPath() + "/di_data.txt";
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DaqDiCtrl::onPollTimeout);
}

DaqDiCtrl::~DaqDiCtrl() { close(); }

bool DaqDiCtrl::open() {
    if (m_process) return true;

    QFile::remove(m_filePath); // 清上次残余

    m_process = new QProcess(this);
    connect(m_process, QOverload<int>::of(&QProcess::finished),
            this, &DaqDiCtrl::onProcessFinished);

    m_process->start("Bridge.exe");
    if (!m_process->waitForStarted(3000)) {
        emit errorOccurred("Bridge.exe start failed: " + m_process->errorString());
        delete m_process;
        m_process = nullptr;
        return false;
    }

    qDebug() << "[DaqDiCtrl] bridge PID=" << m_process->processId();

    // 等 1.5 秒让 Bridge 写完第一行
    QTimer::singleShot(1500, this, [this]() {
        if (!QFile::exists(m_filePath)) {
            emit errorOccurred("Bridge.exe running but di_data.txt not created");
            return;
        }
        // 读一次确认设备正常
        QFile f(m_filePath);
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray d = f.readAll().trimmed();
            if (d.contains("err")) {
                QJsonObject obj = QJsonDocument::fromJson(d).object();
                emit errorOccurred("Device: " + obj["err"].toString());
                return;
            }
        }
        m_hasDevice = true;
        onPollTimeout(); // 立刻读第一次
        m_timer->start(1000); // 然后每秒读
    });

    return true;
}

void DaqDiCtrl::close() {
    m_timer->stop();
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(2000);
        delete m_process;
        m_process = nullptr;
    }
    QFile::remove(m_filePath);
    m_hasDevice = false;
}

void DaqDiCtrl::onPollTimeout() {
    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly)) return;
    QByteArray d = f.readAll().trimmed();
    f.close();
    if (d.isEmpty()) return;

    QJsonObject obj = QJsonDocument::fromJson(d).object();
    if (obj.contains("err")) {
        emit errorOccurred(obj["err"].toString());
        return;
    }
    if (obj.contains("ports")) {
        QJsonArray arr = obj["ports"].toArray();
        QVector<int> vals;
        for (auto v : arr) vals.append(v.toInt());
        emit diDataReady(vals);
    }
}

void DaqDiCtrl::onProcessFinished(int) {
    if (m_hasDevice)
        emit errorOccurred("Bridge.exe exited unexpectedly");
    m_process = nullptr;
    m_hasDevice = false;
}
