// Local
#include "photolayoutseditor.h"
#include "photolayoutseditor_p.h"
#include "CanvasSizeDialog.h"
#include "Canvas.h"
#include "Scene.h"
#include "LayersSelectionModel.h"
#include "UndoCommandEventFilter.h"
#include "PhotoEffectsLoader.h"
#include "AbstractPhotoEffectFactory.h"
#include "ImageFileDialog.h"
#include "GridSetupDialog.h"
#include "PLEConfigDialog.h"
#include "PLEConfigSkeleton.h"
#include "global.h"

#include "BorderDrawerInterface.h"
#include "BorderDrawersLoader.h"

// Qt
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QTreeView>
#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QDockWidget>
#include <QHeaderView>
#include <QLabel>
#include <QApplication>
#include <QPushButton>
#include <QDebug>
#include <QPluginLoader>
#include <QFile>
#include <QPrintPreviewDialog>
#include <QImageWriter>
#include <QPrinter>
#include <QPrintDialog>

// KDE
#include <kmenubar.h>
#include <kstandardaction.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include <ktip.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kprintpreview.h>
#include <kconfigdialog.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kdebug.h>

using namespace KIPIPhotoLayoutsEditor;

class KIPIPhotoLayoutsEditor::CanvasSizeChangeCommand : public QUndoCommand
{
    CanvasSize m_size;
    Canvas * m_canvas;
public:
    CanvasSizeChangeCommand(const CanvasSize & size, Canvas * canvas, QUndoCommand * parent = 0) :
        QUndoCommand(i18n("Canvas size change"), parent),
        m_size(size),
        m_canvas(canvas)
    {}
    virtual void redo()
    {
        this->run();
    }
    virtual void undo()
    {
        this->run();
    }
    void run()
    {
        CanvasSize temp = m_canvas->canvasSize();
        m_canvas->setCanvasSize(m_size);
        m_size = temp;
    }
};

PhotoLayoutsEditor * PhotoLayoutsEditor::m_instance = 0;

PhotoLayoutsEditor::PhotoLayoutsEditor(QWidget * parent) :
    KXmlGuiWindow(parent),
    m_canvas(0),
    m_interface(0),
    d(new PhotoLayoutsEditorPriv)
{
    Q_INIT_RESOURCE(icons);

    setXMLFile("photolayoutseditorui.rc");

    setCaption("Photo Layouts Editor");

    m_instance = this;

    loadEffects();
    loadBorders();
    setupActions();
    createWidgets();
    refreshActions();

    this->setAcceptDrops(true);
}

PhotoLayoutsEditor::~PhotoLayoutsEditor()
{
    if (m_canvas)
        delete m_canvas;
    if (d)
        delete d;

    Q_CLEANUP_RESOURCE(icons);
}

PhotoLayoutsEditor * PhotoLayoutsEditor::instance(QWidget * parent)
{
    if (m_instance)
        return m_instance;
    else
    {
        KApplication * app = KApplication::kApplication();
        app->installEventFilter(new UndoCommandEventFilter(app));
        return (m_instance = new PhotoLayoutsEditor(parent));
    }
}

void PhotoLayoutsEditor::addUndoCommand(QUndoCommand * command)
{
    if (command)
    {
        if (m_canvas)
            m_canvas->undoStack()->push(command);
        else
        {
            command->redo();
            delete command;
        }
    }
}

void PhotoLayoutsEditor::beginUndoCommandGroup(const QString & name)
{
    if (m_canvas)
        m_canvas->undoStack()->beginMacro(name);
}

void PhotoLayoutsEditor::endUndoCommandGroup()
{
    if (m_canvas)
        m_canvas->undoStack()->endMacro();
}

void PhotoLayoutsEditor::setInterface(KIPI::Interface * interface)
{
    if (interface)
        m_interface = interface;
}

bool PhotoLayoutsEditor::hasInterface() const
{
    return (bool) m_interface;
}

KIPI::Interface * PhotoLayoutsEditor::interface() const
{
    return this->m_interface;
}

