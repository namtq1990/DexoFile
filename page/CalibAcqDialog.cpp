#include "CalibAcqDialog.h"

navigation::NavigationEntry* navigation::toCalibCountDlg(BaseView* parent, const QString& tag) {
    auto widget = dynamic_cast<QWidget*>(parent);
    auto dlg = new CalibAcqDialog(tag, widget);
    auto entry = new navigation::NavigationEntry(DIALOG, dlg, nullptr, widget);
    navigation::toChoiceDlg(parent->getNavigation(), entry, nullptr);
    return entry;
}

CalibAcqDialog::CalibAcqDialog(QString tag, QWidget *parent) : ChoicesDialog(tag, parent)
{
    setTitle("Calibration Count\n(unit: n x 1000)");
//    auto choiceData = shared_ptr<QMap<QString, QVariant>>(new QMap<QString, QVariant>());
    Choices choiceData = {
        {"100", 100000},
        {"200", 200000},
        {"300", 300000},
        {"400", 400000},
        {"500", 500000}
    };

    setData(choiceData);
}
