#ifndef SCALINGWIDGETITEM_H
#define SCALINGWIDGETITEM_H

#include "AbstractItemInterface.h"

namespace KIPIPhotoLayoutsEditor
{
    class AbstractPhoto;
    class ScalingWidgetItemPrivate;

    class ScalingWidgetItem : public AbstractItemInterface
    {
            ScalingWidgetItemPrivate * d;

        public:

            ScalingWidgetItem(QGraphicsItem * parent = 0, QGraphicsScene * scene = 0);
            virtual ~ScalingWidgetItem();

            virtual QRectF boundingRect() const;
            virtual QPainterPath opaqueArea() const;
            virtual QPainterPath shape() const;
            virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);

            virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
            virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

            void setScaleItems(const QList<AbstractPhoto*> & items);

        friend class ScalingWidgetItemPrivate;
    };
}

#endif // SCALINGWIDGETITEM_H
