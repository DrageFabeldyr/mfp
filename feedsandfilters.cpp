#include "feedsandfilters.h"

FeedsAndFilters::FeedsAndFilters(QWidget *parent): QWidget()
{
    pFeeds = static_cast<DFRSSFilter*>(parent)->pFeeds; // приводим тип, т.к. parent у нас QWidget
    settings = static_cast<DFRSSFilter*>(parent)->settings; // приводим тип, т.к. parent у нас QWidget

    feedModel = new FeedModel(parent);

    feedList = new QTreeView (this);
    feedList->setModel(feedModel);
    feedList->setColumnWidth(0, 40); // чтобы не было видно численного значения "галочки"
    feedList->hideColumn(3); // спрячем столбец с id (он нужен для работы с БД)

    filterModel = new FilterModel(parent);

    filterList = new QTreeView (this);
    filterList->setModel(filterModel);
    filterList->setColumnWidth(0, 40); // чтобы не было видно численного значения "галочки"
    filterList->hideColumn(3); // спрячем столбец с id (он нужен для работы с БД)
    filterList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    h1_layout = new QHBoxLayout;
    feedAdd = new QPushButton;
    feedEdit = new QPushButton;
    feedDel = new QPushButton;
    h1_layout->addWidget(feedAdd);
    h1_layout->addWidget(feedEdit);
    h1_layout->addWidget(feedDel);
    feedAdd->setText(tr("Добавить"));
    feedEdit->setText(tr("Редактировать"));
    feedDel->setText(tr("Удалить"));

    h2_layout = new QHBoxLayout;
    filterAdd = new QPushButton;
    filterEdit = new QPushButton;
    filterDel = new QPushButton;
    h2_layout->addWidget(filterAdd);
    h2_layout->addWidget(filterEdit);
    h2_layout->addWidget(filterDel);
    filterAdd->setText(tr("Добавить"));
    filterEdit->setText(tr("Редактировать"));
    filterDel->setText(tr("Удалить"));

    h3_layout = new QHBoxLayout;
    filterCheckAll = new QPushButton;
    filterUncheckAll = new QPushButton;
    filterDeleteAll = new QPushButton;
    h3_layout->addWidget(filterCheckAll);
    h3_layout->addWidget(filterUncheckAll);
    h3_layout->addWidget(filterDeleteAll);
    filterCheckAll->setText(tr("Включить все"));
    filterUncheckAll->setText(tr("Отключить все"));
    filterDeleteAll->setText(tr("Удалить все"));

    vb_layout = new QVBoxLayout;
    vb_layout->addLayout(h2_layout);
    vb_layout->addLayout(h3_layout);

    hint1 = new QLabel(this);
    hint2 = new QLabel(this);
    hint1->setText("В ленте, не имеющей фильтров, будут выведены все новости");
    hint2->setText("Добавление, редактирование и удаление фильтров доступно только после выбора ленты, которой они принадлежат");

    v_layout = new QVBoxLayout;
    v_layout->addLayout(h1_layout);
    v_layout->addWidget(feedList);
    v_layout->addWidget(hint1);
    v_layout->addLayout(vb_layout);
    v_layout->addWidget(filterList);
    v_layout->addWidget(hint2);

    setLayout(v_layout); // установка главного лэйаута

    resize(640,480);
    setWindowTitle("RSS-ленты и фильтры");
    connect(feedList, SIGNAL(clicked(QModelIndex)), this, SLOT(UpdateFilter(QModelIndex)));
    connect(feedList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ShowEditFeed(QModelIndex)));
    connect(filterList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ShowEditFilter(QModelIndex)));
    connect(filterAdd, SIGNAL(clicked(bool)), this, SLOT(ShowAddFilter()));
    connect(feedAdd, SIGNAL(clicked(bool)), this, SLOT(ShowAddFeed()));
    connect(filterEdit, SIGNAL(clicked(bool)), this, SLOT(ShowEditFilter()));
    connect(feedEdit, SIGNAL(clicked(bool)), this, SLOT(ShowEditFeed()));
    connect(filterDel, SIGNAL(clicked(bool)), this, SLOT(FilterDel()));
    connect(feedDel, SIGNAL(clicked(bool)), this, SLOT(FeedDel()));
    connect(filterCheckAll, SIGNAL(clicked(bool)), this, SLOT(filters_check_all()));
    connect(filterUncheckAll, SIGNAL(clicked(bool)), this, SLOT(filters_uncheck_all()));
    connect(filterDeleteAll, SIGNAL(clicked(bool)), this, SLOT(filters_delete_all()));

    this->setWindowIcon(QIcon(":/rss.ico"));

    show();
}

void FeedsAndFilters::updateFeeds()
{
    feedModel->Update();
    // и на всякий случай:
    feedList->setColumnWidth(0, 40); // чтобы не было видно численного значения "галочки"
    feedList->hideColumn(3); // спрячем столбец с id (он нужен для работы с БД)
}

void FeedsAndFilters::updateFilters()
{
    filterModel->Update();
    // и на всякий случай:
    filterList->setColumnWidth(0, 40); // чтобы не было видно численного значения "галочки"
    filterList->hideColumn(3); // спрячем столбец с id (он нужен для работы с БД)
}

void FeedsAndFilters::UpdateFilter(QModelIndex indexx)
{
    QModelIndex mindex =  feedModel->index(indexx.row(), 3);
    QVariant value = feedModel->data(mindex, Qt::DisplayRole);
    filterModel->idFeed = QString::number(value.toInt());
    updateFilters();
}

