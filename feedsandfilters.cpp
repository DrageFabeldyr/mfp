#include "feedsandfilters.h"
#include "dfrssfilter.h"


FeedsAndFilters::FeedsAndFilters(QWidget *parent): QWidget(parent)
{
    this->setAttribute(Qt::WA_AcceptTouchEvents, true); // для обработки нажатия на экран


    pFeeds = static_cast<DFRSSFilter*>(parent)->pFeeds; // приводим тип, т.к. parent у нас QWidget
    settings = static_cast<DFRSSFilter*>(parent)->settings; // приводим тип, т.к. parent у нас QWidget

    feedModel = new FeedModel(parent);

    feedList = new QTreeView (this);
    feedList->setModel(feedModel);
    feedList->setColumnWidth(0, 40); // чтобы не было видно численного значения "галочки"
    feedList->hideColumn(3); // спрячем столбец с id (он нужен для работы с БД)
    feedList->setSelectionMode(QAbstractItemView::ExtendedSelection);   // чтобы можно было выделить несколько
    feedList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(feedList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_feeds_menu()));
#ifdef Q_OS_ANDROID
    feedList->setAttribute(Qt::WA_AcceptTouchEvents, true); // для обработки нажатия на экран
    feedList->setAttribute(Qt::WA_StaticContents);
#endif

    filterModel = new FilterModel(parent);

    filterList = new QTreeView (this);
    filterList->setModel(filterModel);
    filterList->setColumnWidth(0, 40); // чтобы не было видно численного значения "галочки"
    filterList->hideColumn(3); // спрячем столбец с id (он нужен для работы с БД)
    filterList->setSelectionMode(QAbstractItemView::ExtendedSelection); // чтобы можно было выделить несколько
    filterList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(filterList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_filters_menu()));
#ifdef Q_OS_ANDROID
    filterList->setAttribute(Qt::WA_AcceptTouchEvents, true); // для обработки нажатия на экран
    filterList->setAttribute(Qt::WA_StaticContents);
#endif
/*
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
*/
    hint1 = new QLabel(this);
    hint2 = new QLabel(this);
    /*
    switch (settings->current_language)
    {
    case 1:
        hint1->setText("В ленте, не имеющей фильтров, будут выведены все новости");
        hint2->setText("Добавление, редактирование и удаление фильтров доступно только после выбора ленты, которой они принадлежат");
        setWindowTitle("RSS-ленты и фильтры");
        break;
    case 2:
        hint1->setText("Feed with no filters will have all news");
        hint2->setText("Choose feed to add, edit or delete its filters");
        setWindowTitle("RSS-feeds and filters");
        break;
    default:
        break;
    }
    */
    v_layout = new QVBoxLayout;
    //v_layout->addLayout(h1_layout);
    v_layout->addWidget(feedList);
    v_layout->addWidget(hint1);
    //v_layout->addLayout(vb_layout);
    v_layout->addWidget(filterList);
    v_layout->addWidget(hint2);

#ifdef Q_OS_ANDROID
    close_fnf_button = new QPushButton(tr("Закрыть"), this);
    connect(close_fnf_button, SIGNAL(clicked()), this, SLOT(close())); // запуск по нажатию кнопки
    v_layout->addWidget(close_fnf_button);
#endif

    setLayout(v_layout); // установка главного лэйаута

#ifdef Q_OS_WIN32
    resize(640,480);
