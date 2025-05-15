/*
 * Backstack.cpp
 *
 *  Created on: Oct 18, 2021
 *      Author: quangnam
 */

#include "navigationcomponent.h"
#include "componentmanager.h"
#include <QMetaObject> // Required for QMetaObject::invokeMethod

using namespace std;
using namespace navigation;

// Static list for global listeners
QList<QObject*> NavigationComponent::sgGlobalNavigationListeners;

navigation::NavigationComponent::NavigationComponent(NavigationComponent* parent)
    : Component("NavigationComp"), mParent(parent), mBackstacks() {}

navigation::NavigationComponent::~NavigationComponent() {
    for (auto entry : mBackstacks) {
        delete entry;
    }
}

bool navigation::NavigationComponent::hasNavigation() {
    return true;
}

navigation::NavigationComponent* navigation::NavigationComponent::getTopNavigation() {
    return ComponentManager::instance().navigationComponent();
}

void navigation::NavigationComponent::processCreate(NavigationEntry* entry) {
    bool topStackChanged = true;
    auto oldEntry = curEntry();
    auto mockEntry = dynamic_cast<const NavigationWithMock*>(entry);
    if (mockEntry) {
        auto targetEntry = findViewWithTag(mockEntry->targetTag);
        bool before = entry->isFlagSet(FLAG_BEFORE_MOCK);
        if (before && targetEntry != mBackstacks.end()) {
            // TODO Fix it for newIndex, it need to be update
            mBackstacks.insert(--targetEntry, entry);
        } else {
            mBackstacks.push_back(entry);
        }

        // TODO Support another flag
    } else {
        mBackstacks.push_back(entry);
    }

    // View perform onCreate and show this view
    entry->view->setNavigation(this);
    entry->view->setChildNavigation(entry->childNav);
    entry->view->setFlag(FLAG_ADDED);
    entry->view->performCreate(entry);

    QMetaObject::invokeMethod(this, [=]() {
        processUpdate(entry, oldEntry);
    }, Qt::QueuedConnection);
}

void navigation::NavigationComponent::processUpdate(NavigationEntry* entry, NavigationEntry* oldEntry) {
    auto oldV = oldEntry ? oldEntry->view : nullptr;

    if (oldV && oldV->isFlagSet(FLAG_HAS_SHOWN)) {
        oldEntry->view->performHide(oldEntry);
    }

    if (entry->view == curScreen()) {
        entry->view->performShow(entry);

        // Emit viewPushed signal
        emit viewPushed(entry->view);

        // Notify global listeners
        for (QObject* listener : sgGlobalNavigationListeners) {
            QMetaObject::invokeMethod(listener, "onGlobalViewPushed",
                                      Q_ARG(navigation::NavigationComponent*, this),
                                      Q_ARG(BaseView*, entry->view));
        }

        // TODO Update child Navigation
        if (auto childNav = entry->view->getChildNavigation()) {
                QMetaObject::invokeMethod(childNav, [childNav]() {
                    if (auto curEntry = childNav->curEntry(false)) {
                        childNav->processUpdate(curEntry, nullptr);
                    }
                });
        }
    }
}

