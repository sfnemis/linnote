#include "PortalShortcuts.h"
#include "core/Settings.h"
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include <QDebug>
#include <QPair>
#include <QUuid>

// Type alias for shortcut: (id, properties)
using Shortcut = QPair<QString, QVariantMap>;
using ShortcutList = QList<Shortcut>;

// Custom D-Bus marshalling for Shortcut (sa{sv})
QDBusArgument &operator<<(QDBusArgument &arg, const Shortcut &shortcut) {
  arg.beginStructure();
  arg << shortcut.first;  // QString (s)
  arg << shortcut.second; // QVariantMap (a{sv})
  arg.endStructure();
  return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Shortcut &shortcut) {
  arg.beginStructure();
  arg >> shortcut.first;
  arg >> shortcut.second;
  arg.endStructure();
  return arg;
}

const QString PortalShortcuts::PORTAL_SERVICE =
    QStringLiteral("org.freedesktop.portal.Desktop");
const QString PortalShortcuts::PORTAL_PATH =
    QStringLiteral("/org/freedesktop/portal/desktop");
const QString PortalShortcuts::PORTAL_INTERFACE =
    QStringLiteral("org.freedesktop.portal.GlobalShortcuts");
const QString PortalShortcuts::TOGGLE_SHORTCUT_ID =
    QStringLiteral("toggle-window");

PortalShortcuts::PortalShortcuts(QObject *parent)
    : QObject(parent), m_portalInterface(nullptr), m_available(false) {
  // Register DBus types
  qDBusRegisterMetaType<QVariantMap>();
  qDBusRegisterMetaType<Shortcut>();
  qDBusRegisterMetaType<ShortcutList>();

  // Create portal interface
  m_portalInterface =
      new QDBusInterface(PORTAL_SERVICE, PORTAL_PATH, PORTAL_INTERFACE,
                         QDBusConnection::sessionBus(), this);

  m_available = m_portalInterface->isValid();

  if (!m_available) {
    qWarning() << "GlobalShortcuts portal not available. "
               << "Make sure xdg-desktop-portal 1.16+ is installed.";
  }
}

PortalShortcuts::~PortalShortcuts() = default;

bool PortalShortcuts::isAvailable() const { return m_available; }

void PortalShortcuts::registerShortcuts() {
  if (!m_available) {
    qWarning() << "Cannot register shortcuts: portal not available";
    return;
  }

  createSession();
}

void PortalShortcuts::createSession() {
  // Generate unique tokens
  QString uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
  m_requestToken = QStringLiteral("linnote_") + uniqueId;
  QString sessionToken = QStringLiteral("linnote_session_") + uniqueId;

  // Build options
  QVariantMap options;
  options["handle_token"] = m_requestToken;
  options["session_handle_token"] = sessionToken;

  // Get the sender name from DBus connection
  QString senderName = QDBusConnection::sessionBus().baseService();
  // Convert :1.123 to 1_123 format
  QString senderPath = senderName;
  senderPath.remove(0, 1); // Remove leading ':'
  senderPath.replace('.', '_');

  // Connect to response signal before making the call
  QString responsePath =
      QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
          .arg(senderPath)
          .arg(m_requestToken);

  qDebug() << "Portal: Creating session...";
  qDebug() << "Portal: Response path:" << responsePath;

  bool connected = QDBusConnection::sessionBus().connect(
      PORTAL_SERVICE, responsePath, "org.freedesktop.portal.Request",
      "Response", this, SLOT(onSessionCreated(uint, QVariantMap)));

  qDebug() << "Portal: Signal connection result:" << connected;

  // Call CreateSession
  QDBusMessage msg = m_portalInterface->call("CreateSession", options);

  if (msg.type() == QDBusMessage::ErrorMessage) {
    qWarning() << "CreateSession failed:" << msg.errorMessage();
  } else {
    qDebug() << "Portal: CreateSession call succeeded, waiting for response...";
  }
}

void PortalShortcuts::onSessionCreated(uint response,
                                       const QVariantMap &results) {
  if (response != 0) {
    qWarning() << "CreateSession failed with response:" << response;
    return;
  }

  m_sessionHandle = results.value("session_handle").toString();
  qDebug() << "Session created:" << m_sessionHandle;

  // Now bind shortcuts
  bindShortcuts();
}

void PortalShortcuts::bindShortcuts() {
  if (m_sessionHandle.isEmpty()) {
    qWarning() << "Cannot bind shortcuts: no session";
    return;
  }

  // Generate new request token
  QString uniqueId = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
  m_requestToken = QStringLiteral("linnote_bind_") + uniqueId;

  // Build options
  QVariantMap options;
  options["handle_token"] = m_requestToken;

  // Connect to response signal
  QString senderName = QDBusConnection::sessionBus().baseService();
  QString senderPath = senderName;
  senderPath.remove(0, 1);
  senderPath.replace('.', '_');

  QString responsePath =
      QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2")
          .arg(senderPath)
          .arg(m_requestToken);

  QDBusConnection::sessionBus().connect(
      PORTAL_SERVICE, responsePath, "org.freedesktop.portal.Request",
      "Response", this, SLOT(onShortcutsBound(uint, QVariantMap)));

  // Connect to Activated signal on the session
  QDBusConnection::sessionBus().connect(
      PORTAL_SERVICE, m_sessionHandle, PORTAL_INTERFACE, "Activated", this,
      SLOT(onActivated(QDBusObjectPath, QString, qlonglong, QVariantMap)));

  // Build shortcuts array with correct D-Bus format: a(sa{sv})
  QString hotkey = Settings::instance()->globalHotkey();

  ShortcutList shortcuts;
  QVariantMap props;
  props["description"] = tr("Toggle LinNote window");
  props["preferred_trigger"] = hotkey;
  shortcuts.append(qMakePair(TOGGLE_SHORTCUT_ID, props));

  // Call BindShortcuts with correct signature
  QDBusMessage msg = QDBusMessage::createMethodCall(
      PORTAL_SERVICE, PORTAL_PATH, PORTAL_INTERFACE, "BindShortcuts");

  // Arguments: session_handle (o), shortcuts a(sa{sv}), parent_window (s),
  // options (a{sv})
  msg << QDBusObjectPath(m_sessionHandle);
  msg << QVariant::fromValue(shortcuts);
  msg << QString(); // parent_window (empty for no parent)
  msg << options;

  QDBusMessage reply = QDBusConnection::sessionBus().call(msg);

  if (reply.type() == QDBusMessage::ErrorMessage) {
    qWarning() << "BindShortcuts failed:" << reply.errorMessage();
  } else {
    qDebug() << "BindShortcuts call succeeded";
  }
}

void PortalShortcuts::onShortcutsBound(uint response,
                                       const QVariantMap &results) {
  if (response != 0) {
    qWarning() << "BindShortcuts failed with response:" << response;
    return;
  }

  qDebug() << "Shortcuts bound successfully";
  Q_UNUSED(results);
}

void PortalShortcuts::onActivated(const QDBusObjectPath &sessionHandle,
                                  const QString &shortcutId,
                                  qlonglong timestamp,
                                  const QVariantMap &options) {
  Q_UNUSED(sessionHandle);
  Q_UNUSED(timestamp);
  Q_UNUSED(options);

  qDebug() << "Shortcut activated:" << shortcutId;

  if (shortcutId == TOGGLE_SHORTCUT_ID) {
    emit toggleRequested();
  }
}
