#ifndef LINNOTE_PORTALSHORTCUTS_H
#define LINNOTE_PORTALSHORTCUTS_H

#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QObject>
#include <QVariantMap>

/**
 * @brief XDG Desktop Portal GlobalShortcuts integration
 *
 * Uses org.freedesktop.portal.GlobalShortcuts to register
 * system-wide keyboard shortcuts on Wayland.
 *
 * Requires: xdg-desktop-portal 1.16+ with GlobalShortcuts support
 * Supported: KDE Plasma 6+
 */
class PortalShortcuts : public QObject {
  Q_OBJECT

public:
  explicit PortalShortcuts(QObject *parent = nullptr);
  ~PortalShortcuts() override;

  /**
   * @brief Register global shortcuts with the portal
   *
   * Creates a session and binds shortcuts. On success,
   * toggleRequested signal will be emitted when shortcut is activated.
   */
  void registerShortcuts();

  /**
   * @brief Check if portal is available
   */
  bool isAvailable() const;

signals:
  /**
   * @brief Emitted when the toggle shortcut is activated
   */
  void toggleRequested();

private slots:
  void onSessionCreated(uint response, const QVariantMap &results);
  void onShortcutsBound(uint response, const QVariantMap &results);
  void onActivated(const QDBusObjectPath &sessionHandle,
                   const QString &shortcutId, qlonglong timestamp,
                   const QVariantMap &options);

private:
  void createSession();
  void bindShortcuts();

  QDBusInterface *m_portalInterface;
  QString m_sessionHandle;
  QString m_requestToken;
  bool m_available;

  static const QString PORTAL_SERVICE;
  static const QString PORTAL_PATH;
  static const QString PORTAL_INTERFACE;
  static const QString TOGGLE_SHORTCUT_ID;
};

#endif // LINNOTE_PORTALSHORTCUTS_H
