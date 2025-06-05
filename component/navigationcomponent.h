/*
 * Backstack.h
 *
 *  Created on: Oct 18, 2021
 *      Author: quangnam
 */

#ifndef SRC_BASE_NAVIGATION_H_
#define SRC_BASE_NAVIGATION_H_

#include <vector>
#include <QObject> // Include QObject for static connections
#include <QList>   // Include QList for static listener list
#include "config.h"
#include "base/baseview.h"
#include "component/component.h"
#include "model/Calibration.h"

class BaseView;
class SubSettingItem;
class ChoicesDialogArgs;
class SpectrumAccumulator;
class EventDetailArgs;

namespace navigation {

enum NavigationType {
    TAB_IN_WINDOW,
    CHILD_IN_WINDOW,
    WINDOW,
    DIALOG
};

constexpr int FLAG_REPLACE = 0b1;
constexpr uint32_t FLAG_DEFAULT = 0;
constexpr int FLAG_BEFORE_MOCK = 0b10;
constexpr int FLAG_REMOVE_VIEW = 0b100;
constexpr int FLAG_ADDED = 0b1000;
constexpr int FLAG_CREATED = FLAG_ADDED << 1;
constexpr int FLAG_HAS_SHOWN = FLAG_CREATED << 1;

class NavigationComponent;

struct NavigationArgs {
    virtual ~NavigationArgs() {}
};

struct NavigationEntry {
private:
    NavigationArgs* mArgs = nullptr;
public:
    NavigationType type;
    BaseView* view;
    NavigationComponent* childNav;
    QObject* host;
    u_int32_t flags;

    NavigationEntry(NavigationType type = TAB_IN_WINDOW,
                    BaseView* v = nullptr,
                    NavigationComponent* childNav = nullptr,
                    QObject* _host = nullptr,
                    uint32_t flags = FLAG_DEFAULT)
        : type(type), view(v), childNav(childNav), host(_host), flags(flags) {}

    virtual ~NavigationEntry() = default;

    inline NavigationEntry* setView(BaseView* v) {
        this->view = v;
        return this;
    }

    inline NavigationEntry* setType(NavigationType type) {
        this->type = type;
        return this;
    }

    inline NavigationEntry* setChildNav(NavigationComponent* childNav) {
        this->childNav = childNav;
        return this;
    }

    inline NavigationEntry* setHost(QObject* host) {
        this->host = host;
        return this;
    }

    inline bool hasChildNavigation() { return childNav != nullptr; }
    inline void setFlag(const int& flag) { flags |= flag; }
    inline void removeFlag(const int& flag) { flags &= ~flag; }
    inline bool isFlagSet(const int& flag) { return flags & flag; }
    template<typename T = NavigationArgs>
    void setArguments(T* args) {
        mArgs = args;
    }
    template<typename T = NavigationArgs>
    T* getArguments() {
        return mArgs;
    }
    template<typename T = NavigationArgs>
    inline void resetArguments() {
        auto arg = dynamic_cast<T*>(mArgs);
        if (arg) {
            delete arg;
        }

        mArgs = nullptr;
    }
};

struct NavigationWithMock : public NavigationEntry {
    QString targetTag;

    NavigationWithMock(const QString& targetTag) : NavigationEntry(), targetTag(targetTag) {
        flags |= FLAG_BEFORE_MOCK;
    }
};

class NavigationUpdateEvent {
public:
    NavigationComponent* navController;
    NavigationEntry* oldEntry;
    NavigationEntry* entry;
    int action;
};

class NavigationComponent : public QObject, public virtual Component {
    Q_OBJECT

signals:
    void viewPushed(BaseView* newView);
    void viewPopped(BaseView* oldView);

   private:
    NavigationComponent* mParent;
    std::vector<NavigationEntry*> mBackstacks;

    static QList<QObject*> sgGlobalNavigationListeners; // Static list for global listeners

protected:
    void processCreate(NavigationEntry* entry);
    void processUpdate(NavigationEntry* newEntry, NavigationEntry* oldEntry);

