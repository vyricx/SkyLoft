#include "InjectorWidget.h"
#include "../core/injector/IInjector.h"

#ifdef _WIN32
#include "../core/injector/WindowsInjector.h"
#else
#include "../core/injector/LinuxInjector.h"
#endif

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <filesystem>

InjectorWidget::InjectorWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(12);
    layout->setContentsMargins(16, 16, 16, 16);

    m_processLabel = new QLabel("No process selected", this);
    m_processLabel->setStyleSheet("color: #888; font-style: italic;");
    layout->addWidget(m_processLabel);

    auto* dllRow = new QHBoxLayout();
    m_dllPathEdit = new QLineEdit(this);
    m_dllPathEdit->setPlaceholderText("Path to .dll file...");
    m_browseBtn = new QPushButton("Browse", this);
    dllRow->addWidget(m_dllPathEdit);
    dllRow->addWidget(m_browseBtn);
    layout->addLayout(dllRow);

    auto* btnRow = new QHBoxLayout();
    m_injectBtn = new QPushButton("Inject", this);
    m_ejectBtn  = new QPushButton("Eject", this);
    m_injectBtn->setEnabled(false);
    m_ejectBtn->setEnabled(false);
    btnRow->addWidget(m_injectBtn);
    btnRow->addWidget(m_ejectBtn);
    layout->addLayout(btnRow);

    layout->addStretch();

    connect(m_browseBtn, &QPushButton::clicked, this, &InjectorWidget::onBrowseDll);
    connect(m_injectBtn, &QPushButton::clicked, this, &InjectorWidget::onInject);
    connect(m_ejectBtn,  &QPushButton::clicked, this, &InjectorWidget::onEject);
}

void InjectorWidget::setSelectedProcess(const GameProcess& proc) {
    m_currentProcess = proc;
    m_hasProcess = true;

    QString label = QString("[%1] %2 (%3)")
        .arg(proc.pid)
        .arg(QString::fromStdString(proc.name))
        .arg(proc.is64Bit ? "64-bit" : "32-bit");
    m_processLabel->setText(label);
    m_processLabel->setStyleSheet("color: #ddd; font-weight: bold;");

    m_injectBtn->setEnabled(true);
    m_ejectBtn->setEnabled(true);
}

void InjectorWidget::clearProcess() {
    m_hasProcess = false;
    m_processLabel->setText("No process selected");
    m_processLabel->setStyleSheet("color: #888; font-style: italic;");
    m_injectBtn->setEnabled(false);
    m_ejectBtn->setEnabled(false);
}

void InjectorWidget::onBrowseDll() {
    QString path = QFileDialog::getOpenFileName(
        this, "Select DLL", QString(),
        "Dynamic Libraries (*.dll);;All Files (*)");
    if (!path.isEmpty())
        m_dllPathEdit->setText(path);
}

void InjectorWidget::onInject() {
    if (!m_hasProcess) return;

    QString dllPath = m_dllPathEdit->text().trimmed();
    if (dllPath.isEmpty()) {
        emit statusMessage("No DLL path specified.", false);
        return;
    }

#ifdef _WIN32
    WindowsInjector injector;
#else
    LinuxInjector injector;
#endif

    bool ok = injector.inject(m_currentProcess.pid, dllPath.toStdString());
    if (ok) {
        emit statusMessage(
            QString("Injected into %1 (PID %2)")
                .arg(QString::fromStdString(m_currentProcess.name))
                .arg(m_currentProcess.pid),
            true);
    } else {
        emit statusMessage(
            QString("Injection failed: %1")
                .arg(QString::fromStdString(injector.lastError())),
            false);
    }
}

void InjectorWidget::onEject() {
    if (!m_hasProcess) return;

    QString dllPath = m_dllPathEdit->text().trimmed();
    if (dllPath.isEmpty()) {
        emit statusMessage("No DLL path specified.", false);
        return;
    }

    std::filesystem::path p(dllPath.toStdString());
    std::string dllName = p.filename().string();

#ifdef _WIN32
    WindowsInjector injector;
#else
    LinuxInjector injector;
#endif

    bool ok = injector.eject(m_currentProcess.pid, dllName);
    if (ok) {
        emit statusMessage(
            QString("Ejected %1 from %2")
                .arg(QString::fromStdString(dllName))
                .arg(QString::fromStdString(m_currentProcess.name)),
            true);
    } else {
        emit statusMessage(
            QString("Eject failed: %1")
                .arg(QString::fromStdString(injector.lastError())),
            false);
    }
}
