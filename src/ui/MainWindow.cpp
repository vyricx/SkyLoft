#include "MainWindow.h"
#include "InjectorWidget.h"
#include "../core/process/ProcessScanner.h"

#include <QListWidget>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QLabel>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("SkyLoft");
    setMinimumSize(800, 520);

    auto* toolbar = addToolBar("Main");
    toolbar->setMovable(false);
    auto* refreshAction = toolbar->addAction("Refresh Processes");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshProcesses);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    m_processList = new QListWidget(splitter);
    m_processList->setMinimumWidth(280);
    splitter->addWidget(m_processList);

    m_injectorWidget = new InjectorWidget(splitter);
    splitter->addWidget(m_injectorWidget);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    connect(m_processList, &QListWidget::itemClicked,
            this, &MainWindow::onProcessSelected);
    connect(m_injectorWidget, &InjectorWidget::statusMessage,
            this, &MainWindow::onStatusMessage);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(5000);
    connect(m_refreshTimer, &QTimer::timeout, this, &MainWindow::refreshProcesses);
    m_refreshTimer->start();

    refreshProcesses();
}

void MainWindow::refreshProcesses() {
    int currentRow = m_processList->currentRow();

    m_processes = ProcessScanner::scan();
    m_processList->clear();

    for (const auto& proc : m_processes) {
        QString label = QString("[%1] %2")
            .arg(proc.pid)
            .arg(QString::fromStdString(proc.name));
        m_processList->addItem(label);
    }

    if (currentRow >= 0 && currentRow < m_processList->count())
        m_processList->setCurrentRow(currentRow);
    else
        m_injectorWidget->clearProcess();
}

void MainWindow::onProcessSelected(QListWidgetItem* item) {
    int row = m_processList->row(item);
    if (row >= 0 && row < static_cast<int>(m_processes.size()))
        m_injectorWidget->setSelectedProcess(m_processes[row]);
}

void MainWindow::onStatusMessage(const QString& msg, bool success) {
    statusBar()->setStyleSheet(
        success ? "color: #4caf50;" : "color: #f44336;");
    statusBar()->showMessage(msg, 5000);
}
