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

template <typename SpectrumType>
struct SpcViewData_t {
    std::shared_ptr<SpectrumType> spc;
    Coeffcients* coef = nullptr;
    int maxViX = 0;     // Max value in X-Axis
    int maxViY = 0;     // Max Value in Y-Axis
    int maxX = 0;
    int maxY = 0;
    int rows = SpcView::DEFAULT_ROWS;
    int cols = SpcView::DEFAULT_COLS;
    int chsize;
public:
    ~SpcViewData_t() {
    }
};

template <typename SpectrumType>
class SpectrumView_t : public QWidget {
//    Q_OBJECT
private:
    SpcViewData_t<SpectrumType> mData;
    QBrush mFillBrush;
    QFontMetrics mFontMetrics;

    int mLineLength = 10;
public:
    SpectrumView_t(QWidget* parent = NULL);
    ~SpectrumView_t();
    void paintEvent(QPaintEvent* ev) override;
    void drawAxisX(QPainter& painter, const QRect& rec, const QRect& unitRec);
    void drawAxisY(QPainter& painter, const QRect& rec, const QRect& unitRec);
    void drawGrid(QPainter& painter, const QRect& rec, const QRect& unitRec);
    void drawChart(QPainter& painter, const QRect& rec, const QSizeF& unitRec);
    void drawROIs(QPainter &painter, const QRect &chartArea, const QSizeF &unitSize);
    void setFont(const QFont& font);
    void setData(std::shared_ptr<SpectrumType> data);
    void setCoefficient(Coeffcients* coef) { mData.coef = coef; }
    void setGridRows(const int rows);
    void setGridCols(const int cols);
    double tranformXValue(const double rawValue);
    double tranformYValue(const double rawValue);
    double toXValue(const double chartValue);
    double toYValue(const double chartValue);
    QString toXAxisText(const int index, const double& value);
    QString toYAxisText(const double value);
    bool shouldShowYLabel(const int index, const double value);
    bool shouldShowXLabel(const int index, const double value);


};

// Method definitions moved from SpectrumView.cpp

#include <QPainter> // Required for QPainter, QColor, etc.
#include <QPainterPath> // If used (not directly in methods, but good for context if it were)
#include "util/util.h" // For ui::textWidth, ui::PADDING_1
#include "util/NcLibrary.h" // For nucare::NcLibrary::energyToChannel etc.
#include "config.h" // For nucare::toExponentFormat (indirectly via util.h or NcLibrary.h perhaps)
#include <algorithm> // for std::max_element
#include <cmath> // for ceil, log10, pow

// Note: using namespace std; is in the .cpp file.
// If SpectrumView.cpp is deleted or fully emptied, and these methods use std:: extensively,
// it might be cleaner to qualify (e.g. std::shared_ptr) or add specific using declarations.
// For now, assuming they are qualified or will be. The make_shared below is changed to std::make_shared.

template <typename SpectrumType>
SpectrumView_t<SpectrumType>::SpectrumView_t(QWidget* parent) : QWidget(parent),
        mData(), mFillBrush(QColor(0x395589)), mFontMetrics(font())
{
    setData(std::make_shared<SpectrumType>());
    mData.cols = 4;
}

