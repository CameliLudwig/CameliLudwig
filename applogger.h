#ifndef APPLOGGER_H
#define APPLOGGER_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class AppLogger {
public:
    static AppLogger& instance() {
        static AppLogger logger;
        return logger;
    }

    void log(const QString &msg) {
        QMutexLocker locker(&m_mutex);
        if (!m_file.isOpen()) return;
        QTextStream ts(&m_file);
        ts << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
           << " " << msg << "\n";
        ts.flush();
    }

    void init(const QString &path = "log.txt") {
        m_file.setFileName(path);
        m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    }

private:
    AppLogger() { init(); }
    ~AppLogger() { if (m_file.isOpen()) m_file.close(); }
    AppLogger(const AppLogger&) = delete;
    AppLogger& operator=(const AppLogger&) = delete;

    QFile m_file;
    QMutex m_mutex;
};

#define LOG(msg) AppLogger::instance().log(QString("%1 [%2:%3] %4").arg(__FUNCTION__).arg(__FILE__).arg(__LINE__).arg(msg))

#endif // APPLOGGER_H
