#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <memory>
#include <QAbstractListModel>


template <typename T, typename Types = void>
class BaseModel : public QAbstractTableModel {
protected:
    std::shared_ptr<QVector<T>> mData;

public:
    BaseModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent),
          mData(std::make_shared<QVector<T>>())
    {
    }

    /**
     * @brief setData Set Data for native pointer. Model will manage itself data memory. (If model is cleared, data is cleared too)
     */
    virtual void setData(QVector<T>* data) {
        setData(std::shared_ptr<QVector<T>>(data));
    }

    /**
     * @brief setData Set Data Method for shared pointer, so this model doesn't manage data memory.
     */
    virtual void setData(std::shared_ptr<QVector<T>> data) {
        beginResetModel();
        mData = data;
        endResetModel();
    }

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return mData ? mData->size() : 0;
    }

    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return 1;
    }

    virtual Types getType(const QModelIndex&) const { return Types(); }

    QModelIndex sibling(int row, int column, const QModelIndex &idx) const override {
        if (!idx.isValid() || column >= columnCount() || row >= rowCount() || row < 0)
            return QModelIndex();
        return createIndex(row, column);
    }

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override {
        if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
            return false;

        beginRemoveRows(QModelIndex(), row, row + count - 1);
        const auto it = mData->begin() + row;
        mData->erase(it, it + count);
        endRemoveRows();
        return true;
    }

    QMap<int, QVariant> itemData(const QModelIndex &index) const override {
        if (index.row() < 0 || index.row() >= (int) mData->size())
            return QMap<int, QVariant>{};

        const QVariant displayData = data(index);
        return QMap<int, QVariant>{{
                std::make_pair<int>(Qt::DisplayRole, displayData),
                        std::make_pair<int>(Qt::EditRole, displayData)
            }};
    }

    T rawData(int row) const {
        return (*mData)[row];
    }

    T *rawDataPtr(int row) const { return mData->data() + row; }

    std::shared_ptr<QVector<T>> getData() { return mData; }
};

class SimpleModel : public BaseModel<QString> {
public:
    explicit SimpleModel(QObject* parent = NULL) : BaseModel<QString>(parent) {}

    virtual QVariant data(const QModelIndex &index, int role) const
    {
        if (role == Qt::DisplayRole) {
            return rawData(index.row());
        }

        return QVariant();
    }
};

#endif // BASEMODEL_H
