#include "pagerouter.h"

PageRouter::PageRouter(QObject *parent) : QObject(parent), mainWindow(nullptr) {}

PageRouter& PageRouter::instance() {
    static PageRouter instance;
    return instance;
}

void PageRouter::initialize(MainWindow *mainWnd) {
    mainWindow = mainWnd;
}

void PageRouter::registerWidget(Page page, RoutableWidget *widget) {
    if (!mainWindow) {
        qWarning("PageRouter not initialized with QMainWindow!");
        return;
    }
    pageMap[page] = widget;
    mainWindow->newWidget(widget);
}

void PageRouter::navigateTo(Page page) {
    if (!mainWindow) {
        qWarning("PageRouter not initialized with QMainWindow!");
        return;
    }

    auto it = pageMap.find(page);
    if (it != pageMap.end()) {

        RoutableWidget *currentWidget = ((RoutableWidget *)mainWindow->getCurrentWidget());
        if (currentWidget) {
            currentWidget->onPageHide();
        }

        it.value()->onPageShow(page);

        mainWindow->setWidget(it.value());
    } else {
        qWarning("Page not found in router!");
    }
}
