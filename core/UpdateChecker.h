#ifndef LINNOTE_UPDATECHECKER_H
#define LINNOTE_UPDATECHECKER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

/**
 * @brief Checks for application updates via GitHub API
 *
 * Uses GitHub Releases API to check if a newer version is available.
 * Compares current version (from Settings::appVersion()) with latest release
 * tag.
 */
class UpdateChecker : public QObject {
  Q_OBJECT

public:
  explicit UpdateChecker(QObject *parent = nullptr);

  // Check for updates (async)
  void checkForUpdates();

  // Get last check results
  QString latestVersion() const;
  QString changelog() const;
  QString downloadUrl() const;
  bool updateAvailable() const;

  // Trigger update (runs install.sh --update)
  void performUpdate();

  // Version comparison helper
  static bool isNewerVersion(const QString &current, const QString &latest);

signals:
  void updateCheckStarted();
  void updateFound(const QString &version, const QString &changelog);
  void noUpdateFound();
  void updateCheckFailed(const QString &error);
  void updateStarted();
  void updateFinished(bool success);

private slots:
  void onCheckFinished(QNetworkReply *reply);

private:
  QNetworkAccessManager *m_networkManager;
  QString m_latestVersion;
  QString m_changelog;
  QString m_downloadUrl;
  bool m_updateAvailable;
};

#endif // LINNOTE_UPDATECHECKER_H
