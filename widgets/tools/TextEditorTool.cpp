#include "TextEditorTool.h"
#include "TextItem.h"
#include "ToolsDockWidget.h"

#include <QVBoxLayout>

using namespace KIPIPhotoLayoutsEditor;

TextEditorTool::TextEditorTool(Scene * scene, QWidget * parent) :
    AbstractItemsTool(scene, Canvas::SingleSelcting, parent),
    m_text_item(0),
    m_created_text_item(0),
    m_browser(0),
    m_create_new_item(true)
{
    QVBoxLayout * layout = new QVBoxLayout();
    this->setLayout(layout);
}

void TextEditorTool::currentItemAboutToBeChanged()
{
    if (m_browser)
    {
        this->layout()->removeWidget(m_browser);
        m_browser->deleteLater();
        m_browser = 0;
    }
}

void TextEditorTool::currentItemChanged()
{
    m_text_item = dynamic_cast<TextItem*>(currentItem());
    if (m_text_item)
    {
        m_browser = m_text_item->propertyBrowser();
        if (m_browser)
            this->layout()->addWidget(m_browser);
    }
}

void TextEditorTool::positionAboutToBeChanged()
{
    if (m_text_item)
        m_create_new_item = false;
    else
        m_create_new_item = true;
}

void TextEditorTool::positionChanged()
{
    if (!m_create_new_item)
        return;
    if (!m_created_text_item || !m_created_text_item->text().join("\n").isEmpty())
        m_created_text_item = new TextItem();
    setCurrentItem( m_created_text_item );
    currentItem()->setPos( this->mousePosition() );
    emit itemCreated( currentItem() );
}
