#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "../app/global.h"
#include "../app/fmevent.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfileitemdelegate.h"
#include "fileinfo.h"
#include "dmenu.h"
#include "dscrollbar.h"

#include <dthememanager.h>

#include <QPushButton>
#include <QMenu>
#include <QWheelEvent>
#include <QDesktopServices>

DWIDGET_USE_NAMESPACE

DFileView::DFileView(QWidget *parent) : DListView(parent)
{
    D_THEME_INIT_WIDGET(DFileView);

    qDebug() << styleSheet();

    initUI();
    initDelegate();
    initModel();
    initConnects();
}

DFileView::~DFileView()
{

}

void DFileView::initUI()
{
    m_iconSizes << 48 << 64 << 96 << 128 << 256;

    setSpacing(5);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setIconSize(currentIconSize());
    setTextElideMode(Qt::ElideMiddle);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionRectVisible(true);
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);

    setVerticalScrollBar(new DScrollBar);
}

void DFileView::initDelegate()
{
    setItemDelegate(new DFileItemDelegate(this));
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
    setRootIndex(model()->setRootPath(QDir::currentPath()));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, &DFileView::openIndex);
    connect(fileSignalManager, &FileSignalManager::requestChangeCurrentUrl,
            this, &DFileView::cd);
    connect(fileSignalManager, &FileSignalManager::childrenChanged,
            model(), &DFileSystemModel::updateChildren);
    connect(fileSignalManager, &FileSignalManager::refreshFolder,
            model(), &DFileSystemModel::refresh);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

DFileItemDelegate *DFileView::itemDelegate() const
{
    return qobject_cast<DFileItemDelegate*>(DListView::itemDelegate());
}

QString DFileView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

bool DFileView::isIconViewMode()
{
    return orientation() == Qt::Vertical && isWrapping();
}

int DFileView::columnWidth(int column) const
{
    return m_headerView ? m_headerView->sectionSize(m_logicalIndexs.value(column)) : 100;
}

void DFileView::setColumnWidth(int column, int width)
{
    if(!m_headerView)
        return;

    m_headerView->resizeSection(m_logicalIndexs.value(column), width);
}

int DFileView::columnCount() const
{
    return m_headerView ? m_headerView->count() : 1;
}

QList<int> DFileView::columnRoleList() const
{
    return m_columnRoles;
}

int DFileView::selectedIndexCount() const
{
    return m_selectedIndexCount;
}

void DFileView::cd(const FMEvent &event)
{
    if(currentUrl() == event.dir)
        return;

    qDebug() << "cd: current url:" << currentUrl() << "to url:" << event;

    QModelIndex index = model()->index(event.dir);

    if(!index.isValid())
        index = model()->setRootPath(event.dir);

    setRootIndex(index);

    emit currentUrlChanged(event.dir);

    fileSignalManager->currentUrlChanged(event);
}

void DFileView::switchToListMode()
{
    if(!isIconViewMode())
        return;

    itemDelegate()->hideAllIIndexWidget();

    if(!m_headerView) {
        m_headerView = new QHeaderView(Qt::Horizontal);
        m_headerView->setModel(model());
        m_headerView->setSectionsMovable(true);
        m_headerView->setHighlightSections(true);
        m_headerView->setSectionsClickable(true);
        m_headerView->setSortIndicatorShown(true);
        m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_headerView->setDefaultSectionSize(120);
        m_headerView->setMinimumSectionSize(120);

        if(selectionModel()) {
            m_headerView->setSelectionModel(selectionModel());
        }

        connect(m_headerView, &QHeaderView::sectionResized,
                this, static_cast<void (DFileView::*)()>(&DFileView::update));
        connect(m_headerView, &QHeaderView::sectionMoved,
                this, &DFileView::moveColumnRole);
        connect(m_headerView, &QHeaderView::sortIndicatorChanged,
                model(), &QAbstractItemModel::sort);
    }

    for(int i = 0; i < m_headerView->count(); ++i) {
        m_columnRoles << model()->headerDataToRole(model()->headerData(i, m_headerView->orientation(), Qt::DisplayRole));
        m_logicalIndexs << i;

        if(m_columnRoles.last() == DFileSystemModel::FileNameRole) {
            m_headerView->resizeSection(i, width() - (m_headerView->count() - 1) * m_headerView->defaultSectionSize());
        }

        QSignalBlocker blocker(m_headerView);
        Q_UNUSED(blocker)
        m_headerView->setSortIndicator(i, model()->sortOrder());
    }

    addHeaderWidget(m_headerView);

    setIconSize(QSize(30, 30));
    setOrientation(QListView::TopToBottom, false);
    setSpacing(0);
}

