#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "desktop.h"

#include <QProcessEnvironment>
#include <QHeaderView>

void MainWindow::InitializeTable() {
  QTableWidget *table = new QTableWidget();
  table->setObjectName("table");
  this->setCentralWidget(table);
  QStringList labels = {"Name", "Address", "Computer Name", "Stub Name", "Active Window Title"};
  table->setColumnCount(labels.size());
  table->setHorizontalHeaderLabels(labels);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->horizontalHeader()->setStretchLastSection(true);
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(table, &QWidget::customContextMenuRequested, this, &MainWindow::tableContextMenuRequested);
  this->table = table;
}

void MainWindow::tableContextMenuRequested(const QPoint &pos) {
    this->selectedClientName = this->table->itemAt(QPoint(0, pos.y()))->text();
    QMenu *menu = new QMenu(this->table);
    QAction *desktopAction = new QAction("Desktop");
    connect(desktopAction, &QAction::triggered, this, &MainWindow::desktopRequested);
    menu->addAction(desktopAction);
    menu->exec(QCursor::pos());
}

void MainWindow::desktopRequested() {
    Desktop *desktop = new Desktop(this->selectedClientName, qgetenv("QTSERPENTINE_API_ADDRESS"), this);
    desktop->show();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  this->InitializeTable();
  this->clientFetcher = new ClientFetcher(qgetenv("QTSERPENTINE_API_ADDRESS"), this->table, this);
}

MainWindow::~MainWindow() { delete ui; }
