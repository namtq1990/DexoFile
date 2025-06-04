#include "AcqTimeDialog.h"
#include "Nucare/ThreadMgr.h"
#include "Domain/UseCase/SpectrumMeterUC.h"

using namespace std;
using namespace app::uc::meter;

AcqTimePresenter::AcqTimePresenter(shared_ptr<SpectrumMeterUsecase> spcRepository, std::string tag)
    : BasePresenter(tag),
      mSpcRepository(spcRepository) {
    mUpdatingTime = spcRepository->getCurData()->goalTime;
    subscription()->add(mSpcRepository->getDataStream()
                        .subscribe());
}

void AcqTimePresenter::bind(BaseScreen *v) {
    BasePresenter::bind(v);
    viewSubscription()->add(
                mSpcRepository->getDataStream()
                .observe_on(sdi::getComponent<nucare::ThreadManager>()->main())
                .subscribe([&](shared_ptr<model::Data> data) {
                    getView<AcqTimeDialog>()->bindData(*data, mUpdatingTime);
                }));
}

void AcqTimePresenter::increaseTime() {
    if (auto data = mSpcRepository->getCurData()) {
        mUpdatingTime += data->jumpAmount;
        getView<AcqTimeDialog>()->bindData(*data, mUpdatingTime);
    }
}

void AcqTimePresenter::decreaseTime() {
    if (auto data = mSpcRepository->getCurData()) {
        if (mUpdatingTime > data->jumpAmount + data->acqTime)
            mUpdatingTime -= data->jumpAmount;
        getView<AcqTimeDialog>()->bindData(*data, mUpdatingTime);
    }
}

void AcqTimePresenter::changeAcqTime() {
    if (auto data = mSpcRepository->getCurData()) {
        if (data->state != model::METERING
                || data->acqTime >= mUpdatingTime) {
            return;
        }
        data->goalTime = mUpdatingTime;
    }
}
