#ifndef TIMESCALESELECTOR_H
#define TIMESCALESELECTOR_H

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsObject>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QDateTime>
#include <QColor>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QCursor>
#include <QDebug>

class ResizableRectItem : public QGraphicsObject
{
    Q_OBJECT

public:
    ResizableRectItem(QGraphicsItem *parent = nullptr)
        : QGraphicsObject(parent), resizing(false)
    {
        setFlags(ItemIsMovable | ItemSendsGeometryChanges);
        setCursor(Qt::SizeHorCursor);
        setRect(QRect(0, 0, 200, 30));
    }

    QRectF boundingRect() const override {
        return rect();
    }

    void setRect(const QRectF &newRect) {
        prepareGeometryChange();
        m_rect = newRect;
    }

    QRectF rect() const {
        return m_rect;
    }

signals:
    void rangeChanged(const QDateTime &start, const QDateTime &end);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override {
        painter->setBrush(QColor(0, 0, 255, 100));
        painter->drawRect(m_rect);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (isOnEdge(event->pos())) {
            resizing = true;
            setCursor(Qt::SizeHorCursor);
        } else {
            QGraphicsObject::mousePressEvent(event);
        }
    }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (resizing) {
            QRectF newRect = rect();
            newRect.setRight(event->pos().x());
            if (parentItem() && newRect.right() > parentItem()->boundingRect().right()) {
                newRect.setRight(parentItem()->boundingRect().right());
            }
            setRect(newRect);
        } else {
            QPointF newPos = event->pos() - event->lastPos();
            moveBy(newPos.x(), 0); // Ограничиваем движение только по горизонтали
            if (parentItem()) {
                if (pos().x() < 0) {
                    setPos(0, pos().y());
                }
                if (pos().x() + rect().width() > parentItem()->boundingRect().width()) {
                    setPos(parentItem()->boundingRect().width() - rect().width(), pos().y());
                }
            }
        }
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        resizing = false;
        setCursor(Qt::SizeHorCursor);
        emitRangeChanged();
        centerInParent();
        QGraphicsObject::mouseReleaseEvent(event);
    }

private:
    bool isOnEdge(const QPointF &pos) const
    {
        return qAbs(pos.x() - rect().right()) < 5;
    }

    void emitRangeChanged()
    {
        if (!parentItem()) return;

        double scaleFactor = parentItem()->boundingRect().width() / 800.0; // Предполагаем, что 800 - это полная ширина времени
        double startRatio = pos().x() / parentItem()->boundingRect().width();
        double endRatio = (pos().x() + rect().width()) / parentItem()->boundingRect().width();

        QDateTime currentTime = QDateTime::currentDateTime();
        QDateTime start = currentTime.addSecs(-3600 * startRatio); // Пример: 1 час назад
        QDateTime end = currentTime.addSecs(-3600 * endRatio);

        emit rangeChanged(start, end);
    }

    void centerInParent()
    {
        if (parentItem()) {
            setPos((parentItem()->boundingRect().width() - rect().width()) / 2, 0);
        }
    }

    QRectF m_rect;
    bool resizing;
};

