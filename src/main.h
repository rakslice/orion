#ifndef MAIN_H
#define MAIN_H

#include <QObject>
#include <QScreen>
#include <QQmlContext>

class ScreenChangeHandler : public QObject {
    Q_OBJECT
public slots:
        void screenChanged(QScreen * screen);
public:
    ScreenChangeHandler(QQmlContext * rootContext) : _rootContext(rootContext) {

    }
private:
    QQmlContext * _rootContext;
};

#endif // MAIN_H
