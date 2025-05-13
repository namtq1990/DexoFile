#ifndef BASELISTWIDGET_H
#define BASELISTWIDGET_H

#include <QListWidget>
#include <vector>   // For std::vector
#include <utility>  // For std::pair

class BaseListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit BaseListWidget(QWidget *parent = nullptr);

    // Custom roles for item data
    enum ItemDataRole {
        ItemTypeRole = Qt::UserRole + 1, // To store whether item is Header or Selectable
        UserDataRole = Qt::UserRole + 2  // For arbitrary user data on selectable items
    };

    // Enum to distinguish item types
    enum ListItemType {
        SelectableItem,
        HeaderItem
    };

    // Public API for navigation and activation
    void selectNextSelectableItem();
    void selectPreviousSelectableItem();
    QListWidgetItem* activateCurrentItem(); // Returns the item if activation is valid

    // Methods to populate the list
    void populateItems(const std::vector<std::pair<QString, ListItemType>>& itemsData);
    void addItemWithType(const QString& text, ListItemType itemType, const QVariant& userData = QVariant());
    void addHeaderItem(const QString& text);
    void addSelectableItem(const QString& text, const QVariant& userData = QVariant());

    // Override to handle selection changes and apply custom styling if needed
    // void currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) override;

private:
    QListWidgetItem* findSelectableItem(int startIndex, int direction, bool wrapAround) const;
    bool isItemSelectable(const QListWidgetItem* item) const;
    
    // Helper to get the first or last selectable item
    QListWidgetItem* getFirstSelectableItem() const;
    QListWidgetItem* getLastSelectableItem() const;
};

#endif // BASELISTWIDGET_H
