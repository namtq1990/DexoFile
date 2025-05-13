#ifndef NUCARE_APPLICATION_H
#define NUCARE_APPLICATION_H

#include <QApplication>

namespace nucare {

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv); // Public constructor
    ~Application();

    static Application* instance();
    void initialize();

private:
    // No longer need m_instance here, QApplication handles its own singleton nature.
    Application(const Application&) = delete; // Keep copy constructor deleted
    Application& operator=(const Application&) = delete; // Keep assignment operator deleted
};

} // namespace nucare

#endif // NUCARE_APPLICATION_H