bool navigation::NavigationComponent::pop(const BaseView *v, const bool& isRecursive) {
    if (v == curScreen() || v == nullptr) {
        // Pop latest stack
        if (getBackstack().empty())
            return false;

        auto oldEntry = curEntry(false);
        if (oldEntry != nullptr && oldEntry->hasChildNavigation() && isRecursive) {
            auto isHandled = oldEntry->childNav->pop(v, true);
            if (isHandled) {
                // If child Navigation is not empty, meaning that child Navigation has handle this action, so just ignore it
                return isHandled;
            }
        }

        if (oldEntry != nullptr) {
            if (oldEntry->view->isFlagSet(FLAG_HAS_SHOWN)) {
                oldEntry->view->performHide(oldEntry);
                oldEntry->view->performDestroy(oldEntry);
            }
            oldEntry->view->removeFlag(FLAG_ADDED);
            mBackstacks.pop_back();

            // Emit viewPopped signal
            emit viewPopped(oldEntry->view);

            // Notify global listeners
            for (QObject* listener : sgGlobalNavigationListeners) {
                QMetaObject::invokeMethod(listener, "onGlobalViewPopped",
                                          Q_ARG(navigation::NavigationComponent*, this),
                                          Q_ARG(BaseView*, oldEntry->view));
            }
        }

        auto newEntry = curEntry();
        if (newEntry == nullptr && oldEntry != nullptr && mParent != nullptr) {
            // If this backstack is empty, it should not be kept in the parent stack. So remove it
            mParent->pop(v, false);
        } else if (newEntry) {
            QMetaObject::invokeMethod(this, [this, newEntry]() {
                processUpdate(newEntry, nullptr);
            }, Qt::QueuedConnection);
        }

        delete oldEntry;

        return true;
    } else {
        // Search for stack and pop it out
        for (auto i = mBackstacks.begin(); i != mBackstacks.end(); i++) {
            auto entry = *i.base();
            if (entry->hasChildNavigation() && isRecursive) {
                if (entry->childNav->pop(v))
                    return true;
            }
            if (entry != nullptr && entry->view == v) {
                entry->view->performDestroy(entry);
                entry->view->removeFlag(FLAG_ADDED);
                mBackstacks.erase(i);

                // Emit viewPopped signal for the removed view
                emit viewPopped(entry->view);

                // Notify global listeners
                for (QObject* listener : sgGlobalNavigationListeners) {
                    QMetaObject::invokeMethod(listener, "onGlobalViewPopped",
                                              Q_ARG(navigation::NavigationComponent*, this),
                                              Q_ARG(BaseView*, entry->view));
                }

                return true;
            }
        }

        return false;
    }
}

bool navigation::NavigationComponent::popUntil(const BaseView *v, const bool &isRecursive) {
    if (mBackstacks.empty()) return false;

    for (auto i = --mBackstacks.end(); i != mBackstacks.begin(); i--) {
        auto entry = *i.base();
        if (entry->view == v) {
            return true;
        }
        if (entry->hasChildNavigation() && isRecursive && entry->childNav->popUntil(v, true)) {
            return true;
        } else {
            pop(entry->view, false);
        }

    }

    return false;
}

void navigation::NavigationComponent::enter(NavigationEntry* entry) {
    //    sEventLoop->get_subscriber().on_next(ev);
    QMetaObject::invokeMethod(this, [this, entry]() {
        processCreate(entry);
    }, Qt::QueuedConnection);
}

//void NavigationComponent::enterBefore(NavigationType type, BaseView *v, QObject *host, const std::string* tag, int flags) {
//    auto frontEntry = findViewWithTag(*tag);
//    if (frontEntry != mBackstacks.begin() && frontEntry->view != mBackstacks.back().view) {
//        mBackstacks.insert(frontEntry, NavigationEntry(type, v, host, flags));
//    } else {
//        enter(NavigationEntry(type, v, host, flags));
//    }
//}

vector<NavigationEntry*>::iterator navigation::NavigationComponent::findViewWithTag(const QString& tag) {
    if (mBackstacks.empty()) return mBackstacks.end();
    for (auto i = mBackstacks.rbegin(); i != mBackstacks.rend(); i++) {
        auto entry = *i;
        if (entry->view->getTag() == tag) {
            return (i + 1).base();
        }
    }

    return mBackstacks.end();
}

void navigation::NavigationComponent::registerGlobalListener(QObject* listener) {
    if (!sgGlobalNavigationListeners.contains(listener)) {
        sgGlobalNavigationListeners.push_back(listener);
    }
}

void navigation::NavigationComponent::unregisterGlobalListener(QObject* listener) {
    sgGlobalNavigationListeners.removeOne(listener);
}

BaseView* navigation::NavigationComponent::curScreen(const bool& isRecursive) {
    if (!getBackstack().empty()) {
        auto entry = mBackstacks.back();
        if (isRecursive
                && entry->childNav != nullptr
                && !entry->childNav->mBackstacks.empty()) {     // This Navigation has child Nav
            return entry->childNav->curScreen(isRecursive);
        } else {
            return entry->view;
        }
    }
    return nullptr;
}

NavigationEntry* navigation::NavigationComponent::curEntry(const bool &isRecursive) {
    if (!getBackstack().empty()) {
        const auto entry = mBackstacks.back();
        if (isRecursive && entry->childNav != nullptr) {     // This Navigation has child Nav
            return entry->childNav->curEntry(isRecursive);
        } else {
            return entry;
        }
    }
    return nullptr;
}
