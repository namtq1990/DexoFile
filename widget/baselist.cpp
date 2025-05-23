#include "baselist.h"
#include "basemodel.h"

#include <functional>
#include <QHeaderView>
#include <QScrollBar>

using namespace std;

inline bool isItemSelectableInList(QAbstractItemModel* model, const int row) {
    auto qIndex = model->index(row, 0);
    return model->flags(qIndex) & Qt::ItemIsSelectable;
}

BaseList::BaseList(QWidget *parent) : QTableView(parent) {
    setSelectionMode(SelectionMode::SingleSelection);
    setSelectionBehavior(SelectionBehavior::SelectRows);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPalette p;
    p.setColor(QPalette::ColorRole::Highlight, QColor(0x0097e0));
    p.setColor(QPalette::Window, Qt::transparent);
    p.setColor(QPalette::Base, Qt::transparent);
    setPalette(p);

    // Connect the scroll bar's valueChanged signal
    // connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &BaseList::updateHeaderVisibility);

    verticalHeader()->setDefaultSectionSize(25);

//    setStyleSheet("QTableView { background: transparent; }");

    setAutoFillBackground(false);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
}

void BaseList::updateHeaderVisibility(int value)
{
    if (value == 0) {
        // At the top of the list, show the header
        horizontalHeader()->show();
    } else {
        // Scrolled down, hide the header
        horizontalHeader()->hide();
    }
}

void BaseList::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);

    connect(model, &QAbstractItemModel::modelReset, this, [this](auto) {
        if (mAutoSelect && mCurSelection == -1) {
            moveDown();
        } else {
            selectRow(mCurSelection);
        }
    });
}

void BaseList::setSelectionModel(QItemSelectionModel *model)
{
    QTableView::setSelectionModel(model);

    if (auto sModel = model) {
        connect(sModel, &QItemSelectionModel::selectionChanged, this, [this](auto) {
            auto index = this->getSelectedIndex();
            if (index >= 0) {
                mCurSelection = this->getSelectedIndex();
            }
        });
    }
}

void BaseList::setAutoSelect(bool isAuto)
{
    mAutoSelect = isAuto;
}


void BaseList::moveUp()
{
    auto rCount = model()->rowCount();
    if (rCount == 0)
        return;

    auto slModel = selectionModel();
    auto curSelect = slModel->selectedRows();
    auto slIndex = curSelect.empty() ? 0
                                     : (curSelect.first().row());
    auto preIndex = previousIndex(slIndex);
    if (preIndex >= 0 && preIndex < rCount) {
//        clearSelection();
        selectRow(preIndex);
    }
}

void BaseList::moveDown() {
    auto _model = model();
    auto rCount = _model->rowCount();
    if (rCount == 0)
        return;

    auto slModel = selectionModel();
    auto curSelect = slModel->selectedRows();
    auto slIndex = curSelect.empty() ? -1
                                     : (curSelect.first().row());

    auto next = nextIndex(slIndex);
    if (next >= 0 && next < rCount) {
//        clearSelection();
        selectRow(next);
    }
}

void BaseList::performClick() {
    if (mListener) {
        auto slIndex = getSelectedIndex();
        if (slIndex >= 0) {
            auto slModel = selectionModel();
            mListener(slModel->selectedIndexes().first());
//            slModel->clearSelection();
        }
    }
}

void BaseList::setClickListener(BaseList::OnClickListener&& listener) {
    mListener = listener;
}

int BaseList::getSelectedIndex()
{
    auto slModel = selectionModel();
    if (slModel->hasSelection()) {
        auto slIndex = slModel->selectedRows().first();
        return slIndex.row();
    }

    return -1;
}

int BaseList::nextIndex(const int &index) const
{
    auto _model = model();

    if (_model->rowCount() > 0) {
        auto next = index + 1;
        if (next >= _model->rowCount()) {
            next = 0;
        }

        // Find next item when loop from NEXT -> ROW COUNT
        for (; next < _model->rowCount(); next++) {
            if (isItemSelectableInList(_model, next)) {
                break;
            }
        }

        if (next >= _model->rowCount()) {
            // Not found any
            for (next = 0; next <= index; next++) {
                if (isItemSelectableInList(_model, next)) {
                    break;
                }
            }
        }

        return next;
    }

    return -1;
}

int BaseList::previousIndex(const int &index) const {
    auto _model = model();

    if (_model->rowCount() > 0) {
        auto next = index - 1;
        if (next < 0) {
            next = _model->rowCount() - 1;
        }

        for (; next >= 0; next--) {
            if (isItemSelectableInList(_model, next))
                break;
        }

        if (next < 0) {
            for (next = _model->rowCount() - 1; next >= index; next--) {
                if (isItemSelectableInList(_model, next))
                    break;
            }
        }

        return next;
    }

    return -1;
}
