#ifndef BASESCREEN_H
#define BASESCREEN_H

#include <QWidget>
#include <QString>
#include <functional> // For std::function

struct NavigationComponent;

// Structure to define actions for menu buttons
struct ViewAction {
    QString name;
    std::function<bool(void)> action = []() { return true; }; // Default action
    const char* iconPath = nullptr; // Resource path for the icon

    // Default constructor
    ViewAction(QString n = "", std::function<bool(void)> act = nullptr, const char* icon = nullptr)
        : name(n), iconPath(icon) {
        if (act) {
            action = act;
        } else {
            // Ensure action is always callable, even if it does nothing or indicates no action.
            // The bool return could signify if the action was valid/handled.
            action = [](){ return false; }; // Default to doing nothing / action not set
        }
    }
     // Check if the action is valid (i.e., has a name or an icon, and a non-default action)
    bool isValid() const {
        // A valid action should at least have a name or an icon.
        // The default action lambda needs to be identifiable if we want to check against it.
        // For simplicity, we can assume if name or iconPath is set, it's intended to be valid.
        // Or, the action function itself can be checked if it's not the placeholder.
        return !name.isEmpty() || iconPath != nullptr;
    }
};

class BaseScreen : public QWidget
{
    Q_OBJECT
public:
    explicit BaseScreen(QWidget *parent = nullptr);
    virtual ~BaseScreen() = default;

    // View actions for the three menu buttons
    ViewAction leftViewAction;
    ViewAction centerViewAction;
    ViewAction rightViewAction;

    // View actions for long clicks
    ViewAction longClickLeftViewAction;
    ViewAction longClickCenterViewAction;
    ViewAction longClickRightViewAction;

    // Setters for view actions
    void setLeftViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath);
    void setCenterViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath);
    void setRightViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath);

    // Convenience overload to set an action directly
    void setLeftViewAction(ViewAction action);
    void setCenterViewAction(ViewAction action);
    void setRightViewAction(ViewAction action);

    // Setters for long click view actions
    void setLongClickLeftViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath);
    void setLongClickCenterViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath);
    void setLongClickRightViewAction(const QString& name, std::function<bool(void)> act, const char* iconPath);

    void setLongClickLeftViewAction(ViewAction action);
    void setLongClickCenterViewAction(ViewAction action);
    void setLongClickRightViewAction(ViewAction action);


    // Called by NavigationComponent when screen becomes active/inactive
    virtual void onScreenShown();
    virtual void onScreenHidden();

signals:
    void screenShown();
    void screenHidden();
    void viewActionsChanged(); // Emitted when any view action is changed

protected:
    // Helper to update a specific ViewAction and emit signal
    void updateViewAction(ViewAction& currentAction, const QString& name, std::function<bool(void)> act, const char* iconPath);
    void updateViewAction(ViewAction& currentAction, ViewAction newAction);


private:
};

#endif // BASESCREEN_H
