#ifndef TOOLSDOCKWIDGET_H
#define TOOLSDOCKWIDGET_H

#include <QDockWidget>
#include <QLayout>
#include <QUndoCommand>
#include <QStackedLayout>

#include <kpushbutton.h>

#include "AbstractTool.h"

namespace KIPIPhotoLayoutsEditor
{
    class Scene;
    class AbstractPhoto;
    class AbstractItemsTool;
    class CanvasEditTool;

    class ToolsDockWidget : public QDockWidget
    {
            Q_OBJECT

            KPushButton * m_tool_pointer;
            KPushButton * m_tool_hand;
            KPushButton * m_canvas_button;
            KPushButton * m_effects_button;
            KPushButton * m_text_button;
            KPushButton * m_rotate_button;
            KPushButton * m_scale_button;
            KPushButton * m_crop_button;
            KPushButton * m_tool_border;
            KPushButton * m_tool_colorize_button;

            bool m_has_selection;

            QStackedLayout * m_tool_widget_layout;
            CanvasEditTool * m_canvas_widget;
            AbstractItemsTool * m_effects_widget;
            AbstractItemsTool * m_text_widget;
            AbstractItemsTool * m_border_widget;

            AbstractPhoto * m_currentPhoto;

            Scene * m_scene;

            static ToolsDockWidget * m_instance;

        public:

            static ToolsDockWidget * instance(QWidget * parent = 0);
            void setDefaultTool();

        signals:

            void undoCommandCreated(QUndoCommand * command);
            void newItemCreated(AbstractPhoto * item);

            void requireSingleSelection();
            void requireMultiSelection();

            void pointerToolSelected();
            void handToolSelected();
            // Effects tool selection signals
            void canvasToolSelectionChanged(bool);
            void canvasToolSelected();
            // Effects tool selection signals
            void effectsToolSelectionChanged(bool);
            void effectsToolSelected();
            // Text tool selection signals
            void textToolSelectionChanged(bool);
            void textToolSelected();
            // Rotate tool selection signals
            void rotateToolSelectionChanged(bool);
            void rotateToolSelected();
            // Scale tool selection signals
            void scaleToolSelectionChanged(bool);
            void scaleToolSelected();
            // Crop tool selection signals
            void cropToolSelectionChanged(bool);
            void cropToolSelected();
            // Border tool selection signals
            void borderToolSelectionChanged(bool);
            void borderToolSelected();

        public slots:

            void setScene(Scene * scene = 0);
            void itemSelected(AbstractPhoto * photo);
            void mousePositionChoosen(const QPointF & position);
            void emitNewItemCreated(AbstractPhoto * item);

            void selectionChanged(bool hasSelection)
            {
                m_tool_widget_layout->setEnabled(hasSelection);
                m_has_selection = hasSelection;
            }

            void setCanvasWidgetVisible(bool isVisible = true);
            void setEffectsWidgetVisible(bool isVisible = true);
            void setTextWidgetVisible(bool isVisible = true);
            void setRotateWidgetVisible(bool isVisible = true);
            void setScaleWidgetVisible(bool isVisible = true);
            void setCropWidgetVisible(bool isVisible = true);
            void setBordersWidgetVisible(bool isVisible = true);

            void emitPointerToolSelected(bool isSelected = true)
            {
                if (isSelected)
                {
                    m_tool_widget_layout->setCurrentIndex(0);
                    this->unsetCursor();
                    emit requireMultiSelection();
                    emit pointerToolSelected();
                }
            }

            void emitHandToolSelected(bool isSelected = true)
            {
                if (isSelected)
                {
                    m_tool_widget_layout->setCurrentIndex(0);
                    this->unsetCursor();
                    emit requireMultiSelection();
                    emit handToolSelected();
                }
            }

        private:

            explicit ToolsDockWidget(QWidget * parent = 0);
    };
}

#endif // TOOLSDOCKWIDGET_H
