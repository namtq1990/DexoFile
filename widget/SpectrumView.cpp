/*
 * SpectrumView.cpp
 *
 *  Created on: Nov 11, 2021
 *      Author: quangnam
 */

#include "config.h"
#include "util/util.h"
#include "util/NcLibrary.h"

#include "SpectrumView.h"

#include <QPainter>
#include <QPainterPath>
using namespace std;

SpectrumView::SpectrumView(QWidget* parent) : QWidget(parent),
        mData(), mFillBrush(QColor(0x395589)), mFontMetrics(font())
{
    setData(make_shared<Spectrum>());
    mData.cols = 4;
}

SpectrumView::~SpectrumView() {}

void SpectrumView::paintEvent(QPaintEvent *) {
    auto font = this->font();
    auto fontMetrics = QFontMetrics(font);
//    auto paddingLeft = fontMetrics.width("COUNT");
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

void SpectrumView::drawAxisY(QPainter& painter, const QRect& rec, const QRect& unitRec) {

    painter.save();
    painter.translate(rec.right() + mFontMetrics.height(),
                      rec.top() + ui::PADDING_1 + ui::textWidth(mFontMetrics, "Count"));
    painter.rotate(-90);
    painter.setPen(Qt::gray);
    painter.drawText(0, 0, "Count");
    painter.restore();

    painter.fillRect(rec.right(), rec.top(), 1, rec.height(), mFillBrush);      // Draw the line axis
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

void SpectrumView::drawAxisX(QPainter& painter, const QRect& rec, const QRect& unitRec) {
    constexpr size_t total = 4;
    constexpr int axisX[total + 1] = {0, 750, 1500, 2250, 3000};

    for (unsigned int i = 0; i <= total; i++) {
        QString text;
        float channel;

        if (mData.coef) {
            channel = min((int) nucare::NcLibrary::energyToChannel(axisX[i], *mData.coef),
                          mData.maxX);
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

    // Draw axisX label
    QString unit = mData.coef ? "KeV" : "CH";
    int unitXPos = rec.left() + mData.cols * unitRec.width() - ui::textWidth(mFontMetrics, unit);
    painter.save();
    painter.setPen(Qt::gray);
    painter.drawText(unitXPos, rec.top() - ui::PADDING_1, unit);
    painter.restore();

}

void SpectrumView::drawGrid(QPainter &painter, const QRect &rec, const QRect& unitRec) {
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

void SpectrumView::drawChart(QPainter &painter, const QRect &rec, const QSizeF& unitSize) {
    painter.setPen(Qt::yellow);

    auto data = mData.spc->data();
    for (int i = 0; i < mData.chsize; i++) {
        auto xVal = tranformXValue(i);
        auto yVal = tranformYValue(data[i]);
        auto x = rec.left() + xVal * unitSize.width();
        auto y = rec.bottom() - yVal * unitSize.height();
        painter.drawLine(x, y, x, min((int) y + mLineLength, rec.bottom()));
    }
}

void SpectrumView::setFont(const QFont &font)
{
    QWidget::setFont(font);
    mFontMetrics = fontMetrics();
}

void SpectrumView::setData(shared_ptr<Spectrum> spc) {
    auto data = spc->data();
    mData.spc = spc;
    mData.chsize = spc->getSize();
    mData.maxX = spc->getSize();
    auto maxValue = *std::max_element(data, data + mData.chsize);

    mData.maxY = toYValue(tranformYValue(maxValue) * 1.2);
    mData.maxY = max(mData.maxY, (int) toYValue(mData.rows));

    mData.maxViY = ceil(tranformYValue(mData.maxY));
    mData.maxViX = tranformXValue(mData.maxX);
    setGridRows(mData.maxViY);

    update();
}

void SpectrumView::setGridRows(const int rows) {
    mData.rows = rows;
    update();
}

void SpectrumView::setGridCols(const int cols)
{
    mData.cols = cols;
    update();
}

double SpectrumView::tranformXValue(const double& rawValue) {
    return rawValue;
}

double SpectrumView::tranformYValue(const double &rawValue) {
    if (rawValue < 10) {
        return rawValue / 10;
    } else {
        return log10(rawValue);
    }
}

QString SpectrumView::toXAxisText(const int , const double& value) {
    int energy = 0;

    if (mData.coef) {
        energy = (int) nucare::NcLibrary::channelToEnergy(value, mData.coef->data());
    } else {
        energy = value;
    }

    if (energy < 0) energy = 0;

    return QString::number(energy);
}

QString SpectrumView::toYAxisText(const double& value) {
    auto exp = (int) round(tranformYValue(value));
    return nucare::toExponentFormat(10, exp);
}

double SpectrumView::toXValue(const double &chartValue) {
    return chartValue;
}

double SpectrumView::toYValue(const double &chartValue) {
    if (chartValue < 1) {
        return chartValue * 10;
    }

    return pow(10, chartValue);
}

bool SpectrumView::shouldShowXLabel(const int &, const double &) {
    return true;
}

bool SpectrumView::shouldShowYLabel(const int &index, const double &value) {
//    return  index % 2 == 0 && value > 0 && value < mData.maxY;
    Q_UNUSED(index);
    return value > 0;
}
