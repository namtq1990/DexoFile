#include "AcqTimeDialog.h"
#include "ui_AcqTimeDialog.h"
#include "component/SpectrumAccumulator.h"
#include "component/componentmanager.h"
#include "component/settingmanager.h"

navigation::NavigationEntry* navigation::toAcqTimeDlg(BaseView* parent, AcqTimeArgs* args, const QString& tag) {
    auto widget = dynamic_cast<QWidget*>(parent);
    auto view = new AcqTimeDialog(widget, tag);
    auto entry = new NavigationEntry(DIALOG, view, nullptr, widget);
    entry->setArguments(args);
    parent->getChildNavigation()->enter(entry);

    return entry;
}

AcqTimeDialog::AcqTimeDialog(QWidget *parent, const QString& tag) :
    BaseDialog(tag, parent),
    ui(new Ui::AcqTimeDialog),
    mUpdatingTime(0)
{
    QWidget* content = new QWidget();
    ui->setupUi(content);

    mSetting = ComponentManager::instance().settingManager();

    setTitle(tr("AcqTime_Title"));
    setContentView(content);

    connect(ui->btnLeft, &QPushButton::clicked, this, &AcqTimeDialog::decreaseTime);
    connect(ui->btnRight, &QPushButton::clicked, this, &AcqTimeDialog::increaseTime);
    connect(this, &QDialog::accepted, this, &AcqTimeDialog::updateTime);
    if (auto left = getLeftAction()) {
        left->name = "";
        left->action = [this]() {
            decreaseTime();
            return true;
        };
        left->icon = ":/icons/menu_acq_left";
    }
    if (auto right = getRightAction()) {
        right->name = "";
        right->action = [this]() {
            increaseTime();
            return true;
        };
        right->icon = ":/icons/menu_acq_right";
    }
}

AcqTimeDialog::~AcqTimeDialog()
{
    delete ui;
}

void AcqTimeDialog::onCreate(navigation::NavigationArgs *args)
{
    BaseDialog::onCreate(args);
    auto acqArgs = static_cast<AcqTimeArgs*>(args);
    mAccumulator = acqArgs->accumulator;
    mUpdatingTime = mAccumulator->getAcqTime();

    connect(mAccumulator, &SpectrumAccumulator::accumulationUpdated, this, &AcqTimeDialog::dataChanged);
    dataChanged();
}

void AcqTimeDialog::reloadLocal()
{
    ui->retranslateUi(this);
}

void AcqTimeDialog::increaseTime()
{
    mUpdatingTime += mSetting->getIncreaseTime();
    dataChanged();
}

void AcqTimeDialog::decreaseTime()
{
    auto expectAcqTime = mUpdatingTime - mSetting->getIncreaseTime();
    if (mAccumulator->getCurrentResult().count < expectAcqTime) {
        mUpdatingTime = expectAcqTime;
        dataChanged();
    }
}

void AcqTimeDialog::updateTime()
{
    if (mUpdatingTime > 0)
        mAccumulator->setTargetTime(mUpdatingTime);
}

void AcqTimeDialog::dataChanged() {
    ui->acqTime->setText(QString::number(mUpdatingTime));

    ui->curAcqTime->setText(QString("%3: %1/%2")
                            .arg((int) mAccumulator->getCurrentResult().count)
                            .arg(mUpdatingTime)
                            .arg("Time elapsed"));
    ui->incrAcqTime->setText(QString("Increment time: %1")
                             .arg(mSetting->getIncreaseTime()));
}
