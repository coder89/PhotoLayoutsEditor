#ifndef ABSTRACTMOVABLEMODEL_H
#define ABSTRACTMOVABLEMODEL_H

#include <QAbstractItemModel>

namespace KIPIPhotoLayoutsEditor
{
    class AbstractMovableModel : public QAbstractItemModel
    {
        public:
            AbstractMovableModel(QObject * parent = 0) :
                QAbstractItemModel(parent)
            {}
            virtual bool moveRows(int sourcePosition, int sourceCount, int destPosition) = 0;
            virtual void setItem(QObject * graphicsItem, const QModelIndex & index) = 0;
            virtual QObject * item(const QModelIndex & index) const = 0;

        Q_DISABLE_COPY(AbstractMovableModel)
    };
};

#endif // ABSTRACTMOVABLEMODEL_H
