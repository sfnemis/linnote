#include "BackupManager.h"
#include "core/Settings.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

BackupManager::BackupManager(QObject *parent)
    : QObject(parent), m_backupTimer(new QTimer(this)) {
  connect(m_backupTimer, &QTimer::timeout, this, &BackupManager::onBackupTimer);
}

BackupManager::~BackupManager() { stopAutoBackup(); }

void BackupManager::startAutoBackup() {
  Settings *s = Settings::instance();
  if (!s->backupEnabled()) {
    qDebug() << "BackupManager: Auto-backup is disabled";
    return;
  }

  int intervalHours = s->backupIntervalHours();
  if (intervalHours <= 0) {
    intervalHours = 3; // Default fallback
  }

  int intervalMs = intervalHours * 60 * 60 * 1000;
  m_backupTimer->start(intervalMs);
  qDebug() << "BackupManager: Started auto-backup, interval:" << intervalHours
           << "hours";

  // Create initial backup if none exists
  if (existingBackups().isEmpty()) {
    createBackup();
  }
}

void BackupManager::stopAutoBackup() {
  if (m_backupTimer->isActive()) {
    m_backupTimer->stop();
    qDebug() << "BackupManager: Stopped auto-backup";
  }
}

void BackupManager::onBackupTimer() {
  qDebug() << "BackupManager: Timer triggered, creating backup...";
  createBackup();
}

QString BackupManager::databasePath() const {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dataPath.isEmpty())
    return QString();
  return dataPath + "/notes.db";
}

QString BackupManager::backupPath() const {
  Settings *s = Settings::instance();
  QString path = s->backupPath();

  // Default to Documents/LinNote_Backups if not set
  if (path.isEmpty()) {
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    path += "/LinNote_Backups";
  }

  // Ensure directory exists
  QDir dir(path);
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  return path;
}

bool BackupManager::createBackup() {
  QString dbPath = databasePath();
  if (dbPath.isEmpty() || !QFile::exists(dbPath)) {
    qDebug() << "BackupManager: Database not found at" << dbPath;
    return false;
  }

  QString backupDir = backupPath();
  if (backupDir.isEmpty()) {
    qDebug() << "BackupManager: Backup path not configured";
    return false;
  }

  // Generate backup filename with timestamp
  QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
  QString backupFile =
      QString("%1/linnote_backup_%2.db").arg(backupDir).arg(timestamp);

  // Copy the database file
  if (QFile::copy(dbPath, backupFile)) {
    m_lastBackupTime = timestamp;
    qDebug() << "BackupManager: Backup created:" << backupFile;

    // Clean old backups
    cleanOldBackups();
    return true;
  } else {
    qDebug() << "BackupManager: Failed to create backup";
    return false;
  }
}

QStringList BackupManager::existingBackups() const {
  QString backupDir = backupPath();
  QDir dir(backupDir);

  if (!dir.exists()) {
    return QStringList();
  }

  // Find all backup files
  QStringList filters;
  filters << "linnote_backup_*.db";
  QStringList backups = dir.entryList(filters, QDir::Files, QDir::Time);

  // Return full paths
  QStringList fullPaths;
  for (const QString &file : backups) {
    fullPaths << dir.absoluteFilePath(file);
  }

  return fullPaths;
}

void BackupManager::cleanOldBackups() {
  Settings *s = Settings::instance();
  int retention = s->backupRetentionCount();
  if (retention <= 0) {
    retention = 12; // Default fallback
  }

  QStringList backups = existingBackups();
  if (backups.size() <= retention) {
    return; // Nothing to clean
  }

  // Remove oldest backups (list is sorted by time, newest first)
  for (int i = retention; i < backups.size(); ++i) {
    QString oldBackup = backups.at(i);
    if (QFile::remove(oldBackup)) {
      qDebug() << "BackupManager: Removed old backup:" << oldBackup;
    }
  }
}