void PhotoLayoutsEditor::setupActions()
{
    d->openNewFileAction = KStandardAction::openNew(this, SLOT(open()), actionCollection());
    d->openNewFileAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_N));
    actionCollection()->addAction("open_new", d->openNewFileAction);
    //------------------------------------------------------------------------
    d->openFileAction = KStandardAction::open(this, SLOT(openDialog()), actionCollection());
    d->openFileAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_O));
    actionCollection()->addAction("open", d->openFileAction);
    //------------------------------------------------------------------------
    d->openRecentFilesMenu = KStandardAction::openRecent(this, SLOT(open(const KUrl &)), actionCollection());
    KUrl::List urls = PLEConfigSkeleton::recentFiles();
    foreach (KUrl url, urls)
        d->openRecentFilesMenu->addUrl(url);
    connect(d->openRecentFilesMenu, SIGNAL(recentListCleared()), this, SLOT(clearRecentList()));
    actionCollection()->addAction("open_recent", d->openRecentFilesMenu);
    //------------------------------------------------------------------------
    d->saveAction = KStandardAction::save(this, SLOT(save()), actionCollection());
    d->saveAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_S));
    actionCollection()->addAction("save", d->saveAction);
    //------------------------------------------------------------------------
    d->saveAsAction = KStandardAction::saveAs(this, SLOT(saveAs()), actionCollection());
    d->saveAsAction->setShortcut(KShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_S));
    actionCollection()->addAction("save_as", d->saveAsAction);
    //------------------------------------------------------------------------
    d->exportFileAction = new KAction(i18nc("Export current frame layout to image file", "Export"), actionCollection());
    d->exportFileAction->setShortcut(KShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_E));
    connect(d->exportFileAction, SIGNAL(triggered()), this, SLOT(exportFile()));
    actionCollection()->addAction("export", d->exportFileAction);
    //------------------------------------------------------------------------
    d->printPreviewAction = KStandardAction::printPreview(this, SLOT(printPreview()), actionCollection());
    d->printPreviewAction->setShortcut(KShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_P));
    actionCollection()->addAction("print_preview", d->printPreviewAction);
    //------------------------------------------------------------------------
    d->printAction = KStandardAction::print(this, SLOT(print()), actionCollection());
    d->printAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_P));
    actionCollection()->addAction("print", d->printAction);
    //------------------------------------------------------------------------
    d->closeAction = KStandardAction::close(this, SLOT(closeDocument()), actionCollection());
    d->closeAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Q));
    actionCollection()->addAction("close", d->closeAction);
    //------------------------------------------------------------------------
    d->quitAction = KStandardAction::quit(this, SLOT(close()), actionCollection());
    d->quitAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Q));
    actionCollection()->addAction("quit", d->quitAction);
    //------------------------------------------------------------------------
    d->undoAction = KStandardAction::undo(0, 0, actionCollection());
    d->undoAction->setShortcut(KShortcut(Qt::CTRL + Qt::Key_Z));
    actionCollection()->addAction("undo", d->undoAction);
    //------------------------------------------------------------------------
    d->redoAction = KStandardAction::redo(0, 0, actionCollection());
    d->redoAction->setShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
    actionCollection()->addAction("redo", d->redoAction);
    //------------------------------------------------------------------------
    d->settingsAction = KStandardAction::preferences(this, SLOT(settings()), actionCollection());
    actionCollection()->addAction("settings", d->settingsAction);
    //------------------------------------------------------------------------
    d->showGridToggleAction = new KToggleAction(i18nc("View grid lines", "Show"), actionCollection());
    d->showGridToggleAction->setShortcut(KShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_G));
    connect(d->showGridToggleAction, SIGNAL(triggered(bool)), this, SLOT(setGridVisible(bool)));
    actionCollection()->addAction("grid_toggle", d->showGridToggleAction);
    //------------------------------------------------------------------------
    d->gridConfigAction = new KAction(i18nc("Configure grid lines visibility", "Setup grid"), actionCollection());
    connect(d->gridConfigAction, SIGNAL(triggered()), this, SLOT(setupGrid()));
    actionCollection()->addAction("grid_config", d->gridConfigAction);
    //------------------------------------------------------------------------
    d->changeCanvasSizeAction = new KAction(i18nc("Configure canvas size", "Change canvas size"), actionCollection());
    connect(d->changeCanvasSizeAction, SIGNAL(triggered()), this, SLOT(changeCanvasSize()));
    actionCollection()->addAction("canvas_size", d->changeCanvasSizeAction);

    createGUI(xmlFile());
}

