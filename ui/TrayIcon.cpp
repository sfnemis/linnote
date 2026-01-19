#include "TrayIcon.h"
#include "MainWindow.h"
#include <QApplication>

TrayIcon::TrayIcon(MainWindow *mainWindow, QObject *parent)
    : QSystemTrayIcon(parent), m_mainWindow(mainWindow), m_menu(nullptr),
      m_showHideAction(nullptr) {
  // Use our custom penguin icon
  setIcon(QIcon(":/app/icons/app/linnote-32.png"));
  setToolTip(tr("LinNote - Quick Scratchpad"));

  setupMenu();

  // Handle tray icon activation
  connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);

  // Update show/hide action text when window visibility changes
  connect(m_mainWindow, &QMainWindow::windowTitleChanged, this,
          &TrayIcon::updateShowHideAction);
}

TrayIcon::~TrayIcon() { delete m_menu; }

void TrayIcon::setupMenu() {
  m_menu = new QMenu();

  // Show/Hide action
  m_showHideAction = m_menu->addAction(tr("Show"));
  connect(m_showHideAction, &QAction::triggered, m_mainWindow,
          &MainWindow::toggleVisibility);

  m_menu->addSeparator();

  // OCR action
  QAction *ocrAction = m_menu->addAction(tr("📷 OCR Al"));
  connect(ocrAction, &QAction::triggered, m_mainWindow,
          &MainWindow::captureOcr);

  // Export action
  QAction *exportAction = m_menu->addAction(tr("Export..."));
  connect(exportAction, &QAction::triggered, m_mainWindow,
          &MainWindow::exportNote);

  // Settings action
  QAction *settingsAction = m_menu->addAction(tr("⚙️ Settings"));
  connect(settingsAction, &QAction::triggered, m_mainWindow,
          &MainWindow::openSettingsDialog);

  m_menu->addSeparator();

  // Quit action
  QAction *quitAction = m_menu->addAction(tr("Quit"));
  connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

  setContextMenu(m_menu);
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
  switch (reason) {
  case QSystemTrayIcon::Trigger:     // Single click
  case QSystemTrayIcon::DoubleClick: // Double click
    m_mainWindow->toggleVisibility();
    break;
  case QSystemTrayIcon::MiddleClick:
    // Could be used for quick paste in the future
    break;
  default:
    break;
  }
}

void TrayIcon::updateShowHideAction() {
  if (m_mainWindow->isVisible()) {
    m_showHideAction->setText(tr("Hide"));
  } else {
    m_showHideAction->setText(tr("Show"));
  }
}
