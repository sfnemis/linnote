#ifndef LINNOTE_DBUSSERVICE_H
#define LINNOTE_DBUSSERVICE_H

#include <QDBusConnection>
#include <QObject>

class MainWindow;

/**
 * @brief DBus service for external control of LinNote
 *
 * Exposes Toggle method that can be called from:
 * - KDE custom shortcuts
 * - Command line: dbus-send --session --type=method_call \
 *     --dest=org.example.LinNote /LinNote org.example.LinNote.Toggle
 */
class DBusService : public QObject {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.example.LinNote")

public:
  explicit DBusService(MainWindow *mainWindow, QObject *parent = nullptr);

  bool registerService();

public slots:
  /**
   * @brief Toggle window visibility (D-Bus callable)
   */
  Q_SCRIPTABLE void Toggle();

  /**
   * @brief Show window (D-Bus callable)
   */
  Q_SCRIPTABLE void Show();

  /**
   * @brief Hide window (D-Bus callable)
   */
  Q_SCRIPTABLE void Hide();

  /**
   * @brief Create new note with optional text (D-Bus callable)
   */
  Q_SCRIPTABLE void NewNote(const QString &text = QString());

  /**
   * @brief Search notes (D-Bus callable)
   */
  Q_SCRIPTABLE void Search(const QString &query);

  /**
   * @brief Open linnote:// URL (D-Bus callable)
   */
  Q_SCRIPTABLE void OpenUrl(const QString &url);

private:
  MainWindow *m_mainWindow;
  void parseAndExecuteUrl(const QString &url);
};

#endif // LINNOTE_DBUSSERVICE_H
