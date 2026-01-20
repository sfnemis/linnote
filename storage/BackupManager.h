#ifndef LINNOTE_BACKUPMANAGER_H
#define LINNOTE_BACKUPMANAGER_H

#include <QObject>
#include <QTimer>

/**
 * @brief Manages automatic database backups
 *
 * Features:
 * - Periodic SQLite database backup
 * - User-configurable backup folder
 * - Retention policy (keeps last N backups)
 * - Encryption preserved (copies encrypted DB as-is)
 */
class BackupManager : public QObject {
  Q_OBJECT

public:
  explicit BackupManager(QObject *parent = nullptr);
  ~BackupManager() override;

  // Control
  void startAutoBackup();
  void stopAutoBackup();

  // Manual backup
  bool createBackup();

  // Cleanup
  void cleanOldBackups();

  // Info
  QString backupPath() const;
  QStringList existingBackups() const;
  QString databasePath() const;

private slots:
  void onBackupTimer();

private:
  QTimer *m_backupTimer;
  QString m_lastBackupTime;
};

#endif // LINNOTE_BACKUPMANAGER_H
