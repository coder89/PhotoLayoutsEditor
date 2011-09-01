#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "AbstractPhoto.h"

#include <QObject>
#include <QtColorPropertyManager>

namespace KIPIPhotoLayoutsEditor
{
    class TextItemPrivate;

    class TextItem : public AbstractPhoto
    {
            Q_OBJECT

            static QColor DEFAULT_COLOR;
            static QFont DEFAULT_FONT;

            class TextEditUndoCommand;
            class TextColorUndoCommand;
            class TextFontUndoCommand;

            TextItemPrivate * d;

            QColor m_color;
            QFont m_font;
            QStringList m_string_list;

            QPainterPath m_complete_path;
            QPainterPath m_text_path;
            QFontMetrics m_metrics;

        public:

            TextItem(const QString & text = QString(), Scene * scene = 0);
            virtual void focusInEvent(QFocusEvent *event);
            virtual void focusOutEvent(QFocusEvent *event);
            virtual void keyPressEvent(QKeyEvent * event);
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

            Q_PROPERTY(QColor m_color READ color WRITE setColor)
            QColor color() const;
            void setColor(const QColor & color);

            Q_PROPERTY(QFont m_font READ font WRITE setFont)
            QFont font() const;
            void setFont(const QFont & font);

            Q_PROPERTY(QStringList m_string_list READ text WRITE setText)
            QStringList text() const;
            void setText(const QStringList & textList);

            QString textMultiline() const;
            void setText(const QString & text);

            virtual QPainterPath itemShape() const;
            virtual QPainterPath itemOpaqueArea() const;
            virtual QPainterPath itemDrawArea() const;

            virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

            virtual QDomElement toSvg(QDomDocument & document) const;
            virtual QDomElement svgVisibleArea(QDomDocument &document) const;
            static TextItem * fromSvg(QDomElement & element);

            /// Returns item's property browser
            virtual QtAbstractPropertyBrowser * propertyBrowser();

        private:

            virtual void refreshItem();
            QPainterPath getLinePath(const QString & string);
            void setCursorPositionVisible(bool isVisible);
            void updateIcon();


        friend class TextItemPrivate;

        friend class TextEditUndoCommand;
        friend class TextColorUndoCommand;
        friend class TextFontUndoCommand;
    };
}

#endif // TEXTITEM_H