class TimeScaleSelector : public QWidget
{
    Q_OBJECT

public:
    explicit TimeScaleSelector(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        // Создаем элементы интерфейса
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QHBoxLayout *scaleSelectorLayout = new QHBoxLayout();QPushButton *button1h = new QPushButton("1h", this);
        QPushButton *button1d = new QPushButton("1d", this);
        QPushButton *button1w = new QPushButton("1w", this);
        QPushButton *button1M = new QPushButton("1M", this);
        yearComboBox = new QComboBox(this);
        yearComboBox->setEditable(true);

        // Добавляем кнопки в горизонтальный макет
        scaleSelectorLayout->addWidget(button1h);
        scaleSelectorLayout->addWidget(button1d);
        scaleSelectorLayout->addWidget(button1w);
        scaleSelectorLayout->addWidget(button1M);
        scaleSelectorLayout->addWidget(yearComboBox);

        // Добавляем горизонтальный макет в основной вертикальный макет
        mainLayout->addLayout(scaleSelectorLayout);

        // Создаем QGraphicsView для отображения временной шкалы
        timeScaleView = new QGraphicsView(this);
        timeScaleView->setMinimumHeight(50);
        timeScaleView->setMaximumHeight(50);
        timeScaleView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mainLayout->addWidget(timeScaleView);

        // Инициализация выпадающего списка с годами
        int currentYear = QDate::currentDate().year();
        for (int i = 0; i < 10; ++i) {
            yearComboBox->addItem(QString::number(currentYear - i));
        }
        yearComboBox->setCurrentText(QString::number(currentYear));

        // Создание сцены для QGraphicsView
        scene = new QGraphicsScene(this);
        timeScaleView->setScene(scene);

        // Добавление основной полосы времени
        updateTimeBar();

        // Добавление полупрозрачной полосы для выбора времени
        selectionBar = new ResizableRectItem();
        selectionBar->setRect(QRect(300, 10, 200, 30));
        scene->addItem(selectionBar);

        // Подключение сигналов
        connect(selectionBar, &ResizableRectItem::rangeChanged, this, &TimeScaleSelector::onRangeChanged);
        connect(button1h, &QPushButton::clicked, this, &TimeScaleSelector::setScale1h);
        connect(button1d, &QPushButton::clicked, this, &TimeScaleSelector::setScale1d);
        connect(button1w, &QPushButton::clicked, this, &TimeScaleSelector::setScale1w);
        connect(button1M, &QPushButton::clicked, this, &TimeScaleSelector::setScale1M);

        // Установка начального масштаба
        setScale1h();
    }

private slots:
    void setScale1h() {
        updateScale(QDateTime::currentDateTime(), 3600, 1800); // 1 час, метки каждые 30 минут
    }

    void setScale1d() {
        updateScale(QDateTime::currentDateTime(), 86400, 43200); // 1 день, метки каждые 12 часов
    }

    void setScale1w() {
        updateScale(QDateTime::currentDateTime(), 604800, 86400); // 1 неделя, метки каждый день
    }

    void setScale1M() {
        updateScale(QDateTime::currentDateTime(), 2592000, 604800); // 1 месяц, метки каждую неделю
    }

    void onRangeChanged(const QDateTime &start, const QDateTime &end) {
        // Обработка изменения диапазона
        qDebug() << "Range changed:" << start.toString() << "to" << end.toString();
    }

private:
    void updateTimeBar() {
        timeBar = scene->addRect(0, 10, timeScaleView->width(), 30, QPen(Qt::black), QBrush(Qt::lightGray));
    }

    void updateScale(const QDateTime &center, int totalSeconds, int labelInterval) {
        scene->clear();
        timeBar = scene->addRect(0, 10, timeScaleView->width(), 30, QPen(Qt::black), QBrush(Qt::lightGray));

        selectionBar = new ResizableRectItem();
        selectionBar->setRect(QRect(300, 10, 200, 30));
        scene->addItem(selectionBar);

        QDateTime start = center.addSecs(-totalSeconds / 2);
        QDateTime end = center.addSecs(totalSeconds / 2);

        for (QDateTime dt = start; dt <= end; dt = dt.addSecs(labelInterval)) {
            QGraphicsTextItem *label = scene->addText(dt.toString("hh:mm dd.MM.yyyy"));
            label->setPos((dt.toSecsSinceEpoch() - start.toSecsSinceEpoch()) * timeScaleView->width() / totalSeconds, 10);
        }
        selectionBar->setRect(QRect((timeScaleView->width() - selectionBar->rect().width()) / 2, 10, selectionBar->rect().width(), 30)); // Центрирование полупрозрачной полосы
    }

    QComboBox *yearComboBox;
    QGraphicsView *timeScaleView;
    QGraphicsScene *scene;
    QGraphicsRectItem *timeBar;
    ResizableRectItem *selectionBar;
};


#endif // TIMESCALESELECTOR_H
