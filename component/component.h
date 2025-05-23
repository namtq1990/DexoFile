#ifndef COMPONENT_H
#define COMPONENT_H

#include <QString>
#include "util/util.h"

class Component
{
public:
    explicit Component(const QString& tag);
    virtual ~Component();

    QString tag() const;

    QDebug logD() const;
    QDebug logI() const;
    QDebug logW() const; // Added Warning level
    QDebug logE() const;

private:
    QString m_tag;
};

#endif // COMPONENT_H
