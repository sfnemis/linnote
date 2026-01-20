#include "DBusService.h"
#include "ui/MainWindow.h"
#include <QDebug>
#include <QUrl>
#include <QUrlQuery>

DBusService::DBusService(MainWindow *mainWindow, QObject *parent)
    : QObject(parent), m_mainWindow(mainWindow) {}

bool DBusService::registerService() {
  QDBusConnection sessionBus = QDBusConnection::sessionBus();

  // Register service name
  if (!sessionBus.registerService("org.linnote.LinNote")) {
    qWarning() << "DBus: Failed to register service name";
    return false;
  }

  // Register object
  if (!sessionBus.registerObject("/LinNote", this,
                                 QDBusConnection::ExportScriptableSlots)) {
    qWarning() << "DBus: Failed to register object";
    return false;
  }

  qDebug() << "DBus: Service registered successfully";
  qDebug() << "DBus: Call with: dbus-send --session --type=method_call "
              "--dest=org.linnote.LinNote /LinNote org.linnote.LinNote.Toggle";

  return true;
}

void DBusService::Toggle() {
  qDebug() << "DBus: Toggle called";
  if (m_mainWindow) {
    m_mainWindow->toggleVisibility();
  }
}

void DBusService::Show() {
  qDebug() << "DBus: Show called";
  if (m_mainWindow) {
    m_mainWindow->showAndFocus();
  }
}

void DBusService::Hide() {
  qDebug() << "DBus: Hide called";
  if (m_mainWindow) {
    m_mainWindow->hideWindow();
  }
}

void DBusService::NewNote(const QString &text) {
  qDebug() << "DBus: NewNote called with text:" << text;
  if (m_mainWindow) {
    m_mainWindow->showAndFocus();
    m_mainWindow->createNewNoteWithText(text);
  }
}

void DBusService::Search(const QString &query) {
  qDebug() << "DBus: Search called with query:" << query;
  if (m_mainWindow) {
    m_mainWindow->showAndFocus();
    m_mainWindow->searchNotes(query);
  }
}

void DBusService::OpenUrl(const QString &url) {
  qDebug() << "DBus: OpenUrl called with:" << url;
  parseAndExecuteUrl(url);
}

void DBusService::parseAndExecuteUrl(const QString &urlString) {
  QUrl url(urlString);

  if (url.scheme() != "linnote") {
    qWarning() << "DBus: Invalid URL scheme, expected 'linnote'";
    return;
  }

  QString action = url.host();
  QUrlQuery query(url);

  qDebug() << "DBus: URL action:" << action;

  if (action == "toggle") {
    Toggle();
  } else if (action == "show") {
    Show();
  } else if (action == "hide") {
    Hide();
  } else if (action == "new") {
    QString text = query.queryItemValue("text", QUrl::FullyDecoded);
    NewNote(text);
  } else if (action == "search") {
    QString q = query.queryItemValue("q", QUrl::FullyDecoded);
    Search(q);
  } else {
    qWarning() << "DBus: Unknown action:" << action;
  }
}
