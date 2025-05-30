#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Spectrum.h"
//#include "NcPeak.h"
#include <QString>
#include <memory>

struct Background {
    int id = -1;
    std::shared_ptr<Spectrum> spc;
    QString date;

//    std::list<NcPeak> peaksInfo;
};

#endif // BACKGROUND_H
