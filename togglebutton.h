#ifndef TOGGLEBUTTON_H
#define TOGGLEBUTTON_H

#include <QWidget>

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
    void onSignal();
    void offSignal();

public:
    void clickOn();
    void clickOff();

private slots:
    void onClicked();
    void offClicked();

private:
    Ui::ToggleButton *ui;
};

#endif // TOGGLEBUTTON_H
