/*
 * SpectrumView.h
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#ifndef COMMON_WIDGET_SPECTRUMVIEW_H_
#define COMMON_WIDGET_SPECTRUMVIEW_H_

#include <QWidget>
#include "model/Spectrum.h"
#include "model/Types.h"

namespace SpcView {
constexpr int DEFAULT_ROWS = 3;
constexpr int DEFAULT_COLS = 5;
}

struct SpcViewData {
    std::shared_ptr<Spectrum> spc;
    Coeffcients* coef = nullptr;
    int maxViX = 0;     // Max value in X-Axis
    int maxViY = 0;     // Max Value in Y-Axis
    int maxX = 0;
    int maxY = 0;
    int rows = SpcView::DEFAULT_ROWS;
    int cols = SpcView::DEFAULT_COLS;
    int chsize;
public:
    ~SpcViewData() {
    }
};


class SpectrumView : public QWidget {
//    Q_OBJECT
private:
    SpcViewData mData;
    QBrush mFillBrush;
    QFontMetrics mFontMetrics;

    int mLineLength = 10;
public:
    SpectrumView(QWidget* parent = NULL);
    ~SpectrumView();
    void paintEvent(QPaintEvent* ev) override;
    void drawAxisX(QPainter& painter, const QRect& rec, const QRect& unitRec);
    void drawAxisY(QPainter& painter, const QRect& rec, const QRect& unitRec);
    void drawGrid(QPainter& painter, const QRect& rec, const QRect& unitRec);
    void drawChart(QPainter& painter, const QRect& rec, const QSizeF& unitRec);
    void drawROIs(QPainter &painter, const QRect &chartArea, const QSizeF &unitSize);
    void setFont(const QFont& font);
    void setData(std::shared_ptr<Spectrum> data);
    void setCoefficient(Coeffcients* coef) { mData.coef = coef; }
    void setGridRows(const int rows);
    void setGridCols(const int cols);
    double tranformXValue(const double& rawValue);
    double tranformYValue(const double& rawValue);
    double toXValue(const double& chartValue);
    double toYValue(const double& chartValue);
    QString toXAxisText(const int index, const double& value);
    QString toYAxisText(const double& value);
    bool shouldShowYLabel(const int& index, const double& value);
    bool shouldShowXLabel(const int& index, const double& value);


};

#endif /* COMMON_WIDGET_SPECTRUMVIEW_H_ */