// функция обработки редактирования по нажатию кнопки
void FeedsAndFilters::ShowEditFeed()
{
    if (feedList->selectionModel()->selectedIndexes().isEmpty())
        return;
    QModelIndex mindex = feedList->selectionModel()->selectedIndexes().first();

    ShowEditFeed(mindex);
}

// функция обработки редактирования по двойному клику на ленту
void FeedsAndFilters::ShowEditFeed(QModelIndex indexx)
{
    QModelIndex mindex =  feedModel->index(indexx.row(),3);
    QVariant value = feedModel->data(mindex, Qt::DisplayRole);
    editW = new EditWindow(EditWindow::enFeed, value.toInt(), this, settings->activateFeeds);
    connect(editW, SIGNAL(destroyed(QObject*)), this , SLOT(updateFeeds()));
    editW->setAttribute(Qt::WA_ShowModal, true); // блокирует родительское окно
    editW->show();
}

// функция обработки редактирования по нажатию кнопки
void FeedsAndFilters::ShowEditFilter()
{
    if (filterList->selectionModel()->selectedIndexes().isEmpty())
        return;
    QModelIndex mindex =  filterList->selectionModel()->selectedIndexes().first();

    ShowEditFilter(mindex);
}

// функция обработки редактирования по двойному клику на фильтр
void FeedsAndFilters::ShowEditFilter(QModelIndex indexx)
{
    QModelIndex mindex =  filterModel->index(indexx.row(),3);
    QVariant value = filterModel->data(mindex, Qt::DisplayRole);
    editW = new EditWindow(EditWindow::enFilter, value.toInt(), this, settings->activateFilters);
    connect(editW, SIGNAL(destroyed(QObject*)), this , SLOT(updateFilters()));
    editW->setAttribute(Qt::WA_ShowModal, true); // блокирует родительское окно
    editW->show();
}

void FeedsAndFilters::ShowAddFilter()
{
    if (feedList->selectionModel()->selectedIndexes().isEmpty())
        return;   

    QModelIndex mindex = feedModel->index(feedList->selectionModel()->selectedIndexes().first().row(), 3);
    parentFeedId = feedModel->data(mindex, Qt::DisplayRole).toInt();

    editW = new EditWindow(EditWindow::enFilter, -1, this, settings->activateFilters);
    connect(editW, SIGNAL(destroyed(QObject*)), this , SLOT(updateFilters()));
    editW->setAttribute(Qt::WA_ShowModal, true); // блокирует родительское окно
    editW->show();
}

void FeedsAndFilters::ShowAddFeed()
{
    editW = new EditWindow(EditWindow::enFeed, -1, this, settings->activateFeeds);
    connect(editW, SIGNAL(destroyed(QObject*)), this , SLOT(updateFeeds()));
    editW->setAttribute(Qt::WA_ShowModal, true); // блокирует родительское окно
    editW->show();
}

void FeedsAndFilters::FeedDel()
{
    if (feedList->selectionModel()->selectedIndexes().empty())
        return;
    QModelIndex mindex =  feedModel->index(feedList->selectionModel()->selectedIndexes().first().row(), 3);
    pFeeds->DeleteFeedById(feedModel->data(mindex, Qt::DisplayRole).toInt());
    updateFeeds();
    updateFilters();
}

void FeedsAndFilters::FilterDel()
{
    if (filterList->selectionModel()->selectedIndexes().empty())
        return;
    /*
    // так было реализовано удаление одного выделенного элемента, когда можно было выбрать только один
    QModelIndex mindex =  filterModel->index(filterList->selectionModel()->selectedIndexes().first().row(), 3);
    pFeeds->DeleteFilterById(filterModel->data(mindex, Qt::DisplayRole).toInt());
    updateFilters();
    */
    QModelIndexList selected = filterList->selectionModel()->selectedIndexes();
    foreach (QModelIndex mindex, selected)
    {
         pFeeds->DeleteFilterById(filterModel->data(mindex, Qt::DisplayRole).toInt());
    }
    updateFilters();
}

void FeedsAndFilters::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    settings->timer->start(); // закрыли окно - запустили таймер
}

void FeedsAndFilters::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    settings->timer->stop(); // останавливаем таймер на время работы с окном
}

void FeedsAndFilters::filters_delete_all()
{
    if (feedList->selectionModel()->selectedIndexes().isEmpty()) // чтобы показались фильтры лента должна быть выделена
        return;
    QModelIndex mindex =  feedModel->index(feedList->selectionModel()->selectedIndexes().first().row(), 3);
    pFeeds->DeleteAllFilters(feedModel->data(mindex, Qt::DisplayRole).toInt());
    updateFilters();
}

void FeedsAndFilters::filters_check_all()
{
    filters_change_status_all(1);
}

void FeedsAndFilters::filters_uncheck_all()
{
    filters_change_status_all(0);
}

void FeedsAndFilters::filters_change_status_all(int status)
{
    if (feedList->selectionModel()->selectedIndexes().isEmpty()) // чтобы показались фильтры лента должна быть выделена
        return;
    QModelIndex mindex =  feedModel->index(feedList->selectionModel()->selectedIndexes().first().row(), 3);
    pFeeds->ChangeStatusAllFilters(feedModel->data(mindex, Qt::DisplayRole).toInt(), status);
    updateFilters();
}