void PhotoLayoutsEditor::refreshActions()
{
    bool isEnabledForCanvas = false;
    if (m_canvas)
    {
        isEnabledForCanvas = true;
        d->undoAction->setEnabled(m_canvas->undoStack()->canUndo());
        d->redoAction->setEnabled(m_canvas->undoStack()->canRedo());
    }
    d->saveAction->setEnabled(isEnabledForCanvas && !m_canvas->isSaved());
    d->saveAsAction->setEnabled(isEnabledForCanvas);
    d->exportFileAction->setEnabled(isEnabledForCanvas);
    d->printPreviewAction->setEnabled(isEnabledForCanvas);
    d->printAction->setEnabled(isEnabledForCanvas);
    d->closeAction->setEnabled(isEnabledForCanvas);
    d->showGridToggleAction->setEnabled(isEnabledForCanvas);
    d->gridConfigAction->setEnabled(isEnabledForCanvas);
    d->changeCanvasSizeAction->setEnabled(isEnabledForCanvas);
    d->treeWidget->setEnabled(isEnabledForCanvas);
    d->toolsWidget->setEnabled(isEnabledForCanvas);
}

void PhotoLayoutsEditor::addRecentFile(const KUrl & url)
{
    if (url.isValid())
    {
        KUrl::List tempList = PLEConfigSkeleton::recentFiles();
        tempList.removeAll(url);
        tempList.push_front(url);
        unsigned maxCount = PLEConfigSkeleton::recentFilesCount();
        while ( ((unsigned)tempList.count()) > maxCount)
            tempList.removeAt(maxCount);
        PLEConfigSkeleton::setRecentFiles(tempList);
        if ( !d->openRecentFilesMenu->urls().contains( url ) )
            d->openRecentFilesMenu->addUrl( url );
    }
}

void PhotoLayoutsEditor::clearRecentList()
{
    PLEConfigSkeleton::setRecentFiles(KUrl::List());
}

void PhotoLayoutsEditor::createWidgets()
{
    // Tools
    d->toolsWidget = ToolsDockWidget::instance(this);
    this->addDockWidget(Qt::RightDockWidgetArea, d->toolsWidget);

    // Layers dockwidget
    d->treeWidget = new QDockWidget("Layers", this);
    d->treeWidget->setFeatures(QDockWidget::DockWidgetMovable);
    d->treeWidget->setFloating(false);
    d->treeWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    d->tree = new LayersTree(d->treeWidget);
    d->tree->setAnimated(true);
    d->treeWidget->setWidget(d->tree);
    d->treeTitle = new LayersTreeTitleWidget(d->treeTitle);
    d->treeWidget->setTitleBarWidget(d->treeTitle);
    this->addDockWidget(Qt::LeftDockWidgetArea, d->treeWidget);
    d->treeWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(d->toolsWidget,SIGNAL(requireMultiSelection()),d->tree,SLOT(setMultiSelection()));
    connect(d->toolsWidget,SIGNAL(requireSingleSelection()),d->tree,SLOT(setSingleSelection()));

    // Central widget (widget with canvas)
    d->centralWidget = new QWidget(this);
    d->centralWidget->setLayout(new QHBoxLayout(d->centralWidget));
    d->centralWidget->layout()->setSpacing(0);
    d->centralWidget->layout()->setMargin(0);
    this->setCentralWidget(d->centralWidget);

    this->open(KUrl("/home/coder89/Desktop/first.pfe"));   /// TODO : REMOVE WHEN FINISH
}

void PhotoLayoutsEditor::createCanvas(const CanvasSize & size)
{
    if (m_canvas)
    {
        d->centralWidget->layout()->removeWidget(m_canvas);
        m_canvas->deleteLater();
    }
    m_canvas = new Canvas(size, d->centralWidget);
    this->prepareSignalsConnections();
}

void PhotoLayoutsEditor::createCanvas(const KUrl & fileUrl)
{
    if (m_canvas)
    {
        d->centralWidget->layout()->removeWidget(m_canvas);
        m_canvas->deleteLater();
    }

    QFile file(fileUrl.path());
    QDomDocument document;
    document.setContent(&file, true);
    m_canvas = Canvas::fromSvg(document);
    if (m_canvas)
    {
        m_canvas->setFile(fileUrl);
        m_canvas->setParent(d->centralWidget);
        this->prepareSignalsConnections();
        m_canvas->undoStack()->clear(); /// TODO : prevent pushing to undo stack when reading from file!!!!
    }
    else
    {
        KMessageBox::error(this,
                           i18n("Can't read image file."));
    }
    file.close();
}

