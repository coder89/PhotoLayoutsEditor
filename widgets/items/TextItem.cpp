#include "TextItem.h"
#include "global.h"
#include "KEditFactory.h"
#include "TextColorChangeListener.h"
#include "TextFontChangeListener.h"

#include <QTimeLine>
#include <QInputMethodEvent>
#include <QtTreePropertyBrowser>
#include <QtColorPropertyManager>

#include <klocalizedstring.h>

#define IS_NULL(node) if (node.isNull()) goto _delete;

using namespace KIPIPhotoLayoutsEditor;

class KIPIPhotoLayoutsEditor::TextItemPrivate
{
    TextItemPrivate(TextItem * item) :
        m_item(item),
        m_cursorIsVisible(false),
        m_cursor_line(0),
        m_cursor_character(0)
    {
    }

    void moveCursorLeft()
    {
        --m_cursor_character;
        if (m_cursor_character < 0)
        {
            --m_cursor_line;
            if (m_cursor_line < 0)
            {
                ++m_cursor_line;
                ++m_cursor_character;
            }
            else
                m_cursor_character = m_item->m_string_list.at(m_cursor_line).length();
        }
    }

    void moveCursorRight()
    {
        ++m_cursor_character;
        if (m_cursor_character > m_item->m_string_list.at(m_cursor_line).length())
        {
            ++m_cursor_line;
            if (m_cursor_line >= m_item->m_string_list.count())
            {
                --m_cursor_line;
                --m_cursor_character;
            }
            else
                m_cursor_character = 0;
        }
    }

    void moveCursorUp()
    {
        --(m_cursor_line);
        if (m_cursor_line < 0)
            m_cursor_line = 0;
        else if (m_cursor_character > m_item->m_string_list.at(m_cursor_line).length())
            m_cursor_character = m_item->m_string_list.at(m_cursor_line).length();
    }

    void moveCursorDown()
    {
        ++(m_cursor_line);
        if (m_cursor_line >= m_item->m_string_list.count())
            --m_cursor_line;
        else if (m_cursor_character > m_item->m_string_list.at(m_cursor_line).length())
            m_cursor_character = m_item->m_string_list.at(m_cursor_line).length();
    }

    void moveCursorEnd()
    {
        m_cursor_character = m_item->m_string_list.at(m_cursor_line).length();
    }

    void moveCursorHome()
    {
        m_cursor_character = 0;
    }

    void removeTextAfter()
    {
        if (m_cursor_character < m_item->m_string_list.at(m_cursor_line).length())
            m_item->m_string_list[m_cursor_line].remove(m_cursor_character, 1);
        else if (m_item->m_string_list.count()-1 > m_cursor_line)
        {
            m_item->m_string_list[m_cursor_line].append( m_item->m_string_list.at(m_cursor_line+1) );
            m_item->m_string_list.removeAt(m_cursor_line+1);
        }
    }

    void removeTextBefore()
    {
        if (m_cursor_character > 0 && m_item->m_string_list.at(m_cursor_line).length() >= m_cursor_character)
            m_item->m_string_list[m_cursor_line].remove(--m_cursor_character, 1);
        else if (m_cursor_line > 0)
        {
            m_item->m_string_list[m_cursor_line-1].append( m_item->m_string_list.at(m_cursor_line) );
            --m_cursor_line;
            m_cursor_character = m_item->m_string_list.at(m_cursor_line).length();
            m_item->m_string_list.removeAt(m_cursor_line+1);
        }
    }

    void addNewLine()
    {
        QString temp;
        if (m_cursor_character < m_item->m_string_list.at(m_cursor_line).length())
        {
            temp = m_item->m_string_list[m_cursor_line].right(m_item->m_string_list.at(m_cursor_line).length()-m_cursor_character);
            m_item->m_string_list[m_cursor_line].remove(m_cursor_character, m_item->m_string_list.at(m_cursor_line).length()-m_cursor_character+1);
        }
        m_cursor_character = 0;
        ++m_cursor_line;
        m_item->m_string_list.insert(m_cursor_line, temp);
    }

    void addText(const QString & text)
    {
        if (!text.length())
            return;
        m_item->m_string_list[m_cursor_line].insert(m_cursor_character, text);
        m_cursor_character += text.length();
    }

    void closeEditor()
    {
        m_item->clearFocus();
    }

    TextItem * m_item;

    QStringList m_temp_string;

    QPointF m_cursor_point;
    bool m_cursorIsVisible;
    int m_cursor_line;
    int m_cursor_character;

    friend class TextItem;
};

