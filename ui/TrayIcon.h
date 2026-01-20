#ifndef LINNOTE_TRAYICON_H
#define LINNOTE_TRAYICON_H

#include <QMenu>
#include <QSystemTrayIcon>

class MainWindow;

/**
 * @brief System tray icon with context menu
 *
 * Provides:
 * - Show/Hide toggle
 * - Export action
 * - Quit action
 * - Double-click to toggle window
 */
class TrayIcon : public QSystemTrayIcon {
  Q_OBJECT

public:
  explicit TrayIcon(MainWindow *mainWindow, QObject *parent = nullptr);
  ~TrayIcon() override;

private slots:
  void onActivated(QSystemTrayIcon::ActivationReason reason);
  void updateShowHideAction();

private:
  void setupMenu();

  MainWindow *m_mainWindow;
  QMenu *m_menu;
  QAction *m_showHideAction;
};

#endif // LINNOTE_TRAYICON_H
