#include "EventDetailScreen.h"
#include "ui_EventDetailScreen.h"  // Generated UI header
#include "util/util.h"
#include "component/componentmanager.h"
#include "component/navigationcomponent.h"
#include "model/Event.h"

using namespace navigation;

NavigationEntry* navigation::toEventDetail(BaseView* parent, EventDetailArgs* args, const QString& tag) {
    auto widget = dynamic_cast<QWidget*>(parent);
    auto view = new EventDetailScreen(tag, widget);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW, view, nullptr, widget->parent());
    ret->setArguments(args);

    parent->getNavigation()->enter(ret);

    return ret;
}

EventDetailScreen::EventDetailScreen(const QString& tag, QWidget* parent)
    : BaseScreen(tag, parent), ui(new Ui::EventDetailScreen()) {
    ui->setupUi(this);
}

EventDetailScreen::~EventDetailScreen() { delete ui; }

void EventDetailScreen::onCreate(navigation::NavigationArgs* args) {
    BaseScreen::onCreate(args);
    if (auto edArgs = static_cast<EventDetailArgs*>(args)) {
        this->mRepository = edArgs->repository;
        this->mCurEvent = mRepository->getEventDetails(edArgs->eventID);

        bindEvent(mCurEvent);
    }
}

void EventDetailScreen::reloadLocal() { ui->retranslateUi(this); }

void EventDetailScreen::bindEvent(std::shared_ptr<Event> event)
{
    auto timeStart = event->getStartedTime();
    auto timeFinish = event->getFinishedTime();
    ui->idLabel->setText(QString("#%1").arg(event->getId()));
    ui->measureTimeValueLabel->setText(datetime::formatDuration(event->getAcqTime()));
    ui->startTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(timeStart));
    ui->stopTimeValueLabel->setText(datetime::formatDate_yyyyMMdd_HHmm(timeFinish));
    ui->materialLabel->setText(QString("Material: %1").arg(event->getPipeMaterial()));
    ui->diameterLabel->setText(QString("Diameter: %1").arg(event->getPipeDiameter(), 0, 'f', 1));
    ui->thicknessLabel->setText(QString("Thickness: %1").arg(event->getPipeThickness(), 0, 'f', 1));
    ui->clogRatioLabel->setText(QString("Clog ratio: %1%").arg(event->getClogRatio(), 0, 'f', 0));
    ui->clogThickEstValue->setText(QString("%1 mm").arg(event->getClogThickness(), 0, 'f', 1));
    ui->clogDensityValue->setText(QString("%1 g/cc").arg(event->getClogDensity(), 0, 'f', 1));
}