void PhotoLayoutsEditor::prepareSignalsConnections()
{
    d->centralWidget->layout()->addWidget(m_canvas);
    d->tree->setModel(m_canvas->model());
    d->tree->setSelectionModel(m_canvas->selectionModel());
    d->toolsWidget->setScene(m_canvas->scene());

    // undo stack signals
    connect(m_canvas,               SIGNAL(savedStateChanged()),    this,                   SLOT(refreshActions()));
    connect(m_canvas->undoStack(),  SIGNAL(canRedoChanged(bool)),   d->redoAction,          SLOT(setEnabled(bool)));
    connect(m_canvas->undoStack(),  SIGNAL(canUndoChanged(bool)),   d->undoAction,          SLOT(setEnabled(bool)));
    connect(d->undoAction,          SIGNAL(triggered()),            m_canvas->undoStack(),  SLOT(undo()));
    connect(d->redoAction,          SIGNAL(triggered()),            m_canvas->undoStack(),  SLOT(redo()));

    // model/tree/canvas synchronization signals
    connect(d->tree,    SIGNAL(selectedRowsAboutToBeRemoved()),     m_canvas,   SLOT(removeSelectedRows()));
    connect(d->tree,    SIGNAL(selectedRowsAboutToBeMovedUp()),     m_canvas,   SLOT(moveSelectedRowsUp()));
    connect(d->tree,    SIGNAL(selectedRowsAboutToBeMovedDown()),   m_canvas,   SLOT(moveSelectedRowsDown()));
    connect(d->treeTitle->moveUpButton(),   SIGNAL(clicked()),      m_canvas,   SLOT(moveSelectedRowsUp()));
    connect(d->treeTitle->moveDownButton(), SIGNAL(clicked()),      m_canvas,   SLOT(moveSelectedRowsDown()));
    // interaction modes (tools)
    connect(m_canvas,       SIGNAL(selectedItem(AbstractPhoto*)),       d->toolsWidget,SLOT(itemSelected(AbstractPhoto*)));
    connect(d->toolsWidget, SIGNAL(undoCommandCreated(QUndoCommand*)),  m_canvas,   SLOT(newUndoCommand(QUndoCommand*)));
    connect(d->toolsWidget, SIGNAL(pointerToolSelected()),              m_canvas,   SLOT(enableDefaultSelectionMode()));
    connect(d->toolsWidget, SIGNAL(handToolSelected()),                 m_canvas,   SLOT(enableViewingMode()));
    connect(d->toolsWidget, SIGNAL(canvasToolSelected()),               m_canvas,   SLOT(enableCanvasEditingMode()));
    connect(d->toolsWidget, SIGNAL(effectsToolSelected()),              m_canvas,   SLOT(enableEffectsEditingMode()));
    connect(d->toolsWidget, SIGNAL(textToolSelected()),                 m_canvas,   SLOT(enableTextEditingMode()));
    connect(d->toolsWidget, SIGNAL(rotateToolSelected()),               m_canvas,   SLOT(enableRotateEditingMode()));
    connect(d->toolsWidget, SIGNAL(scaleToolSelected()),                m_canvas,   SLOT(enableScaleEditingMode()));
    connect(d->toolsWidget, SIGNAL(cropToolSelected()),                 m_canvas,   SLOT(enableCropEditingMode()));
    connect(d->toolsWidget, SIGNAL(borderToolSelected()),               m_canvas,   SLOT(enableBordersEditingMode()));
    connect(d->toolsWidget, SIGNAL(newItemCreated(AbstractPhoto*)),     m_canvas,   SLOT(addNewItem(AbstractPhoto*)));
    connect(m_canvas->scene()->toGraphicsScene(), SIGNAL(mousePressedPoint(QPointF)), d->toolsWidget, SLOT(mousePositionChoosen(QPointF)));

    d->toolsWidget->setDefaultTool();
}

void PhotoLayoutsEditor::open()
{
    closeDocument();

    CanvasSizeDialog * canvasSizeDialog = new CanvasSizeDialog(this);
    canvasSizeDialog->setModal(true);
    int result = canvasSizeDialog->exec();

    CanvasSize size = canvasSizeDialog->canvasSize();
    if (result == KDialog::Accepted && size.isValid())
        createCanvas(size);

    refreshActions();
}

