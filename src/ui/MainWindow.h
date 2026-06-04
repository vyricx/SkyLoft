#pragma once
#include <QMainWindow>
#include <QTimer>
#include <vector>
#include "../core/process/GameProcess.h"

class QListWidget;
class QListWidgetItem;
class QStatusBar;
class InjectorWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void refreshProcesses();
    void onProcessSelected(QListWidgetItem* item);
    void onStatusMessage(const QString& msg, bool success);

private:
    QListWidget*    m_processList;
    InjectorWidget* m_injectorWidget;
    QTimer*         m_refreshTimer;

    std::vector<GameProcess> m_processes;
};
