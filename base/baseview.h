/*
 * BaseView.h
 *
 *  Created on: Oct 18, 2021
 *      Author: quangnam
 */

#ifndef SRC_BASE_BASEVIEW_H_
#define SRC_BASE_BASEVIEW_H_

#include "navigationcomponent.h"
#include <QObject>
#include <functional>
#include <memory>

namespace navigation {
struct NavigationArgs;
struct NavigationEntry;
class NavigationComponent;
}  // namespace navigation

class BaseWindow;
class QEvent;
class QObject;

struct ViewAction {
    QString                   name;
    std::function<bool(void)> action;
    const char*               icon;

    ViewAction(
        const QString&            name = QString(),
        std::function<bool(void)> action =
            []() {
                return true;
            },
        const char* icon = nullptr)
        : name(name), action(action), icon(icon) {}
};

/**
 * Base Class for view component, example: Dialog, Window, Screen, ...
 * This is first on class hierarchy. If you use simple component, extend from its.
 * If your view is in MVP architecture, extend the BaseScreen class.
 * All of its is extend from this one.
 */
class BaseView
{
    friend class navigation::NavigationComponent;

   private:
    std::shared_ptr<ViewAction> mLeftAction;
    std::shared_ptr<ViewAction> mCenterAction;
    std::shared_ptr<ViewAction> mRightAction;
    std::shared_ptr<ViewAction> mLeftLongAction;
    std::shared_ptr<ViewAction> mCenterLongAction;
    std::shared_ptr<ViewAction> mRightLongAction;

    navigation::NavigationComponent* mNavigation;
    navigation::NavigationComponent* mChildNavigation = NULL;
    int                              mState           = 0;
    QString                          mTag;

    bool mShowLoading = false;

    virtual void showWindow(const navigation::NavigationEntry& entry);
    virtual void showDialog(const navigation::NavigationEntry& entry);
    virtual void showChildWidget(const navigation::NavigationEntry& entry);
    virtual void showTabWidget(const navigation::NavigationEntry& entry);
    virtual void popWindow(const navigation::NavigationEntry& entry);
    virtual void popDialog(const navigation::NavigationEntry& entry);
    virtual void popChildWidget(const navigation::NavigationEntry& entry);
    virtual void popTabWidget(const navigation::NavigationEntry& entry);

   protected:
    virtual void changeEvent(QEvent* ev);
    inline void  setFlag(const int& flag) { mState |= flag; }
    inline void  removeFlag(const int& flag) { mState &= ~flag; }
    inline bool  isFlagSet(const int& flag) { return mState & flag; }
    void         performCreate(navigation::NavigationEntry* entry);
    void         performShow(navigation::NavigationEntry* entry);
    void         performHide(navigation::NavigationEntry* entry);
    void         performDestroy(navigation::NavigationEntry* entry);

    virtual void performShowLoading();
    virtual void performHideLoading();

   public:
    BaseView(const QString& tag);
    virtual ~BaseView();
    const QString&   getTag() const { return mTag; }
    virtual void     onCreate(navigation::NavigationArgs* entry);
    virtual void     onEnter();
    virtual void     onExit();
    virtual void     onDestroy();
    virtual void     reloadLocal() = 0;
    BaseWindow*      topWindow();
    virtual QObject* content() { return nullptr; }

    void                             setNavigation(navigation::NavigationComponent* nav) { mNavigation = nav; }
    navigation::NavigationComponent* getNavigation() { return mNavigation; }
    void setChildNavigation(navigation::NavigationComponent* nav) { mChildNavigation = nav; }
    navigation::NavigationComponent*     getChildNavigation() { return mChildNavigation; }
    virtual navigation::NavigationEntry* showError(const char* msg);
    virtual navigation::NavigationEntry* showInfo(const char* msg);
    void                                 showShortMessage(const char* msg, bool delayed = false);
    void                                 showLoading();
    void                                 hideLoading();
    bool                                 isShowLoading();

    void setLeftAction(std::shared_ptr<ViewAction> action);
    void setCenterAction(std::shared_ptr<ViewAction> action);
    void setRightAction(std::shared_ptr<ViewAction> action);
    void setLongLeftAction(std::shared_ptr<ViewAction> action);
    void setLongCenterAction(std::shared_ptr<ViewAction> action);
    void setLongRightAction(std::shared_ptr<ViewAction> action);

    void setLeftAction(ViewAction* action);
    void setCenterAction(ViewAction* action);
    void setRightAction(ViewAction* action);
    void setLongLeftAction(ViewAction* action);
    void setLongCenterAction(ViewAction* action);
    void setLongRightAction(ViewAction* action);

    std::shared_ptr<ViewAction> getLeftAction();
    std::shared_ptr<ViewAction> getCenterAction();
    std::shared_ptr<ViewAction> getRightAction();
    std::shared_ptr<ViewAction> getLongLeftAction();
    std::shared_ptr<ViewAction> getLongCenterAction();
    std::shared_ptr<ViewAction> getLongRightAction();

    std::string translate(const char* text);
    QString     translateQ(const char* text);
    std::string translate(std::string&& text);

    virtual void updateMenu();

    QString tag() const { return mTag; }
    void    setTag(const QString& tag) { mTag = tag; }
};

#endif /* SRC_BASE_BASEVIEW_H_ */
