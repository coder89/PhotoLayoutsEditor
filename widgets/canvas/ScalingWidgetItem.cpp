#include "ScalingWidgetItem.h"
#include "AbstractPhoto.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

using namespace KIPIPhotoLayoutsEditor;

class KIPIPhotoLayoutsEditor::ScalingWidgetItemPrivate
{   
    enum
    {
        Top,
        VCenter,
        Bottom
    };

    enum
    {
        Left,
        HCenter,
        Right,
    };

    ScalingWidgetItemPrivate() :
        currentViewTransform(0, 0, 0,    0, 0, 0,   0, 0, 0),
        recalculate(true),
        pressedVHandler(-1),
        pressedHHandler(-1)
    {
        m_handlers[Top][Left] = QRectF(0, 0, 20, 20);
        m_handlers[Top][HCenter] = QRectF(0, 0, 20, 20);
        m_handlers[Top][Right] = QRectF(0, 0, 20, 20);
        m_handlers[VCenter][Left] = QRectF(0, 0, 20, 20);
        m_handlers[VCenter][Right] = QRectF(0, 0, 20, 20);
        m_handlers[Bottom][Left] = QRectF(0, 0, 20, 20);
        m_handlers[Bottom][HCenter] = QRectF(0, 0, 20, 20);
        m_handlers[Bottom][Right] = QRectF(0, 0, 20, 20);
        m_elipse.addEllipse(QPointF(0,0), 10, 10);
    }

    QTransform currentViewTransform;
    void transformDrawings(const QTransform & viewTransform);
    void calculateDrawings();
    void correctRect(QRectF & r);

    QList<AbstractPhoto*> m_items;
    QPainterPath m_crop_shape;
    QPainterPath m_shape;
    QRectF m_rect;
    QRectF m_begin_rect;
    QRectF m_handlers[Bottom+1][Right+1];
    QPainterPath m_elipse;
    bool recalculate;
    int pressedVHandler;
    int pressedHHandler;
    QPointF handlerOffset;

    friend class ScalingWidgetItem;
};

void ScalingWidgetItemPrivate::transformDrawings(const QTransform & viewTransform)
{
    if (currentViewTransform == viewTransform && !recalculate)
        return;

    m_rect = currentViewTransform.inverted().mapRect(m_rect);
    m_rect = viewTransform.mapRect(m_rect);

    this->calculateDrawings();

    recalculate = false;
    currentViewTransform = viewTransform;
}

void ScalingWidgetItemPrivate::calculateDrawings()
{
    // Scale height of handlers
    qreal h = qAbs(m_rect.height()) - 60;
    h = (h < 0 ? h / 3.0 : 0);
    h = (h < -10 ? -10 : h);
    m_handlers[Top][Left].setHeight(20+h);
    m_handlers[Top][HCenter].setHeight(20+h);
    m_handlers[Top][Right].setHeight(20+h);
    m_handlers[VCenter][Left].setHeight(20+h);
    m_handlers[VCenter][Right].setHeight(20+h);
    m_handlers[Bottom][Left].setHeight(20+h);
    m_handlers[Bottom][HCenter].setHeight(20+h);
    m_handlers[Bottom][Right].setHeight(20+h);

    // Scale width of handlers
    qreal w = qAbs(m_rect.width()) - 60;
    w = (w < 0 ? w / 3.0 : 0);
    w = (w < -10 ? -10 : w);
    m_handlers[Top][Left].setWidth(20+w);
    m_handlers[Top][HCenter].setWidth(20+w);
    m_handlers[Top][Right].setWidth(20+w);
    m_handlers[VCenter][Left].setWidth(20+w);
    m_handlers[VCenter][Right].setWidth(20+w);
    m_handlers[Bottom][Left].setWidth(20+w);
    m_handlers[Bottom][HCenter].setWidth(20+w);
    m_handlers[Bottom][Right].setWidth(20+w);

    m_elipse = QPainterPath();
    m_elipse.addEllipse(m_rect.center(), 18 + w, 18 + h);

    w = qAbs(m_rect.width()) - 35;
    w = (w < 0 ? w / 2.0 : 0);
    h = qAbs(m_rect.height()) - 35;
    h = (h < 0 ? h / 2.0 : 0);
    m_handlers[Top][Left].moveCenter(m_rect.topLeft() + QPointF(w,h));
    m_handlers[Top][HCenter].moveCenter( QPointF( m_rect.center().x(), m_rect.top() + h ) );
    m_handlers[Top][Right].moveCenter(m_rect.topRight() + QPointF(-w,h));
    m_handlers[VCenter][Left].moveCenter( QPointF( m_rect.left() + w, m_rect.center().y() ) );
    m_handlers[VCenter][Right].moveCenter( QPointF( m_rect.right() - w, m_rect.center().y() ) );
    m_handlers[Bottom][Left].moveCenter(m_rect.bottomLeft() + QPointF(w,-h));
    m_handlers[Bottom][HCenter].moveCenter( QPointF( m_rect.center().x(), m_rect.bottom() - h ) );
    m_handlers[Bottom][Right].moveCenter(m_rect.bottomRight() + QPointF(-w,-h));

    m_shape = QPainterPath();
    m_shape.setFillRule(Qt::WindingFill);
    m_shape.addRect(m_rect);
    for (int i = Top; i <= Bottom; ++i)
        for (int j = Left; j <= Right; ++j)
            m_shape.addRect(m_handlers[i][j]);
}

