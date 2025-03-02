#ifndef ROUTABLEWIDGET_H
#define ROUTABLEWIDGET_H

#include <page.h>
#include <QWidget>


class RoutableWidget : public QWidget
{
public:
    RoutableWidget(QWidget* parent = nullptr) : QWidget(parent) {}

    ~RoutableWidget() {}

    virtual void onPageHide() {

    };

    virtual void onPageShow(Page page) {

    };
};

#endif // ROUTABLEWIDGET_H