class KIPIPhotoLayoutsEditor::TextItem::TextEditUndoCommand : public QUndoCommand
{
        TextItem * m_item;
        QStringList m_prevText;
        bool done;
    public:
        TextEditUndoCommand(const QStringList & prevText, TextItem * item, bool isDone = true, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Change text"), parent),
            m_item(item),
            m_prevText(prevText),
            done(isDone)
        {}
        virtual void redo()
        {
            if (done)
                return;
            done = !done;
            QStringList temp = m_item->m_string_list;
            m_item->m_string_list = m_prevText;
            m_prevText = temp;
            m_item->refresh();
        }
        virtual void undo()
        {
            if (!done)
                return;
            done = !done;
            QStringList temp = m_item->m_string_list;
            m_item->m_string_list = m_prevText;
            m_prevText = temp;
            m_item->refresh();
        }
};
class KIPIPhotoLayoutsEditor::TextItem::TextColorUndoCommand : public QUndoCommand
{
        TextItem * m_item;
        QColor m_color;
    public:
        TextColorUndoCommand(const QColor & color, TextItem * item, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Change text color"), parent),
            m_item(item),
            m_color(color)
        {}
        virtual void redo()
        {
            run();
        }
        virtual void undo()
        {
            run();
        }
        void run()
        {
            QColor temp = m_item->m_color;
            m_item->m_color = m_color;
            m_color = temp;
            m_item->refresh();
        }
};
class KIPIPhotoLayoutsEditor::TextItem::TextFontUndoCommand : public QUndoCommand
{
        TextItem * m_item;
        QFont m_font;
    public:
        TextFontUndoCommand(const QFont & font, TextItem * item, QUndoCommand * parent = 0) :
            QUndoCommand(i18n("Change text font"), parent),
            m_item(item),
            m_font(font)
        {}
        virtual void redo()
        {
            run();
        }
        virtual void undo()
        {
            run();
        }
        void run()
        {
            QFont temp = m_item->m_font;
            m_item->m_font = m_font;
            m_font = temp;
            m_item->refresh();
        }
};

TextItem::TextItem(const QString & text, Scene * scene) :
    AbstractPhoto((text.isEmpty() ? i18n("Text item") : text), scene),
    d(new TextItemPrivate(this)),
    m_color(Qt::black),
    m_font(QFont()),
    m_string_list(QString(text).remove('\t').split('\n')),
    m_metrics(m_font)
{
    this->setFlag(QGraphicsItem::ItemIsFocusable);
    if (text.isEmpty())
        this->setName(i18n("New text"));
    else
        this->setName(text);
    this->refresh();
}

void TextItem::focusInEvent(QFocusEvent * event)
{
    if (!this->isSelected())
    {
        this->clearFocus();
        return;
    }
    d->m_temp_string = m_string_list;
    this->setCursorPositionVisible(true);
    AbstractPhoto::focusInEvent(event);
    this->setCursor(QCursor(Qt::IBeamCursor));
    this->setFlag(QGraphicsItem::ItemIsMovable, false);
}

void TextItem::focusOutEvent(QFocusEvent * event)
{
    if (this->scene() && d->m_temp_string != m_string_list)
    {
        QUndoCommand * undo = new TextEditUndoCommand(d->m_temp_string, this);
        PLE_PostUndoCommand(undo);
    }
    d->m_temp_string.clear();
    this->setCursorPositionVisible(false);
    AbstractPhoto::focusOutEvent(event);
    this->unsetCursor();
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->refresh();
}

void TextItem::keyPressEvent(QKeyEvent * event)
{
    switch (event->key())
    {
        case Qt::Key_Left:
            d->moveCursorLeft();
            break;
        case Qt::Key_Right:
            d->moveCursorRight();
            break;
        case Qt::Key_Up:
            d->moveCursorUp();
            break;
        case Qt::Key_Down:
            d->moveCursorDown();
            break;
        case Qt::Key_Home:
            d->moveCursorHome();
            break;
        case Qt::Key_End:
            d->moveCursorEnd();
            break;
        case Qt::Key_Delete:
            d->removeTextAfter();
            break;
        case Qt::Key_Backspace:
            d->removeTextBefore();
            break;
        case Qt::Key_Return:
            d->addNewLine();
            break;
        case Qt::Key_Escape:
            d->closeEditor();
            break;
        default:
            d->addText(event->text());
    }
    refreshItem();
    event->setAccepted(true);
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    QPointF p = event->pos();

    // Get clicked line number
    d->m_cursor_line =  p.y() / m_metrics.lineSpacing();
    if (d->m_cursor_line >= m_string_list.count())
        d->m_cursor_line = m_string_list.count()-1;
    QString currentLine = m_string_list.at( d->m_cursor_line );

    // Get clicked char position
    if (currentLine.length())
    {
        p.setX(p.x()-m_metrics.leftBearing(currentLine.at(0)));
        d->m_cursor_character = p.x() / m_metrics.averageCharWidth();
        if (d->m_cursor_character > currentLine.length())
            d->m_cursor_character = currentLine.length();
        int width = m_metrics.width(currentLine, d->m_cursor_character);
        int leftSpace = 0;
        while (width > p.x() && d->m_cursor_character > 0)
        {
            leftSpace = width - p.x();
            width = m_metrics.width(currentLine, --(d->m_cursor_character));
        }
        int rightSpace = 0;
        while (width < p.x() && d->m_cursor_character < currentLine.length())
        {
            rightSpace = p.x() - width;
            width = m_metrics.width(currentLine, ++(d->m_cursor_character));
        }
    }
    else
        p.setX(0);

    this->update();
}

QColor TextItem::color() const
{
    return m_color;
}

void TextItem::setColor(const QColor & color)
{
    QUndoCommand * undo = new TextColorUndoCommand(color, this);
    PLE_PostUndoCommand(undo);
}

QFont TextItem::font() const
{
    return m_font;
}

void TextItem::setFont(const QFont & font)
{
    QUndoCommand * undo = new TextFontUndoCommand(font, this);
    PLE_PostUndoCommand(undo);
}

QStringList TextItem::text() const
{
    return m_string_list;
}

QString TextItem::textMultiline() const
{
    return m_string_list.join("\n");
}

void TextItem::setText(const QStringList & textList)
{
    QUndoCommand * undo = new TextEditUndoCommand(textList, this, false);
    PLE_PostUndoCommand(undo);
}

void TextItem::setText(const QString & text)
{
    QString temp = text;
    temp.remove('\t');
    this->setText(temp.split('\n'));
}

QPainterPath TextItem::itemShape() const
{
    if (cropShape().isEmpty())
        return m_complete_path;
    else
        return m_complete_path & this->cropShape();
}

QPainterPath TextItem::itemOpaqueArea() const
{
    if (cropShape().isEmpty())
        return m_text_path;
    else
        return m_text_path & this->cropShape();
}

QPainterPath TextItem::itemDrawArea() const
{
    return m_complete_path;
}

void TextItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    if (!m_text_path.isEmpty())
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        if (this->cropShape().isEmpty())
            painter->fillPath(m_text_path, m_color);
        else
            painter->fillPath(m_text_path & this->cropShape(), m_color);
        painter->restore();
    }

