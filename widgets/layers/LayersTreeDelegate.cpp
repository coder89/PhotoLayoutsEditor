#include "LayersTreeDelegate.h"
#include "LayersModelItem.h"
#include "AbstractPhoto.h"

#include <QPainter>

using namespace KIPIPhotoLayoutsEditor;

LayersTreeDelegate::LayersTreeDelegate(QWidget * parent) :
    QStyledItemDelegate(parent),
    m_eye(QPixmap::fromImage(QImage(":/eye.png"))),
    m_eye_off(m_eye.size()),
    m_padlock(QPixmap::fromImage(QImage(":/padlock.png"))),
    m_padlock_off(m_padlock.size())
{
    QPainter p;

    m_eye_off.fill(Qt::transparent);
    p.begin(&m_eye_off);
    p.drawPixmap(0,0,m_eye);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(m_eye_off.rect(), QColor(0, 0, 0, 80));
    p.end();

    m_padlock_off.fill(Qt::transparent);
    p.begin(&m_padlock_off);
    p.drawPixmap(0,0,m_padlock);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(m_padlock_off.rect(), QColor(0, 0, 0, 80));
    p.end();
}

void LayersTreeDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (index.column() == LayersModelItem::EyeIcon)
    {
        painter->fillRect(option.rect,Qt::white);
        AbstractPhoto * photo = static_cast<LayersModelItem*>(index.internalPointer())->photo();
        if (photo)
        {
            QPoint point = option.rect.topLeft();
            point += QPoint(option.rect.width()-m_eye.width(), option.rect.height()-m_eye.height())*0.5;
            if (photo->isVisible())
                painter->drawPixmap(point,m_eye);
            else
                painter->drawPixmap(point,m_eye_off);

        }
    }
    else if (index.column() == LayersModelItem::PadLockIcon)
    {
        painter->fillRect(option.rect,Qt::white);
        AbstractPhoto * photo = static_cast<LayersModelItem*>(index.internalPointer())->photo();
        if (photo)
        {
            QPoint point = option.rect.topLeft();
            point += QPoint(option.rect.width()-m_padlock.width(), option.rect.height()-m_padlock.height())*0.5;
            if (photo->flags() & QGraphicsItem::ItemIsSelectable)
                painter->drawPixmap(point,m_padlock_off);
            else
                painter->drawPixmap(point,m_padlock);
        }
    }
    else
        QStyledItemDelegate::paint(painter,option,index);
}

QSize LayersTreeDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    if (index.column() == LayersModelItem::EyeIcon)
        return m_eye.size();
    else if (index.column() == LayersModelItem::PadLockIcon)
        return m_padlock.size();
    else
        return QStyledItemDelegate::sizeHint(option,index);
}

void LayersTreeDelegate::itemClicked(const QModelIndex & index)
{
    if (index.column() == LayersModelItem::EyeIcon)
    {
        AbstractPhoto * photo = static_cast<LayersModelItem*>(index.internalPointer())->photo();
        if (photo)
        {
            photo->setVisible(!photo->isVisible());
            emit itemRepaintNeeded(index);
        }
    }
    else if (index.column() == LayersModelItem::PadLockIcon)
    {
        AbstractPhoto * photo = static_cast<LayersModelItem*>(index.internalPointer())->photo();
        if (photo)
        {
            photo->setFlags(photo->flags() ^ QGraphicsItem::ItemIsSelectable);
            emit itemRepaintNeeded(index);
        }
    }
}
