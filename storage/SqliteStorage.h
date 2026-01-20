#ifndef LINNOTE_SQLITESTORAGE_H
#define LINNOTE_SQLITESTORAGE_H

#include "core/Note.h"
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QSqlDatabase>

/**
 * @brief SQLite-based persistent storage for notes
 *
 * Stores notes in: ~/.local/share/LinNote/notes.db
 * Flatpak compatible via XDG_DATA_HOME
 */
class SqliteStorage : public QObject {
  Q_OBJECT

public:
  explicit SqliteStorage(QObject *parent = nullptr);
  ~SqliteStorage();

  /**
   * @brief Save all notes to database
   * @param notes List of notes to save
   * @param currentIndex Currently selected note index
   * @return true if save was successful
   */
  bool save(const QList<Note> &notes, int currentIndex);

  /**
   * @brief Load all notes from database
   * @param currentIndex Output: saved current index
   * @return List of loaded notes (empty if no saved data)
   */
  QList<Note> load(int &currentIndex);

  /**
   * @brief Save a single note (insert or update)
   */
  bool saveNote(const Note &note);

  /**
   * @brief Delete a note by ID
   */
  bool deleteNote(const QString &id);

  /**
   * @brief Save settings to database (key-value pairs in metadata table)
   * @param settings QJsonObject with all settings
   * @return true if save was successful
   */
  bool saveSettings(const QJsonObject &settings);

  /**
   * @brief Load settings from database
   * @return QJsonObject with all settings (empty if none saved)
   */
  QJsonObject loadSettings();

  /**
   * @brief Get the database file path
   */
  QString databasePath() const;

private:
  bool initDatabase();
  bool createTables();
  QString ensureStorageDir() const;

  QSqlDatabase m_db;
  bool m_initialized;
};

#endif // LINNOTE_SQLITESTORAGE_H
