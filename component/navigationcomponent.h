#ifndef NAVIGATIONCOMPONENT_H
#define NAVIGATIONCOMPONENT_H

#include <QObject>
#include <QStack>
#include <QPointer> // For QPointer to QStackedWidget to handle potential deletion
#include "component.h" // Include the Component base class

// Forward declarations
class QStackedWidget;
class BaseScreen; // From widget/basescreen.h

class NavigationComponent : public QObject, virtual public Component
{
    Q_OBJECT
public:
    explicit NavigationComponent(QStackedWidget *stackedWidget, QObject *parent = nullptr);

    void navigateTo(BaseScreen *screen);
    void navigateBack();
    BaseScreen* currentScreen() const;
    bool canNavigateBack() const;

    void navigateHomePage(QWidget* parent = nullptr);
    void navigateSettingPage(QWidget *parent = nullptr);

   signals:
    // Emitted when the current screen changes. oldScreen can be nullptr.
    void currentScreenChanged(BaseScreen *newScreen, BaseScreen *oldScreen);
    // Emitted when the ability to navigate back changes (stack size becomes 1 or >1)
    void canNavigateBackChanged(bool canGoBack);


private:
    QPointer<QStackedWidget> m_stackedWidget; // Use QPointer for safety
    QStack<BaseScreen*> m_navigationStack;

    void updateCanNavigateBackState(bool oldCanGoBack);
};

#endif // NAVIGATIONCOMPONENT_H