    if (d->m_cursorIsVisible)
    {
        painter->save();
        painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        painter->setPen(Qt::gray);
        int y = m_metrics.lineSpacing() * d->m_cursor_line;
        int x = 0;
        if ( m_string_list.count() > d->m_cursor_line && !m_string_list.at(d->m_cursor_line).isEmpty() )
        {
            x = m_metrics.width(m_string_list.at(d->m_cursor_line),
                                d->m_cursor_character)
                - m_metrics.leftBearing(m_string_list.at(d->m_cursor_line).at(0));
        }
        painter->drawLine(x, y, x, y+m_metrics.lineSpacing());
        painter->restore();
    }
    AbstractPhoto::paint(painter, option, widget);
}

QDomElement TextItem::toSvg(QDomDocument & document) const
{
    QDomElement result = AbstractPhoto::toSvg(document);
    result.setAttribute("class", "TextItem");

    // 'defs' tag
    QDomElement defs = document.createElement("defs");
    defs.setAttribute("class", "data");
    result.appendChild(defs);

    // 'defs'-> pfe:'data'
    QDomElement appNS = document.createElementNS(KIPIPhotoLayoutsEditor::uri(), "data");
    appNS.setPrefix(KIPIPhotoLayoutsEditor::name());
    defs.appendChild(appNS);

    // 'defs'-> pfe:'data' -> 'text'
    QDomElement text = document.createElement("text");
    text.appendChild(document.createTextNode(m_string_list.join("\n").toUtf8()));
    text.setPrefix(KIPIPhotoLayoutsEditor::name());
    appNS.appendChild(text);

    // 'defs'-> pfe:'data' -> 'color'
    QDomElement color = document.createElement("color");
    color.setPrefix(KIPIPhotoLayoutsEditor::name());
    color.setAttribute("name", m_color.name());
    appNS.appendChild(color);

    // 'defs'-> pfe:'data' -> 'font'
    QDomElement font = document.createElement("font");
    font.setPrefix(KIPIPhotoLayoutsEditor::name());
    font.setAttribute("data", m_font.toString());
    appNS.appendChild(font);

    return result;
}

