#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

namespace Ui {
class ToggleButton;
}

class ToggleButton : public QWidget
{
    Q_OBJECT

public:
    explicit ToggleButton(QWidget *parent = nullptr);
    ~ToggleButton();

signals:
    void startSignal();
    void stopSignal();
    void pauseSignal();

public slots:
    void onStartClicked();
    void onStopClicked();
    void onPauseClicked();

private:
    Ui::ToggleButton *ui;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *pauseButton;
};

#endif // TOGGLEBUTTON_H
