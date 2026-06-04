#pragma once
#include <QWidget>
#include "../core/process/GameProcess.h"

class QLabel;
class QLineEdit;
class QPushButton;

class InjectorWidget : public QWidget {
    Q_OBJECT

public:
    explicit InjectorWidget(QWidget* parent = nullptr);

    void setSelectedProcess(const GameProcess& proc);
    void clearProcess();

signals:
    void statusMessage(const QString& msg, bool success);

private slots:
    void onBrowseDll();
    void onInject();
    void onEject();

private:
    QLabel*      m_processLabel;
    QLineEdit*   m_dllPathEdit;
    QPushButton* m_browseBtn;
    QPushButton* m_injectBtn;
    QPushButton* m_ejectBtn;

    GameProcess m_currentProcess{};
    bool        m_hasProcess = false;
};