void PhotoLayoutsEditor::openDialog()
{
    if (!d->fileDialog)
        d->fileDialog = new KFileDialog(KUrl(), "*.pfe|Photo Frames Editor files", this);
    d->fileDialog->setOperationMode(KFileDialog::Opening);
    d->fileDialog->setMode(KFile::File);
    d->fileDialog->setKeepLocation(true);
    int result = d->fileDialog->exec();
    if (result == KFileDialog::Accepted)
        open(d->fileDialog->selectedUrl());
}

void PhotoLayoutsEditor::open(const KUrl & fileUrl)
{
    if (m_canvas && m_canvas->file() == fileUrl)
        return;

    if (fileUrl.isValid())
    {
        closeDocument();
        createCanvas(fileUrl);
        refreshActions();

        // Adds recent open file
        this->addRecentFile(m_canvas->file());
    }
}

void PhotoLayoutsEditor::save()
{
    if (!m_canvas)
        return;
    if (m_canvas->file().fileName().isEmpty())
        saveAs();
    else
        saveFile();
}

void PhotoLayoutsEditor::saveAs()
{
    if (!d->fileDialog)
        d->fileDialog = new KFileDialog(KUrl(), "*.pfe|Photo Frames Editor files", this);
    d->fileDialog->setOperationMode(KFileDialog::Saving);
    d->fileDialog->setMode(KFile::File);
    d->fileDialog->setKeepLocation(true);
    int result = d->fileDialog->exec();
    if (result == KFileDialog::Accepted)
    {
        KUrl url = d->fileDialog->selectedUrl();
        saveFile(url);
    }
}

void PhotoLayoutsEditor::saveFile(const KUrl & fileUrl, bool setFileAsDefault)
{
    if (m_canvas)
    {
        QString error = m_canvas->save(fileUrl, setFileAsDefault);
        if (!error.isEmpty())
        {
            KMessageBox::detailedError(this,
                                       i18n("Can't save canvas to file!"),
                                       error);
        }
    }
    else
        KMessageBox::error(this,
                           i18n("There is nothing to save!"));
}

void PhotoLayoutsEditor::exportFile()
{
    if (!m_canvas)
        return;
    ImageFileDialog imageDialog(KUrl(), this);
    imageDialog.setOperationMode(KFileDialog::Saving);
    int result = imageDialog.exec();
    if (result == KFileDialog::Accepted)
    {
        const char * format = imageDialog.format();
        if (format)
        {
            QPixmap image(m_canvas->sceneRect().size().toSize());
            image.fill(Qt::transparent);
            m_canvas->renderCanvas(&image);
            QImageWriter writer(imageDialog.selectedFile());
            writer.setFormat(format);
            if (!writer.canWrite())
            {
                KMessageBox::error(this,
                                   i18n("Image can't be saved in selected file."));
            }
            if (!writer.write(image.toImage()))
            {
                KMessageBox::detailedError(this,
                                   i18n("Unexpected error while saving an image!"),
                                   writer.errorString());
            }
        }
    }
}

void PhotoLayoutsEditor::printPreview()
{
    if (m_canvas && m_canvas->scene())
    {
        QPrinter printer;
        m_canvas->preparePrinter(&printer);
        QPrintPreviewDialog dialog(&printer, this);
        connect(&dialog, SIGNAL(paintRequested(QPrinter*)), m_canvas, SLOT(renderCanvas(QPrinter*)));
        dialog.exec();
    }
}

void PhotoLayoutsEditor::print()
{
    QPrinter printer;
    m_canvas->preparePrinter(&printer);
    QPrintDialog dialog(&printer, this);
    connect(&dialog, SIGNAL(accepted(QPrinter*)), m_canvas, SLOT(renderCanvas(QPrinter*)));
    dialog.exec();
}

bool PhotoLayoutsEditor::closeDocument()
{
    if (m_canvas)
    {
        // Adds recent open file
        this->addRecentFile(m_canvas->file());

        // Try to save unsaved changes
        int saving = KMessageBox::No;
        if (!m_canvas->isSaved())
            saving = KMessageBox::warningYesNoCancel( this, i18n("Save changes to current frame?"));
        switch (saving)
        {
            case KMessageBox::Yes:
                save();
            case KMessageBox::No:
                d->tree->setModel(0);
                m_canvas->deleteLater();
                m_canvas = 0;
                refreshActions();
                return true;
            default:
                return false;
        }
    }
    refreshActions();
    return true;
}

