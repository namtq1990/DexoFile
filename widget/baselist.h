#ifndef BASELIST_H
#define BASELIST_H

#include <QTableView>
#include <memory>
#include <functional>

class BaseList : public QTableView
{
    Q_OBJECT
public:
    typedef std::function<void(const QModelIndex&)> OnClickListener;

    BaseList(QWidget* parent = NULL);

    void setModel(QAbstractItemModel *model) override;
    void setSelectionModel(QItemSelectionModel *selectionModel) override;

    void setAutoSelect(bool isAuto);

    void moveUp();
    void moveDown();
    void performClick();
    void setClickListener(OnClickListener&& listener);
    void updateHeaderVisibility(int value);
    /*
     *  Return current selection, if none, return -1
     * */
    int getSelectedIndex();

    // Return next index on the list of this item, if index reach end, restart to start position
    int nextIndex(const int& index) const;
    /*
     *  Return Pre index on the list of this index, if index reach start, return it to end of list
     *  */
    int previousIndex(const int& index) const;

private:
    OnClickListener mListener = nullptr;
    int mCurSelection = -1;
    bool mAutoSelect = false;

};

#endif // BASELIST_H