#endif
    connect(feedList, SIGNAL(clicked(QModelIndex)), this, SLOT(UpdateFilter(QModelIndex)));
    connect(feedList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ShowEditFeed(QModelIndex)));
    connect(filterList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(ShowEditFilter(QModelIndex)));
    /*
    connect(filterAdd, SIGNAL(clicked(bool)), this, SLOT(ShowAddFilter()));
    connect(feedAdd, SIGNAL(clicked(bool)), this, SLOT(ShowAddFeed()));
    connect(filterEdit, SIGNAL(clicked(bool)), this, SLOT(ShowEditFilter()));
    connect(feedEdit, SIGNAL(clicked(bool)), this, SLOT(ShowEditFeed()));
    connect(filterDel, SIGNAL(clicked(bool)), this, SLOT(FilterDel()));
    connect(feedDel, SIGNAL(clicked(bool)), this, SLOT(FeedDel()));
    connect(filterCheckAll, SIGNAL(clicked(bool)), this, SLOT(filters_check_all()));
    connect(filterUncheckAll, SIGNAL(clicked(bool)), this, SLOT(filters_uncheck_all()));
    connect(filterDeleteAll, SIGNAL(clicked(bool)), this, SLOT(filters_delete_all()));
    */
    this->setWindowIcon(QIcon(":/img/rss.ico"));

    // устанавливаем наш обработчик событий
    installEventFilter(this);
    feedList->installEventFilter(this);
    filterList->installEventFilter(this);
}

