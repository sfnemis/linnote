#include "UpdateChecker.h"
#include "Settings.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)),
      m_updateAvailable(false) {
  connect(m_networkManager, &QNetworkAccessManager::finished, this,
          &UpdateChecker::onCheckFinished);
}

void UpdateChecker::checkForUpdates() {
  emit updateCheckStarted();

  QString apiUrl = QString("https://api.github.com/repos/%1/releases/latest")
                       .arg(Settings::githubRepo());

  QNetworkRequest request(apiUrl);
  request.setHeader(QNetworkRequest::UserAgentHeader, "LinNote-UpdateChecker");
  request.setRawHeader("Accept", "application/vnd.github.v3+json");

  m_networkManager->get(request);
  qDebug() << "UpdateChecker: Checking for updates at" << apiUrl;
}

void UpdateChecker::onCheckFinished(QNetworkReply *reply) {
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    QString error = reply->errorString();
    qWarning() << "UpdateChecker: Check failed -" << error;
    emit updateCheckFailed(error);
    return;
  }

  QByteArray data = reply->readAll();
  QJsonDocument doc = QJsonDocument::fromJson(data);

  if (!doc.isObject()) {
    emit updateCheckFailed("Invalid response from GitHub");
    return;
  }

  QJsonObject json = doc.object();

  // Parse release info
  QString tagName = json["tag_name"].toString();
  QString name = json["name"].toString();
  QString body = json["body"].toString();
  QString htmlUrl = json["html_url"].toString();

  // Remove 'v' prefix if present (v1.0.0 -> 1.0.0)
  m_latestVersion = tagName;
  if (m_latestVersion.startsWith('v') || m_latestVersion.startsWith('V')) {
    m_latestVersion = m_latestVersion.mid(1);
  }

  m_changelog = body;
  m_downloadUrl = htmlUrl;

  // Compare versions
  QString currentVersion = Settings::appVersion();
  m_updateAvailable = isNewerVersion(currentVersion, m_latestVersion);

  qDebug() << "UpdateChecker: Current:" << currentVersion
           << "Latest:" << m_latestVersion
           << "Update available:" << m_updateAvailable;

  if (m_updateAvailable) {
    emit updateFound(m_latestVersion, m_changelog);
  } else {
    emit noUpdateFound();
  }
}

bool UpdateChecker::isNewerVersion(const QString &current,
                                   const QString &latest) {
  // Parse version strings (e.g., "1.0.0" -> [1, 0, 0])
  QStringList currentParts = current.split('.');
  QStringList latestParts = latest.split('.');

  // Compare each part
  for (int i = 0; i < qMax(currentParts.size(), latestParts.size()); ++i) {
    int currentNum = (i < currentParts.size()) ? currentParts[i].toInt() : 0;
    int latestNum = (i < latestParts.size()) ? latestParts[i].toInt() : 0;

    if (latestNum > currentNum) {
      return true;
    } else if (latestNum < currentNum) {
      return false;
    }
  }

  return false; // Versions are equal
}

QString UpdateChecker::latestVersion() const { return m_latestVersion; }

QString UpdateChecker::changelog() const { return m_changelog; }

QString UpdateChecker::downloadUrl() const { return m_downloadUrl; }

bool UpdateChecker::updateAvailable() const { return m_updateAvailable; }

void UpdateChecker::performUpdate() {
  emit updateStarted();

  // Run the install script with --update flag
  QProcess *process = new QProcess(this);

  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          [this, process](int exitCode, QProcess::ExitStatus status) {
            bool success = (status == QProcess::NormalExit && exitCode == 0);
            emit updateFinished(success);
            process->deleteLater();

            if (success) {
              qDebug() << "UpdateChecker: Update completed successfully";
            } else {
              qWarning() << "UpdateChecker: Update failed with exit code"
                         << exitCode;
            }
          });

  // Download and run the install script
  QString script = R"(
    curl -fsSL https://raw.githubusercontent.com/sfnemis/linnote/main/install.sh | bash -s -- --update
  )";

  process->start("/bin/bash", QStringList() << "-c" << script);
  qDebug() << "UpdateChecker: Starting update process...";
}
