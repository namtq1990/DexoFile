#ifndef NCBUTTON_H
#define NCBUTTON_H

#include <QPushButton>

/**
 * @brief The NcButton Special button for this application, support Long click on a button
 */
class NcButton : public QPushButton
{
    Q_OBJECT
private:
    qint64 mMousePressInTime;
    QTimer* mTimer;
public:
    enum State {
        NONE,
        PRESSED,
        CANCEL
    } state = NONE;

    explicit NcButton(QWidget *parent = nullptr);
    ~NcButton();

    bool eventFilter(QObject* obj, QEvent* event) override;

    void updateState(State state);

signals:
    void longClicked(long msec);
    void cancelled(long msec);
};

#endif // NCBUTTON_H
