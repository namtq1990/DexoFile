#ifndef BASESCREEN_H
#define BASESCREEN_H

#include <QWidget>
#include "baseview.h"

struct NavigationComponent;

class BaseScreen : public QWidget, public BaseView
{
    Q_OBJECT
public:
    explicit BaseScreen(const QString& tag,QWidget *parent = nullptr);
    virtual ~BaseScreen() = default;
    
};

#endif // BASESCREEN_H
