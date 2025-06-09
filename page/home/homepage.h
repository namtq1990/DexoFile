#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include "base/basescreen.h"
#include "model/AccumulationDataTypes.h"

// Forward declaration for the UI class
namespace Ui {
class HomePage;
}

class SpectrumAccumulator;

// NavigationComponent is forward-declared in basescreen.h which is included above.
// No need for a redundant forward declaration here.

class HomePage : public BaseScreen
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr); // navComp parameter removed
    ~HomePage() override;

    void reloadLocal() override;

    void start();
    void stop();

    AccumulatorState getState();

    void setIntervalTime(int sec);
    void setMeasureTime(int sec);
    void setIsotope(const QString& src);
    void setPipeMaterial(const QString& material);
    void setPipeDiameter(double diameter);
    void setPipeThickness(double thickness);
    void setPipeDensity(double density);

public slots:
    void stateChanged(AccumulatorState);
    void updateEvent();

private:
    Ui::HomePage *ui;
    std::shared_ptr<SpectrumAccumulator> m_accumulator;
};

#endif // HOMEPAGE_H
