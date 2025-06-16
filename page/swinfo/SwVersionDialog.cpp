#include "SwVersionDialog.h"
#include "ui_SwVersion.h"
#include "component/componentmanager.h"
#include "component/detectorcomponent.h"
#include "model/DetectorProp.h"

navigation::NavigationEntry* navigation::toSwVersion(BaseView* parent, const QString& tag) {
    auto widget = dynamic_cast<QWidget*>(parent);
    auto view = new SwVersionDialog(std::move(tag), widget);
    auto ret = new NavigationEntry(DIALOG, view, nullptr, widget);
    parent->getNavigation()->enter(ret);

    return ret;
}

SwVersionDialog::SwVersionDialog(const QString &&tag, QWidget *parent) : BaseDialog(tag, parent)
{
    setTitle("Device Information");
    ui = new Ui::SwVersionDialog();

    QWidget* content = new QWidget(this);
    ui->setupUi(content);
    setContentView(content);

    setMaximumWidth(parent->width());

    if (auto detector = ComponentManager::instance().detectorComponent()) {
        auto prop = detector->properties();
        m_info.device = prop->getSerial();
        m_info.detectorType = nucare::toString(prop->getDetectorCode()->cType);
    }

    m_info.version = QCoreApplication::applicationVersion();
    dataChanged();
}

SwVersionDialog::~SwVersionDialog()
{
    delete ui;
}

void SwVersionDialog::dataChanged() {
    ui->version->setText(m_info.version);
//    ui->user->setText(m_info.user);
//    ui->location->setText(info->location.c_str());
    ui->serial->setText(m_info.device);
    ui->detectorType->setText(m_info.detectorType);
}