   public:
    NavigationComponent(NavigationComponent* parent = NULL);
    virtual ~NavigationComponent();
    virtual bool hasNavigation();
    virtual BaseView* curScreen(const bool& isRecursive = false);
    virtual NavigationEntry* curEntry(const bool& isRecursive = false);
    virtual void enter(NavigationEntry* entry);

    // Static methods for global listener registration
    static void registerGlobalListener(QObject* listener);
    static void unregisterGlobalListener(QObject* listener);

    /**
     * @brief findViewWithTag Find the view with existing tag in the backstack.
     * @param tag
     * @return The iterrator of this view, return Iterator::end() if not found
     */
    virtual std::vector<NavigationEntry*>::iterator findViewWithTag(const QString& tag);

    /**
     * @brief pop lastest view out of system stack
     * @return true if this navigation handled, otherwise return false
     */
    virtual bool pop(const BaseView* v = nullptr, const bool& isRecursive = true);
    virtual bool popUntil(const BaseView* v = nullptr, const bool& isRecursive = true);
    NavigationComponent* getTopNavigation();
    inline NavigationComponent* getParentNavigation() { return mParent; }
    inline std::vector<NavigationEntry*>& getBackstack() { return mBackstacks; }
    inline void setParent(NavigationComponent* parent) { mParent = parent; }
};

void exit(NavigationComponent* navController, const bool& isForceQuit);
void toMainWindow(NavigationComponent* navController, const QString& tag = tag::WINDOW_TAG);
void toHome(NavigationComponent* navController, NavigationEntry* entry, const QString& tag = tag::HOME_TAG);
void toSetting(NavigationComponent* navController, NavigationEntry* entry, SubSettingItem* args,
               const QString& tag = tag::SETTING_TAG);
void toBackground(NavigationComponent* navController, NavigationEntry* entry, const QString& tag = tag::SETTING_TAG);
void toCalibration(NavigationComponent* navController, NavigationEntry* entry,
                   Calibration::Mode mode,
                   bool updateStdPeak = false,
                   const QString& tag = tag::CALIBRATION_TAG);
void toSpectrumViewer(NavigationComponent* navController, NavigationEntry* entry,
                      std::shared_ptr<SpectrumAccumulator> accumulator,
                      const QString& tag = tag::SPECTRUMVIEW_TAG);
void toChoiceDlg(NavigationComponent* navController,
                 NavigationEntry* entry,
                 ChoicesDialogArgs* args,
                 const QString& tag = tag::CHOICE_TAG);

NavigationEntry* toChoiceDlg(BaseView* parent, ChoicesDialogArgs* args);
NavigationEntry* toBackground(BaseView* parent);
NavigationEntry* toCalibration(BaseView* parent,
                               Calibration::Mode mode,
                               bool updateStdPeak = false);
NavigationEntry* toCalibCountDlg(BaseView* parent, const QString& = tag::CHOICE_TAG);
NavigationEntry* toSpectrumViewer(BaseView* parent, std::shared_ptr<SpectrumAccumulator> accumulator);
NavigationEntry* toEventList(BaseView* parent, const QString& = tag::EVENTS_TAG);
NavigationEntry* toEventDetail(BaseView* parent, EventDetailArgs* args, const QString& = tag::EVENT_DETAIL_TAG);

void showWarning(NavigationComponent* navController,
                 NavigationEntry* entry,
                 const QString& text,
                 const QString& tag = tag::WARNING_DLG);

void showWarning(const QString&& text,
                 const QString&& tag = tag::WARNING_DLG);

NavigationEntry* showWarning(BaseView* parent, const QString&& text, const QString& tag = tag::WARNING_DLG);

void showSuccess(NavigationComponent* navController,
                 NavigationEntry* entry,
                 const QString& text,
                 const QString& tag = tag::SUCCESS_DLG);

void showSuccess(const QString&& text,
                 const QString&& tag = tag::SUCCESS_DLG);

NavigationEntry* showSuccess(BaseView* parent, const QString&& text, const QString& tag = tag::SUCCESS_DLG);

void showShortMessage(const QString&& text);

}

#endif /* SRC_BASE_NAVIGATION_H_ */
