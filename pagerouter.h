#ifndef PAGEROUTER_H
#define PAGEROUTER_H

#include "mainwindow.h"
#include "routablewidget.cpp"

#include <QWidget>
#include <QMap>
#include <QObject>


class PageRouter : public QObject {
    Q_OBJECT

public:

    static PageRouter& instance();
    void initialize(MainWindow *mainWindow);

    void registerWidget(Page page, RoutableWidget *widget);
    void navigateTo(Page page);

private:
    explicit PageRouter(QObject *parent = nullptr);
    PageRouter(const PageRouter&) = delete;
    PageRouter& operator=(const PageRouter&) = delete;

    MainWindow *mainWindow;
    QMap<Page, RoutableWidget*> pageMap;
};

#endif // PAGEROUTER_H