void DFileView::switchToIconMode()
{
    if(isIconViewMode())
        return;

    if(m_headerView) {
        removeHeaderWidget(0);

        m_headerView = Q_NULLPTR;
    }

    m_columnRoles.clear();

    setIconSize(currentIconSize());
    setOrientation(QListView::LeftToRight, true);
    setSpacing(5);
}

void DFileView::moveColumnRole(int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
{
    m_columnRoles.move(oldVisualIndex, newVisualIndex);
    m_logicalIndexs.move(oldVisualIndex, newVisualIndex);

    update();
}

void DFileView::onMenuActionTrigger(const QAction *action)
{
    QMenu *menu = qobject_cast<QMenu*>(sender());

    if(!menu)
        return;

    FileMenuManager::MenuAction key = (FileMenuManager::MenuAction)action->data().toInt();

    switch (key) {
    case FileMenuManager::Open: {
        const QModelIndex index = qvariant_cast<QModelIndex>(menu->property("index"));

        openIndex(index);

        break;
    }
    default:
        break;
    }
}

void DFileView::contextMenuEvent(QContextMenuEvent *event)
{
    DMenu *menu;

    if (isEmptyArea(event->pos())){
        menu = FileMenuManager::createViewSpaceAreaMenu();
        menu->setProperty("index", rootIndex());
    }else{
        menu = FileMenuManager::createFileMenu();
        menu->setProperty("index", indexAt(event->pos()));
    }

    connect(menu, &DMenu::triggered, this, &DFileView::onMenuActionTrigger);

    menu->exec(mapToGlobal(event->pos()));
    menu->deleteLater();

    event->accept();
}

void DFileView::wheelEvent(QWheelEvent *event)
{
    if(isIconViewMode() && m_ctrlIsPressed) {
        if(event->angleDelta().y() > 0) {
            enlargeIconSize();
        } else {
            shrinkIconSize();
        }

        event->accept();
    } else {
        DListView::wheelEvent(event);
    }
}

void DFileView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        m_ctrlIsPressed = true;
    }

    DListView::keyPressEvent(event);
}

void DFileView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control) {
        m_ctrlIsPressed = false;
    }

    DListView::keyReleaseEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    setFocus();
}

void DFileView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        setDragEnabled(!isEmptyArea(event->pos()));
    } else if(event->button() == Qt::RightButton) {
        QWidget::mousePressEvent(event);

        return;
    }

    DListView::mousePressEvent(event);
}

void DFileView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    m_selectedIndexCount = selected.indexes().count();

    DListView::selectionChanged(selected, deselected);
}

bool DFileView::isEmptyArea(const QPoint &pos) const
{
    QModelIndex index = indexAt(pos);

    if(selectionModel()->selectedIndexes().contains(index)) {
        return false;
    } else {
        QStyleOptionViewItem option = viewOptions();

        option.rect = visualRect(index);

        const QList<QRect> &geometry_list = itemDelegate()->paintGeomertyss(option, index);

        for(const QRect &rect : geometry_list) {
            if(rect.contains(pos)) {
                return false;
            }
        }
    }

    return true;
}

QSize DFileView::currentIconSize() const
{
    int size = m_iconSizes.value(m_currentIconSizeIndex);

    return QSize(size, size);
}

void DFileView::enlargeIconSize()
{
    if(m_currentIconSizeIndex < m_iconSizes.count() - 1)
        ++m_currentIconSizeIndex;

    setIconSize(currentIconSize());
}

void DFileView::shrinkIconSize()
{
    if(m_currentIconSizeIndex > 0)
        --m_currentIconSizeIndex;

    setIconSize(currentIconSize());
}

void DFileView::openIndex(const QModelIndex &index)
{
    if(model()->hasChildren(index)){
        FMEvent event;

        event.dir = model()->getUrlByIndex(index);
        event.source = FMEvent::FileView;

        emit fileSignalManager->requestChangeCurrentUrl(event);
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(model()->getUrlByIndex(index)));
    }
}
