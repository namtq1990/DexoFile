#ifndef INPUTCOMPONENT_H
#define INPUTCOMPONENT_H

#include <QObject> // For QObject base
#include "component/component.h" // For Component base
#include <QSocketNotifier>
#include <linux/input.h>

namespace nucare {

class InputComponent : public QObject, public virtual Component
{
    Q_OBJECT
public:
    explicit InputComponent(QObject *parent = nullptr, const QString& tag = "InputComponent");
    ~InputComponent();

    bool initialize();

signals:
    void btnLeftPress();
    void btnLeftRelease();
    void btnCenterPress();
    void btnCenterRelease();
    void btnRightPress();
    void btnRightRelease();

private slots:
    void readInput();

private:
    int fd_ = -1;
    QSocketNotifier *notifier_ = nullptr;
};

} // namespace nucare

#endif // INPUTCOMPONENT_H
