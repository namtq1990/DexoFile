/*
 * BaseWindow.h
 *
 *  Created on: Oct 18, 2021
 *      Author: quangnam
 */

#ifndef SRC_BASE_BASEWINDOW_H_
#define SRC_BASE_BASEWINDOW_H_

#include "baseview.h"
#include <QMainWindow>

class BaseWindow : public QMainWindow, public BaseView {
    Q_OBJECT
protected:
    virtual void changeEvent(QEvent* ev) override;
    virtual void performShowLoading() override;
    virtual void performHideLoading() override;
public:
    BaseWindow(const QString& tag = "", QWidget* parent = NULL);
    ~BaseWindow();

    void closeEvent(QCloseEvent* ev) override;
    virtual void updateMenu() override;
};

#endif /* SRC_BASE_BASEWINDOW_H_ */
