#include "widget/concentriccircleswidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <algorithm> // For std::min

ConcentricCirclesWidget::ConcentricCirclesWidget(QWidget *parent) : QWidget(parent)
{
    initializeDefaultLayers();
    // Set a reasonable minimum size for the widget to be useful
    setMinimumSize(100, 100); // Smallest circle is 80x80 (radius 40)
}

void ConcentricCirclesWidget::initializeDefaultLayers()
{
    m_layers.clear();
    // Colors inspired by the image (shades of gray/green, lighter center)
    // Assuming 5 layers, index 0 is outermost, index 4 is innermost.
    // These are placeholder colors.
    m_layers.append(LayerData(0, QColor(50, 60, 50)));  // Outermost
    m_layers.append(LayerData(1, QColor(70, 80, 70)));
    m_layers.append(LayerData(2, QColor(90, 100, 90)));
    m_layers.append(LayerData(3, QColor(110, 120, 110)));
    m_layers.append(LayerData(4, QColor(200, 210, 200))); // Innermost (brightest)
}

void ConcentricCirclesWidget::setLayers(const QVector<LayerData>& layers)
{
    m_layers = layers;
    update(); // Trigger repaint
}

const QVector<LayerData>& ConcentricCirclesWidget::getLayers() const
{
    return m_layers;
}

void ConcentricCirclesWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    QPointF center(w / 2.0, h / 2.0);

    if (m_layers.isEmpty()) {
        return;
    }

    // Ensure there are exactly 5 layers for this specific drawing logic
    // Or adapt logic for variable number of layers if needed in future
    if (m_layers.size() != 5) {
        // Fallback or error: for now, just draw a simple circle or nothing
        // This example expects 5 layers for the radius calculation logic.
        // If you want dynamic layers, the radius calculation needs to be more generic.
        // For now, if not 5, we'll just clear and use defaults to avoid crashing.
        // Ideally, this state should be validated in setLayers or constructor.
        qWarning("ConcentricCirclesWidget: Expected 5 layers for drawing, found %d. Reinitializing.", m_layers.size());
        initializeDefaultLayers(); // Re-init to defaults if layer count is wrong
        if(m_layers.size() != 5) return; // Still not 5, something is wrong.
    }


    double max_outer_radius = std::min(w, h) / 2.0 * 0.95; // Use 95% to have a small margin
    double min_inner_radius = 20.0;

    if (max_outer_radius < min_inner_radius) {
        // Widget is too small to draw even the smallest defined circle.
        // Draw a single small circle fitting the widget or nothing.
        painter.setBrush(m_layers.last().color); // Color of the innermost intended circle
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(center, std::min(w,h)/2.0 * 0.90, std::min(w,h)/2.0 * 0.90);
        return;
    }
    
    // We have 5 layers. Layer 0 is outermost, Layer 4 is innermost.
    // The radii are: R_outer, R_outer-T, R_outer-2T, R_outer-3T, R_inner (which is R_outer-4T)
    // So, there are 4 steps of thickness.
    double total_thickness_span = max_outer_radius - min_inner_radius;
    double ring_thickness_step = 0;
    if (m_layers.size() > 1) {
         ring_thickness_step = total_thickness_span / (m_layers.size() - 1);
    }


    // Draw from largest (background) to smallest (foreground)
    // Layer indices in m_layers are 0 (outermost) to 4 (innermost)
    for (int i = 0; i < m_layers.size(); ++i) {
        const LayerData& layer = m_layers[i]; // Layer 0 is outermost
        double current_radius = max_outer_radius - (layer.index * ring_thickness_step);
        
        // Ensure radius doesn't go below a very small value if calculations are off
        if (current_radius < 1.0) current_radius = 1.0;

        painter.setBrush(layer.color);
        painter.setPen(Qt::NoPen); // No outline for the circles themselves
        painter.drawEllipse(center, current_radius, current_radius);
    }
}

void ConcentricCirclesWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    update(); // Repaint on resize
}