QDomElement TextItem::svgVisibleArea(QDomDocument & document) const
{
    QDomElement element = KIPIPhotoLayoutsEditor::pathToSvg(m_text_path, document);
    element.setAttribute("fill", m_color.name());
    return element;
}

TextItem * TextItem::fromSvg(QDomElement & element)
{
    TextItem * result = new TextItem();
    if (result->AbstractPhoto::fromSvg(element))
    {
        QDomElement defs = element.firstChildElement("defs");
        while (!defs.isNull() && defs.attribute("class") != "data")
            defs = defs.nextSiblingElement("defs");
        IS_NULL(defs);

        QDomElement data = defs.firstChildElement("data");
        IS_NULL(data);

        // text
        QDomElement text = data.firstChildElement("text");
        IS_NULL(text);
        QDomNode textValue = text.firstChild();
        while (!textValue.isNull() && !textValue.isText())
            textValue = textValue.nextSibling();
        IS_NULL(textValue);
        result->m_string_list = textValue.toText().data().remove('\t').split('\n');

        // Color
        QDomElement color = data.firstChildElement("color");
        IS_NULL(color);
        result->m_color = QColor(color.attribute("name"));

        // Font
        QDomElement font = data.firstChildElement("font");
        IS_NULL(font);
        result->m_font.fromString(font.attribute("data"));

        result->refresh();
        return result;
    }
_delete:
    delete result;
    return 0;
}

void TextItem::refreshItem()
{
    m_metrics = QFontMetrics(m_font);
    m_text_path = QPainterPath();
    int i = 1;
    int maxBearing = 0;
    int maxWidth = 0;
    const int lineSpacing = m_metrics.lineSpacing();
    foreach (QString string, m_string_list)
    {
        if (string.length())
        {
            int width = m_metrics.width(string);
            int leftBearing = -m_metrics.leftBearing(string.at(0));
            m_text_path.addText(leftBearing,
                                lineSpacing*(i)-m_metrics.descent(),
                                m_font,
                                string);
            if (maxWidth < width)
                maxWidth = width;
            if (maxBearing < leftBearing)
                maxBearing = leftBearing;
        }
        ++i;
    }
    if (maxWidth == 0)
        maxWidth = 1;
    m_complete_path = QPainterPath();
    m_complete_path.addRect(0,
                            0,
                            maxWidth + maxBearing,
                            m_string_list.count() * m_metrics.lineSpacing());
    this->prepareGeometryChange();
    this->updateIcon();
}

QtAbstractPropertyBrowser * TextItem::propertyBrowser()
{
    QtTreePropertyBrowser * browser = new QtTreePropertyBrowser();

    // Color
    QtColorPropertyManager * colorManager = new QtColorPropertyManager(browser);
    KColorEditorFactory * colorFactory = new KColorEditorFactory(browser);
    browser->setFactoryForManager(colorManager, colorFactory);
    QtProperty * colorProperty = colorManager->addProperty(i18n("Text color"));
    colorManager->setValue(colorProperty, m_color);
    browser->addProperty(colorProperty);
    TextColorChangeListener * colorListener = new TextColorChangeListener(this);
    colorListener->connect(browser, SIGNAL(destroyed()), SLOT(deleteLater()));
    colorListener->connect(colorManager, SIGNAL(propertyChanged(QtProperty*)), SLOT(propertyChanged(QtProperty*)));

    // Font
    QtFontPropertyManager * fontManager = new QtFontPropertyManager(browser);
    KFontEditorFactory * fontFactory = new KFontEditorFactory(browser);
    browser->setFactoryForManager(fontManager, fontFactory);
    QtProperty * fontProperty = fontManager->addProperty(i18n("Font"));
    fontManager->setValue(fontProperty, m_font);
    browser->addProperty(fontProperty);
    TextFontChangeListener * fontListener = new TextFontChangeListener(this);
    fontListener->connect(browser, SIGNAL(destroyed()), SLOT(deleteLater()));
    fontListener->connect(fontManager, SIGNAL(propertyChanged(QtProperty*)), SLOT(propertyChanged(QtProperty*)));

    return browser;
}

QPainterPath TextItem::getLinePath(const QString & string)
{
    QPainterPath result;
    result.addText(0, 0, m_font, string);
    return result;
}

void TextItem::setCursorPositionVisible(bool isVisible)
{
    d->m_cursorIsVisible = isVisible;
    this->update();
}

void TextItem::updateIcon()
{
    QPixmap px(50, 50);
    px.fill(Qt::transparent);
    QPainter p(&px);
    QFont f = this->font();
    f.setPixelSize(40);
    p.setFont(f);
    p.drawText(px.rect(), Qt::AlignCenter, "T");
    this->setIcon(QIcon(px));
}
