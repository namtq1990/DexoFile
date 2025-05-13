#include "component.h"

Component::Component(const QString& tag)
    : m_tag(tag)
{
}

QString Component::tag() const
{
    return m_tag;
}

QDebug Component::logD() const
{
    QDebug stream = nucare::logD();
    stream.nospace().noquote() << "[" << m_tag << "]";
    return stream;
}

QDebug Component::logI() const
{
    QDebug stream = nucare::logI();
    stream.nospace().noquote() << "[" << m_tag << "]";
    return stream;
}

QDebug Component::logW() const
{
    QDebug stream = nucare::logW();
    stream.nospace().noquote() << "[" << m_tag << "]";
    return stream;
}

QDebug Component::logE() const
{
    QDebug stream = nucare::logE();
    stream.nospace().noquote() << "[" << m_tag << "]";
    return stream;
}