FeedsAndFilters::~FeedsAndFilters()
{
    delete feedModel;
    delete filterModel;
    delete model2;
    delete feedList;
    delete filterList;
    delete v_layout;
    delete editW;
    delete hint1;
    delete hint2;
    delete feeds_menu;
    delete filters_menu;
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
    /*
    QModelIndex mindex = feedList->selectionModel()->selectedIndexes().first();
    ShowEditFeed(mindex);
    */
    QModelIndexList selected = feedList->selectionModel()->selectedIndexes();
    foreach (QModelIndex mindex, selected)
    {
        if (mindex.column() == 0) // иначе берёт каждую строку столько раз, сколько колонок в дереве
            ShowEditFeed(mindex);
    }
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
    /*
    QModelIndex mindex =  filterList->selectionModel()->selectedIndexes().first();
    ShowEditFilter(mindex);
    */
    QModelIndexList selected = filterList->selectionModel()->selectedIndexes();
    foreach (QModelIndex mindex, selected)
    {
        if (mindex.column() == 0) // иначе берёт каждую строку столько раз, сколько колонок в дереве
            ShowEditFilter(mindex);
    }
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
    /*
    QModelIndex mindex =  feedModel->index(feedList->selectionModel()->selectedIndexes().first().row(), 3);
    pFeeds->DeleteFeedById(feedModel->data(mindex, Qt::DisplayRole).toInt());
    updateFeeds();
    updateFilters();
    */
    QModelIndexList selected = feedList->selectionModel()->selectedIndexes();
    foreach (QModelIndex mindex, selected)
    {
        pFeeds->DeleteFeedById(feedModel->data(mindex, Qt::DisplayRole).toInt());
    }
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

void FeedsAndFilters::slot_feeds_menu()
{
    updateFilters();

    if (!feeds_menu)
    {
        feeds_menu = new QMenu;
        // добавить
        feeds_menu_add = new QAction(this);
        //feeds_menu_add->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_add);
        connect(feeds_menu_add, SIGNAL(triggered()), this, SLOT(ShowAddFeed()));
        // редактировать
        feeds_menu_edit = new QAction(this);
        //feeds_menu_edit->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_edit);
        connect(feeds_menu_edit, SIGNAL(triggered()), this, SLOT(ShowEditFeed()));
        // удалить
        feeds_menu_del = new QAction(this);
        //feeds_menu_del->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_del);
        connect(feeds_menu_del, SIGNAL(triggered()), this, SLOT(FeedDel()));
        // разделитель
        feeds_menu->insertSeparator(0);
        // добавить фильтр(ы)
        feeds_menu_add_filters = new QAction(this);
        //feeds_menu_add_filters->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_add_filters);
        connect(feeds_menu_add_filters, SIGNAL(triggered()), this, SLOT(ShowAddFilter()));
        // удалить все фильтры
        feeds_menu_del_all_filters = new QAction(this);
        //feeds_menu_del_all_filters->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_del_all_filters);
        connect(feeds_menu_del_all_filters, SIGNAL(triggered()), this, SLOT(filters_delete_all()));
        // включить все фильтры
        feeds_menu_on_all_filters = new QAction(this);
        //feeds_menu_on_all_filters->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_on_all_filters);
        connect(feeds_menu_on_all_filters, SIGNAL(triggered()), this, SLOT(filters_check_all()));
        // выключить все фильтры
        feeds_menu_off_all_filters = new QAction(this);
        //feeds_menu_off_all_filters->setIcon(QIcon(":/img/settings.ico"));
        feeds_menu->addAction(feeds_menu_off_all_filters);
        connect(feeds_menu_off_all_filters, SIGNAL(triggered()), this, SLOT(filters_uncheck_all()));
    }
    switch (settings->current_language)
    {
    case 1:
        feeds_menu_add->setText("Добавить");
        feeds_menu_edit->setText("Редактировать");
        feeds_menu_del->setText("Удалить");
        feeds_menu_add_filters->setText("Добавить фильтр(ы)");
        feeds_menu_del_all_filters->setText("Удалить все фильтры");
        feeds_menu_on_all_filters->setText("Включить все фильтры");
        feeds_menu_off_all_filters->setText("Выключить все фильтры");
        break;
    case 2:
        feeds_menu_add->setText("Add");
        feeds_menu_edit->setText("Edit");
        feeds_menu_del->setText("Delete");
        feeds_menu_add_filters->setText("Add filter(s)");
        feeds_menu_del_all_filters->setText("Delete all filters");
        feeds_menu_on_all_filters->setText("Enable all filters");
        feeds_menu_off_all_filters->setText("Disable all filters");
        break;
    default:
        break;
    }
    feeds_menu->exec(QCursor::pos());
    /*
#ifdef Q_OS_WIN32
    feeds_menu->exec(QCursor::pos());
#endif
#ifdef Q_OS_ANDROID
    QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
    QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
    if (touchPoints.count() == 1)
    {
        feeds_menu->exec(touchPoints.first().pos());
     }
#endif
*/
}

void FeedsAndFilters::slot_filters_menu()
{
    if (!filters_menu)
    {
        filters_menu = new QMenu;
        // добавить
        filters_menu_add = new QAction(this);
        //feeds_menu_add->setIcon(QIcon(":/img/settings.ico"));
        filters_menu->addAction(filters_menu_add);
        connect(filters_menu_add, SIGNAL(triggered()), this, SLOT(ShowAddFilter()));
        // редактировать
        filters_menu_edit = new QAction(this);
        //filters_menu_edit->setIcon(QIcon(":/img/settings.ico"));
        filters_menu->addAction(filters_menu_edit);
        connect(filters_menu_edit, SIGNAL(triggered()), this, SLOT(ShowEditFilter()));
        // удалить
        filters_menu_del = new QAction(this);
        //filters_menu_del->setIcon(QIcon(":/img/settings.ico"));
        filters_menu->addAction(filters_menu_del);
        connect(filters_menu_del, SIGNAL(triggered()), this, SLOT(FilterDel()));
        // разделитель
        filters_menu->insertSeparator(0);
        // удалить все фильтры
        filters_menu_del_all = new QAction(this);
        //filters_menu_del_all->setIcon(QIcon(":/img/settings.ico"));
        filters_menu->addAction(filters_menu_del_all);
        connect(filters_menu_del_all, SIGNAL(triggered()), this, SLOT(filters_delete_all()));
        // включить все фильтры
        filters_menu_on_all = new QAction(this);
        //filters_menu_on_all->setIcon(QIcon(":/img/settings.ico"));
        filters_menu->addAction(filters_menu_on_all);
        connect(filters_menu_on_all, SIGNAL(triggered()), this, SLOT(filters_check_all()));
        // выключить все фильтры
        filters_menu_off_all = new QAction(this);
        //filters_menu_off_all->setIcon(QIcon(":/img/settings.ico"));
        filters_menu->addAction(filters_menu_off_all);
        connect(filters_menu_off_all, SIGNAL(triggered()), this, SLOT(filters_uncheck_all()));
    }
    switch (settings->current_language)
    {
    case 1:
        filters_menu_add->setText("Добавить");
        filters_menu_edit->setText("Редактировать");
        filters_menu_del->setText("Удалить");
        filters_menu_del_all->setText("Удалить все");
        filters_menu_on_all->setText("Включить все");
        filters_menu_off_all->setText("Выключить все");
        break;
    case 2:
        filters_menu_add->setText("Add");
        filters_menu_edit->setText("Edit");
        filters_menu_del->setText("Delete");
        filters_menu_del_all->setText("Delete all");
        filters_menu_on_all->setText("Enable all");
        filters_menu_off_all->setText("Disable all");
        break;
    default:
        break;
    }
    filters_menu->exec(QCursor::pos());
}
/*
void FeedsAndFilters::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    settings->timer->start(); // закрыли окно - запустили таймер
}

void FeedsAndFilters::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    settings->timer->stop(); // останавливаем таймер на время работы с окном
    switch (settings->current_language)
    {
    case 1:
        hint1->setText("В ленте, не имеющей фильтров, будут выведены все новости");
        hint2->setText("Работа с фильтрами доступна только после выбора ленты");
        setWindowTitle("RSS-ленты и фильтры");
        break;
    case 2:
        hint1->setText("Feed with no filters will have all news");
        hint2->setText("Choose feed to work with filters");
        setWindowTitle("RSS-feeds and filters");
        break;
    default:
        break;
    }
}
*/
// обработчик событий сворачивания/разворачивания
bool FeedsAndFilters::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Show && obj != NULL)
    {
        Q_UNUSED(event);
        settings->timer->stop(); // останавливаем таймер на время работы с окном
        switch (settings->current_language)
        {
        case 1:
            hint1->setText("В ленте, не имеющей фильтров, будут выведены все новости");
            hint2->setText("Работа с фильтрами доступна только после выбора ленты");
            setWindowTitle("RSS-ленты и фильтры");
            break;
        case 2:
            hint1->setText("Feed with no filters will have all news");
            hint2->setText("Choose feed to work with filters");
            setWindowTitle("RSS-feeds and filters");
            break;
        default:
            break;
        }
    }

    if (event->type() == QEvent::Close && obj != NULL)
    {
        Q_UNUSED(event);
        settings->timer->start(); // закрыли окно - запустили таймер
    }

    return false;
}

// обработка пальценажатий
#ifdef Q_OS_ANDROID
void FeedsAndFilters::grabGestures(const QList<Qt::GestureType> &gestures)
{
    foreach (Qt::GestureType gesture, gestures)
        grabGesture(gesture);
}

bool FeedsAndFilters::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        return gestureEvent(static_cast<QGestureEvent*>(event));
    return QWidget::event(event);
}

bool FeedsAndFilters::gestureEvent(QGestureEvent *event)
{
    if (QGesture *tap_and_hold = event->gesture(Qt::TapAndHoldGesture))
        tap_and_holdTriggered(static_cast<QTapAndHoldGesture *>(tap_and_hold));
    return true;
}

void FeedsAndFilters::tap_and_holdTriggered(QTapAndHoldGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished)
    {
        /*
        if (gesture->horizontalDirection() == QSwipeGesture::Left
            || gesture->verticalDirection() == QSwipeGesture::Up) {
            qCDebug(lcExample) << "swipeTriggered(): swipe to previous";
            goPrevImage();
        } else {
            qCDebug(lcExample) << "swipeTriggered(): swipe to next";
            goNextImage();
        }
        */
        //slot_feeds_menu();
        close();
        //update();
    }
}
#endif