template <typename SpectrumType>
SpectrumView_t<SpectrumType>::~SpectrumView_t() {}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::paintEvent(QPaintEvent *) {
    auto font = this->font();
    auto fontMetrics = QFontMetrics(font);
    auto paddingLeft = ui::textWidth(mFontMetrics, nucare::toExponentFormat(10, 3)) + 2 * ui::PADDING_1;
    auto paddingBottom = fontMetrics.height() + ui::PADDING_1;
    auto paddingTop = fontMetrics.height();
    auto paddingRight = 6;
    auto contentRec = contentsRect();
    auto chartArea = QRect(contentRec.left() + paddingLeft,
                           contentRec.top() + paddingTop,
                           contentRec.width() - paddingLeft - paddingRight,
                           contentRec.height() - paddingBottom - paddingTop);
    auto xAxisRec = QRect(chartArea.left(), chartArea.bottom(), chartArea.width() + paddingRight, paddingBottom);
    auto yAxisRec = QRect(contentRec.x(), chartArea.y(), paddingLeft, chartArea.height());
    auto unitWidth = ceil(chartArea.width() / (float) mData.cols);
    auto unitHeight = chartArea.height() / mData.rows;
    auto unitRec = QRect(chartArea.topLeft(), QSize(unitWidth, unitHeight));

    QPainter painter(this);
    painter.setFont(font);
    drawAxisX(painter, xAxisRec, unitRec);
    drawAxisY(painter, yAxisRec, unitRec);
    drawGrid(painter, chartArea, unitRec);
    drawChart(painter, chartArea, QSizeF(chartArea.width() / (float) mData.maxViX,
                                         chartArea.height() / (float) mData.maxViY));
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::drawAxisY(QPainter& painter, const QRect& rec, const QRect& unitRec) {
    painter.save();
    painter.translate(rec.right() + mFontMetrics.height(),
                      rec.top() + ui::PADDING_1 + ui::textWidth(mFontMetrics, "Count"));
    painter.rotate(-90);
    painter.setPen(Qt::gray);
    painter.drawText(0, 0, "Count");
    painter.restore();

    painter.fillRect(rec.right(), rec.top(), 1, rec.height(), mFillBrush);
    auto padding = ui::PADDING_1;

    int i = mData.maxViY;
    while (i > 0) {
        auto value = toYValue(i);
        auto text = toYAxisText(value);
        auto textWidth = ui::textWidth(mFontMetrics, text);
        auto textHeight = mFontMetrics.height();
        if (shouldShowYLabel(i, toYValue(i))) {
            painter.drawText(rec.right() - textWidth - padding,
                             (mData.maxViY - i) * unitRec.height() + textHeight,
                             text);
        }
        i--;
    }
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::drawAxisX(QPainter& painter, const QRect& rec, const QRect& unitRec) {
    constexpr size_t total = 4;
    constexpr int axisX[total + 1] = {0, 750, 1500, 2250, 3000};

    for (unsigned int i = 0; i <= total; i++) {
        QString text;
        float channel;

        if (mData.coef) {
            channel = std::min((int) nucare::NcLibrary::energyToChannel(axisX[i], *mData.coef),
                          mData.maxX); // std::min
            text = QString::number(axisX[i]);
        } else {
            channel = i * mData.maxX / (float) total;
            text = QString::number((int)channel);
        }

        QPoint p(channel, 1);

        auto textWidth = ui::textWidth(mFontMetrics, text);
        auto textHeight = mFontMetrics.height();

        QTransform xTransform = QTransform::fromTranslate(rec.left() + (i == total ? -textWidth
                                                      : -textWidth / 2),
                             rec.top() + textHeight);
        xTransform.scale((float) rec.width() / mData.maxX, 1);

        p = xTransform.map(p);
        painter.drawText(p, text);
    }

    QString unit = mData.coef ? "KeV" : "CH";
    int unitXPos = rec.left() + mData.cols * unitRec.width() - ui::textWidth(mFontMetrics, unit);
    painter.save();
    painter.setPen(Qt::gray);
    painter.drawText(unitXPos, rec.top() - ui::PADDING_1, unit);
    painter.restore();
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::drawGrid(QPainter &painter, const QRect &rec, const QRect& unitRec) {
    auto color = QColor(0xFF, 0xFF, 0xFF, 20);
    painter.setPen(color);
    for (int i = 0; i <= mData.rows; i++) {
        painter.drawLine(rec.left(), i * unitRec.height() + rec.top(), rec.right(), i * unitRec.height() + rec.top());
    }
    for (int j = 0; j <= mData.cols; j++) {
        painter.drawLine(j * unitRec.width() + rec.left(), rec.top(), j * unitRec.width() + rec.left(), rec.height() + rec.top());
    }
    painter.fillRect(rec.left(), rec.bottom(), rec.width(), 1, mFillBrush);
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::drawChart(QPainter &painter, const QRect &rec, const QSizeF& unitSize) {
    painter.setPen(Qt::yellow);
    auto data = mData.spc->data();
    for (int i = 0; i < mData.chsize; i++) {
        auto xVal = tranformXValue(i);
        auto yVal = tranformYValue(data[i]);
        auto x = rec.left() + xVal * unitSize.width();
        auto y = rec.bottom() - yVal * unitSize.height();
        painter.drawLine(x, y, x, std::min((int) y + mLineLength, rec.bottom())); // std::min
    }
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::setFont(const QFont &font)
{
    QWidget::setFont(font);
    mFontMetrics = fontMetrics();
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::setData(std::shared_ptr<SpectrumType> spc) {
    auto data = spc->data();
    mData.spc = spc;
    mData.chsize = spc->getSize();
    mData.maxX = spc->getSize();
    auto maxValue = *std::max_element(data, data + mData.chsize - 1);

    mData.maxY = toYValue(tranformYValue(maxValue) * 1.2);
    mData.maxY = std::max(mData.maxY, (int) toYValue(mData.rows)); // std::max

    mData.maxViY = ceil(tranformYValue(mData.maxY));
    mData.maxViX = tranformXValue(mData.maxX);
    setGridRows(mData.maxViY);
    update();
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::setGridRows(const int rows) {
    mData.rows = rows;
    update();
}

template <typename SpectrumType>
void SpectrumView_t<SpectrumType>::setGridCols(const int cols)
{
    mData.cols = cols;
    update();
}

template <typename SpectrumType>
double SpectrumView_t<SpectrumType>::tranformXValue(const double rawValue) {
    return rawValue;
}

template <typename SpectrumType>
double SpectrumView_t<SpectrumType>::tranformYValue(const double rawValue) {
    if (rawValue < 10) {
        return rawValue / 10;
    } else {
        return log10(rawValue);
    }
}

template <typename SpectrumType>
QString SpectrumView_t<SpectrumType>::toXAxisText(const int /*index*/, const double& value) {
    int energy = 0;
    if (mData.coef) {
        energy = (int) nucare::NcLibrary::channelToEnergy(value, mData.coef->data());
    } else {
        energy = value;
    }
    if (energy < 0) energy = 0;
    return QString::number(energy);
}

template <typename SpectrumType>
QString SpectrumView_t<SpectrumType>::toYAxisText(const double value) {
    auto exp = (int) round(tranformYValue(value));
    return nucare::toExponentFormat(10, exp);
}

template <typename SpectrumType>
double SpectrumView_t<SpectrumType>::toXValue(const double chartValue) {
    return chartValue;
}

template <typename SpectrumType>
double SpectrumView_t<SpectrumType>::toYValue(const double chartValue) {
    if (chartValue < 1) {
        return chartValue * 10;
    }
    return pow(10, chartValue);
}

template <typename SpectrumType>
bool SpectrumView_t<SpectrumType>::shouldShowXLabel(const int /*index*/, const double /*value*/) {
    return true;
}

template <typename SpectrumType>
bool SpectrumView_t<SpectrumType>::shouldShowYLabel(const int index, const double value) {
    Q_UNUSED(index);
    return value > 0;
}


// Typedefs for common SpectrumView_t instantiations
typedef SpectrumView_t<Spectrum> SpectrumView;
typedef SpectrumView_t<HwSpectrum> SpectrumHwView;

#endif /* COMMON_WIDGET_SPECTRUMVIEW_H_ */
