#ifndef CIRCLELINEEDIT_H
#define CIRCLELINEEDIT_H

#include <QLineEdit>
#include <QPainter>
#include <QRect>
#include <QLinearGradient>
#include <QGraphicsDropShadowEffect>

class CircleLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit CircleLineEdit(QWidget* parent = nullptr, bool enabled = false)
        : QLineEdit(parent), m_enabled(enabled) {
        setFixedSize(50, 50); // 设置固定大小
        setEnabled(m_enabled);
        // 设置阴影效果
        QGraphicsDropShadowEffect* shadowEffect = new QGraphicsDropShadowEffect(this);
        shadowEffect->setBlurRadius(8);
        shadowEffect->setOffset(2, 2);
        shadowEffect->setColor(Qt::gray);
        setGraphicsEffect(shadowEffect);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QImage buffer(size(), QImage::Format_ARGB32_Premultiplied);
        buffer.fill(Qt::transparent);

        QPainter painter(&buffer);
        painter.setRenderHint(QPainter::Antialiasing);

        // 绘制立体感圆形
        QRect rect(0, 0, width(), height());
        QLinearGradient gradient(rect.topLeft(), rect.bottomRight());
        gradient.setColorAt(0, QColor(255, 255, 255, 180)); // 高光颜色
        gradient.setColorAt(1, getColor().darker(150)); // 颜色加深产生立体感

        painter.setBrush(gradient);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(rect);

        QPainter thisPainter(this);
        thisPainter.drawImage(0, 0, buffer);
    }


private:
    QColor getColor() const {
        QString value = text();
        if (value == "0") return QColor(60, 60, 60);     // 暗灰=关闭
        if (value == "1" || value == "01") return QColor(0, 255, 0); // 绿色=通过/ON
        if (value == "10" || value == "2") return QColor(0, 0, 255); // 蓝色
        if (value == "11" || value == "3") return QColor(255, 255, 0); // 黄色
        return QColor(200, 200, 200); // 默认灰色
    }

    bool m_enabled;

};

#endif // CIRCLELINEEDIT_H