bool PhotoLayoutsEditor::queryClose()
{
    if (closeDocument())
        return true;
    else
        return false;
}

void PhotoLayoutsEditor::settings()
{
    if ( KConfigDialog::showDialog( "settings" ) )
        return;

    PLEConfigDialog * dialog = new PLEConfigDialog(this);
    dialog->show();
}

void PhotoLayoutsEditor::setGridVisible(bool isVisible)
{
    d->showGridToggleAction->setChecked(isVisible);
    PLEConfigSkeleton::setShowGrid(isVisible);
    PLEConfigSkeleton::self()->writeConfig();
    if (m_canvas && m_canvas->scene())
        m_canvas->scene()->setGridVisible(isVisible);
}

void PhotoLayoutsEditor::setupGrid()
{
    if (m_canvas && m_canvas->scene())
    {
        GridSetupDialog dialog(this);
        dialog.setHorizontalDistance( m_canvas->scene()->gridHorizontalDistance() );
        dialog.setVerticalDistance( m_canvas->scene()->gridVerticalDistance() );
        dialog.exec();
        m_canvas->scene()->setGrid(dialog.horizontalDistance(),
                                   dialog.verticalDistance());
    }
}

void PhotoLayoutsEditor::changeCanvasSize()
{
    if (!m_canvas)
        return;
    CanvasSizeDialog ccd(m_canvas->canvasSize(), this);
    int result = ccd.exec();
    CanvasSize size = ccd.canvasSize();
    if (result == KDialog::Accepted)
    {
        if (size.isValid())
        {
            if (m_canvas->canvasSize() != size)
            {
                CanvasSizeChangeCommand * command = new CanvasSizeChangeCommand(size, m_canvas);
                PLE_PostUndoCommand(command);
            }
        }
        else
            KMessageBox::error(this, i18n("Invalid image size!"));
    }
}

void PhotoLayoutsEditor::loadEffects()
{
    const KService::List offers = KServiceTypeTrader::self()->query("PhotoLayoutsEditor/EffectPlugin");
    foreach (const KService::Ptr& service, offers)
    {
        if (service)
            d->effectsServiceMap[service->name()] = service;
    }

    foreach (const QString & name, d->effectsServiceMap.keys())
    {
        KService::Ptr service = d->effectsServiceMap.value(name);
        AbstractPhotoEffectFactory * plugin;

        if ( d->effectsMap.contains(name) )
            continue;
        else
        {
            QString error;
            plugin = service->createInstance<AbstractPhotoEffectFactory>(this, QVariantList(), &error);
            if (plugin)
            {
                d->effectsMap[name] = plugin;
                PhotoEffectsLoader::registerEffect(plugin);
                kDebug() << "PhotoLayoutsEditor: Loaded effect " << service->name();
            }
            else
            {
                kWarning() << "PhotoLayoutsEditor: createInstance returned 0 for "
                           << service->name()
                           << " (" << service->library() << ")"
                           << " with error: "
                           << error;
            }
        }
    }
}

void PhotoLayoutsEditor::loadBorders()
{
    const KService::List offers = KServiceTypeTrader::self()->query("PhotoLayoutsEditor/BorderPlugin");
    foreach (const KService::Ptr& service, offers)
    {
        if (service)
            d->bordersServiceMap[service->name()] = service;
    }

    foreach (const QString & name, d->bordersServiceMap.keys())
    {
        KService::Ptr service = d->bordersServiceMap.value(name);
        BorderDrawerFactoryInterface * plugin;

        if ( d->bordersMap.contains(name) )
            continue;
        else
        {
            QString error;
            plugin = service->createInstance<BorderDrawerFactoryInterface>(this, QVariantList(), &error);
            if (plugin)
            {
                d->bordersMap[name] = plugin;
                BorderDrawersLoader::registerDrawer(plugin);
                kDebug() << "PhotoLayoutsEditor: Loaded border:" << service->name();
            }
            else
            {
                kWarning() << "PhotoLayoutsEditor: createInstance returned 0 for "
                           << service->name()
                           << " (" << service->library() << ")"
                           << " with error: "
                           << error;
            }
        }
    }
}
