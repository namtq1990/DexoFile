#include "widget/baselist.h"
#include "component/componentmanager.h" // For ComponentManager
#include "thememanager.h"             // For ThemeManager
#include "util/util.h"
#include <QListWidgetItem>
#include <QVariant>
#include <QFont>
#include <QColor>

BaseListWidget::BaseListWidget(QWidget *parent) : QListWidget(parent)
{
    // Basic setup, e.g., selection mode
    setSelectionMode(QAbstractItemView::SingleSelection);
    // setFocusPolicy(Qt::StrongFocus); // Ensure it can receive focus for selection

    // Remove default background and border to make it blend with parent
    setStyleSheet("QListWidget { background-color: transparent; border: none; }"
                //   "QListWidget::item { background-color: transparent; }" // Ensure items are also transparent by default
                  // Selection color will be handled by the default delegate or a theme-aware one
                  // "QListWidget::item:selected { background-color: red; color: white; }" // Example selection
                 );
    //setAttribute(Qt::WA_StyledBackground, true); // Might be needed for some stylesheet effects
}

void BaseListWidget::addItemWithType(const QString& text, ListItemType itemType, const QVariant& userData)
{
    QListWidgetItem *item = new QListWidgetItem(text, this);
    item->setData(ItemTypeRole, static_cast<int>(itemType));
    if (itemType == SelectableItem) {
        item->setData(UserDataRole, userData);
        item->setFlags(item->flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    } else { // HeaderItem
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        
        ThemeManager* tm = ComponentManager::instance().themeManager();
        if (tm) {
            item->setFont(tm->listHeaderFont());
            item->setForeground(tm->listHeaderTextColor());
        } else {
            // Fallback or log warning if ThemeManager is not available
            // This case should ideally not happen if initialization order is correct.
            nucare::logW() << "BaseListWidget: ThemeManager not available for styling header item.";
            // Fallback styling for header if ThemeManager is missing
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setForeground(Qt::darkBlue); // Default fallback color
        }
    }
    addItem(item);
}

void BaseListWidget::addHeaderItem(const QString& text)
{
    addItemWithType(text, HeaderItem);
}

void BaseListWidget::addSelectableItem(const QString& text, const QVariant& userData)
{
    addItemWithType(text, SelectableItem, userData);
}

void BaseListWidget::populateItems(const std::vector<std::pair<QString, ListItemType>>& itemsData)
{
    clear(); // Clear existing items
    for (const auto& itemPair : itemsData) {
        addItemWithType(itemPair.first, itemPair.second);
    }
    // Select the first selectable item by default, if any
    QListWidgetItem* firstSelectable = getFirstSelectableItem();
    if (firstSelectable) {
        setCurrentItem(firstSelectable);
    }
}

bool BaseListWidget::isItemSelectable(const QListWidgetItem* item) const
{
    if (!item) return false;
    return static_cast<ListItemType>(item->data(ItemTypeRole).toInt()) == SelectableItem;
}

QListWidgetItem* BaseListWidget::findSelectableItem(int startIndex, int direction, bool wrapAround) const
{
    int numItems = count();
    if (numItems == 0) return nullptr;

    int currentIndex = startIndex;

    for (int i = 0; i < numItems; ++i) {
        currentIndex += direction;

        if (wrapAround) {
            if (currentIndex < 0) currentIndex = numItems - 1;
            else if (currentIndex >= numItems) currentIndex = 0;
        } else {
            if (currentIndex < 0 || currentIndex >= numItems) return nullptr;
        }
        
        QListWidgetItem* currentItemCandidate = item(currentIndex);
        if (currentItemCandidate && isItemSelectable(currentItemCandidate)) {
            return currentItemCandidate;
        }
        // If we started with a valid index and wrapped around back to it without finding anything
        // (only relevant if all items are headers, or numItems is 1 and it's a header)
        if (wrapAround && i > 0 && currentIndex == startIndex && !isItemSelectable(item(startIndex))) return nullptr;
    }
    return nullptr; // No selectable item found
}

QListWidgetItem* BaseListWidget::getFirstSelectableItem() const {
    for (int i = 0; i < count(); ++i) {
        if (isItemSelectable(item(i))) {
            return item(i);
        }
    }
    return nullptr;
}

QListWidgetItem* BaseListWidget::getLastSelectableItem() const {
    for (int i = count() - 1; i >= 0; --i) {
        if (isItemSelectable(item(i))) {
            return item(i);
        }
    }
    return nullptr;
}


void BaseListWidget::selectNextSelectableItem()
{
    int currentRow = -1;
    QListWidgetItem* currentSelItem = currentItem();
    if (currentSelItem) {
        currentRow = row(currentSelItem);
    } else { // No item currently selected, try to select the first one
        QListWidgetItem* first = getFirstSelectableItem();
        if (first) {
            setCurrentItem(first);
        }
        return; // If no selectable items, do nothing
    }
    
    QListWidgetItem* nextItem = findSelectableItem(currentRow, 1, true); // Wrap around
    if (nextItem) {
        setCurrentItem(nextItem);
    }
}

void BaseListWidget::selectPreviousSelectableItem()
{
    int currentRow = -1;
    QListWidgetItem* currentSelItem = currentItem();
    if (currentSelItem) {
        currentRow = row(currentSelItem);
    } else { // No item currently selected, try to select the last one
        QListWidgetItem* last = getLastSelectableItem();
        if (last) {
            setCurrentItem(last);
        }
        return; // If no selectable items, do nothing
    }

    QListWidgetItem* prevItem = findSelectableItem(currentRow, -1, true); // Wrap around
    if (prevItem) {
        setCurrentItem(prevItem);
    }
}

QListWidgetItem* BaseListWidget::activateCurrentItem()
{
    QListWidgetItem* currentSelItem = currentItem();
    if (currentSelItem && isItemSelectable(currentSelItem)) {
        // The screen using this list will handle the action based on this item.
        // For example, it might get UserDataRole from the item.
        return currentSelItem;
    }
    return nullptr;
}

// Example of how currentItemChanged might be used for styling (optional, can be done via stylesheet too)
// void BaseListWidget::currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
// {
//     QListWidget::currentItemChanged(current, previous);
//     // Apply custom styling for selected/deselected items if not using stylesheets
//     // This is where ThemeManager colors could be used.
//     // For example:
//     // if (previous) {
//     //    previous->setBackground(Qt::transparent); // Or default color from ThemeManager
//     // }
//     // if (current && isItemSelectable(current)) {
//     //    current->setBackground(Qt::red); // Or selection color from ThemeManager
//     // }
// }
