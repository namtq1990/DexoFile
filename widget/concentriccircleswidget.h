#ifndef CONCENTRICCIRCLESWIDGET_H
#define CONCENTRICCIRCLESWIDGET_H

#include <QWidget>
#include <QColor>
#include <QVector>

struct LayerData {
    int index;
    QColor color;

    LayerData(int idx = 0, QColor col = Qt::transparent) : index(idx), color(col) {}
};

class ConcentricCirclesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ConcentricCirclesWidget(QWidget *parent = nullptr);

    void setLayers(const QVector<LayerData>& layers);
    const QVector<LayerData>& getLayers() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;


private:
    QVector<LayerData> m_layers;
    
    // Define 5 default layers.
    // Layers will be drawn from index 0 (largest/outermost) to 4 (smallest/innermost).
    void initializeDefaultLayers(); 
};

#endif // CONCENTRICCIRCLESWIDGET_H