void ScalingWidgetItemPrivate::correctRect(QRectF & r)
{
    if (r.bottom() < r.top())
    {
        if (this->pressedVHandler == Top)
            r.setTop( r.bottom() - 1 );
        else
            r.setBottom( r.top() + 1);
    }

    if (r.left() > r.right())
    {
        if (this->pressedHHandler == Left)
            r.setLeft( r.right() - 1 );
        else
            r.setRight( r.left() + 1);
    }
}

ScalingWidgetItem::ScalingWidgetItem(QGraphicsItem * parent, QGraphicsScene * scene) :
    AbstractItemInterface(parent, scene),
    d(new ScalingWidgetItemPrivate)
{
    this->setAcceptHoverEvents(true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, false);
    this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

ScalingWidgetItem::~ScalingWidgetItem()
{
    delete d;
}

QRectF ScalingWidgetItem::boundingRect() const
{
    return d->m_shape.boundingRect();
}

QPainterPath ScalingWidgetItem::opaqueArea() const
{
    return d->m_shape;
}

QPainterPath ScalingWidgetItem::shape() const
{
    return d->m_shape;
}

void ScalingWidgetItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * widget)
{
    // Get the view
    QGraphicsView * view = qobject_cast<QGraphicsView*>(widget->parentWidget());
    if (!view)
        return;
    QTransform viewTransform = view->transform();
    d->transformDrawings(viewTransform);

    painter->save();

    // Draw bounding rect
    painter->setCompositionMode(QPainter::RasterOp_NotSourceAndNotDestination);
    painter->setPen(Qt::red);
    painter->setPen(Qt::SolidLine);
    painter->drawPath(d->m_shape);
    painter->drawPath(d->m_elipse);

    painter->restore();
}

void ScalingWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    d->pressedVHandler = -1;
    d->pressedHHandler = -1;
    d->handlerOffset = QPointF(0,0);
    this->setFocus( Qt::MouseFocusReason );
    d->m_begin_rect = d->m_rect;
    if (event->button() == Qt::LeftButton)
    {
        QGraphicsView * view = qobject_cast<QGraphicsView*>(event->widget()->parentWidget());
        QPointF handledPoint = view->transform().map(this->mapFromScene(event->buttonDownScenePos(Qt::LeftButton)));
        for (int i = ScalingWidgetItemPrivate::Top; i <= ScalingWidgetItemPrivate::Bottom; ++i)
        {
            for (int j = ScalingWidgetItemPrivate::Left; j <= ScalingWidgetItemPrivate::Right; ++j)
            {
                if (d->m_handlers[i][j].contains(handledPoint))
                {
                    d->pressedVHandler = i;
                    d->pressedHHandler = j;
                    goto found;
                }
            }
        }
        if (d->m_elipse.contains(handledPoint))
        {
            d->pressedVHandler = ScalingWidgetItemPrivate::VCenter;
            d->pressedHHandler = ScalingWidgetItemPrivate::HCenter;
        }
        return;
        found:
            d->handlerOffset = d->m_handlers[d->pressedVHandler][d->pressedHHandler].center() - handledPoint;
    }
}

void ScalingWidgetItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (d->pressedHHandler == -1 || d->pressedVHandler == -1)
        return;

    QGraphicsView * view = qobject_cast<QGraphicsView*>(event->widget()->parentWidget());

    // New handler position calc
    QPointF point = view->transform().map(event->pos()) + d->handlerOffset;

    QRectF temp = d->m_rect;

    // Position change
    if (d->pressedVHandler == ScalingWidgetItemPrivate::VCenter &&
        d->pressedHHandler == ScalingWidgetItemPrivate::HCenter)
    {
        QPointF dif = view->transform().map(event->scenePos()) - view->transform().map(event->lastScenePos());
        temp.translate(dif);
        dif = (event->scenePos()) - (event->lastScenePos());
        foreach (AbstractPhoto * item, d->m_items)
            item->QGraphicsItem::moveBy(dif.x(), dif.y());
    }
    // Size change
    else
    {
        // Vertical size change
        if (d->pressedVHandler == ScalingWidgetItemPrivate::Top)
            temp.setTop( point.y() );
        else if (d->pressedVHandler == ScalingWidgetItemPrivate::Bottom)
            temp.setBottom( point.y() );

        // Horizontal size change
        if (d->pressedHHandler == ScalingWidgetItemPrivate::Right)
            temp.setRight( point.x() );
        else if (d->pressedHHandler == ScalingWidgetItemPrivate::Left)
            temp.setLeft( point.x() );

        d->correctRect( temp );

        // Keeps aspect ratio
        if (event->modifiers() & Qt::ShiftModifier)
        {
            qreal xFactor = temp.width()  / d->m_begin_rect.width();
            qreal yFactor = temp.height() / d->m_begin_rect.height();
            if (d->pressedHHandler == ScalingWidgetItemPrivate::HCenter)
            {
                qreal dif = d->m_begin_rect.width() - d->m_begin_rect.width() * yFactor;
                temp.setRight( d->m_begin_rect.right() - dif / 2.0 );
                temp.setLeft( d->m_begin_rect.left() + dif / 2.0 );
            }
            else if (d->pressedVHandler == ScalingWidgetItemPrivate::VCenter)
            {
                qreal dif = d->m_begin_rect.height() - d->m_begin_rect.height() * xFactor;
                temp.setTop( d->m_begin_rect.top() + dif / 2.0 );
                temp.setBottom( d->m_begin_rect.bottom() - dif / 2.0 );
            }
            else if (xFactor > yFactor)
            {
                qreal dif = d->m_begin_rect.width() - d->m_begin_rect.width() * yFactor;
                if (d->pressedHHandler == ScalingWidgetItemPrivate::Right)
                    temp.setRight( d->m_begin_rect.right() - dif );
                else if (d->pressedHHandler == ScalingWidgetItemPrivate::Left)
                    temp.setLeft( d->m_begin_rect.left() + dif );
            }
            else if (xFactor < yFactor)
            {
                qreal dif = d->m_begin_rect.height() - d->m_begin_rect.height() * xFactor;
                if (d->pressedVHandler == ScalingWidgetItemPrivate::Top)
                    temp.setTop( d->m_begin_rect.top() + dif );
                else if (d->pressedVHandler == ScalingWidgetItemPrivate::Bottom)
                    temp.setBottom( d->m_begin_rect.bottom() - dif );
            }
        }
    }

    QTransform sc;
    sc.scale(temp.width() / d->m_rect.width(), temp.height() / d->m_rect.height());

    foreach(AbstractPhoto * item, d->m_items)
    {
        QRectF before = item->mapRectToScene(item->boundingRect());
        item->setTransform( item->transform() * sc );
        QRectF after = item->mapRectToScene(item->boundingRect());
        QPointF p(0,0);
        if (d->pressedVHandler == ScalingWidgetItemPrivate::Top)
            p.setY(before.bottom() - after.bottom());
        else if (d->pressedVHandler == ScalingWidgetItemPrivate::Bottom)
            p.setY(before.top() - after.top());
        else
            p.setY((before.center() - after.center()).y());

        if (d->pressedHHandler == ScalingWidgetItemPrivate::Left)
            p.setX(before.right() - after.right());
        else if (d->pressedHHandler == ScalingWidgetItemPrivate::Right)
            p.setX(before.left() - after.left());
        else
            p.setX((before.center() - after.center()).x());

        item->moveBy( p.x(), p.y() );
    }

    d->m_rect = temp;
    d->calculateDrawings();
    event->setAccepted(true);
    this->update();
}

void ScalingWidgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * /*event*/)
{}

void ScalingWidgetItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * /*event*/)
{}

void ScalingWidgetItem::setScaleItems(const QList<AbstractPhoto*> & items)
{
    d->m_items = items;

    d->m_crop_shape = QPainterPath();
    foreach (AbstractPhoto * item, items)
        d->m_crop_shape += this->mapFromItem(item, item->opaqueArea());

    this->setPos( d->m_crop_shape.boundingRect().topLeft() );
    d->m_crop_shape.translate( -d->m_crop_shape.boundingRect().topLeft() );
    d->m_rect = d->m_crop_shape.boundingRect();

    d->recalculate = true;
    d->calculateDrawings();
    if (!d->m_rect.isValid())
    {
        this->hide();
        return;
    }
    this->update();
    this->show();
}
