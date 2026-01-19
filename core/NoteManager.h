#ifndef LINNOTE_NOTEMANAGER_H
#define LINNOTE_NOTEMANAGER_H

#include "Note.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>

class SqliteStorage;

/**
 * @brief Manages all notes in the application
 *
 * Provides:
 * - Note CRUD operations
 * - Current note tracking
 * - Persistence via SqliteStorage
 */
class NoteManager : public QObject {
  Q_OBJECT

public:
  explicit NoteManager(QObject *parent = nullptr);
  ~NoteManager() override;

  // Note access
  QList<Note> notes() const;
  int noteCount() const;
  Note noteAt(int index) const;
  Note noteById(const QString &id) const;
  int indexOfNote(const QString &id) const;
  QString noteContentAt(int index) const;
  void setNoteContentAt(int index, const QString &content);

  // Current note
  Note currentNote() const;
  int currentIndex() const;
  void setCurrentIndex(int index);
  void setCurrentNoteById(const QString &id);

  // CRUD operations
  Note createNote(const QString &title = QString());
  void deleteNote(const QString &id);
  void deleteNoteAt(int index);
  void deleteAllNotes(); // Delete all notes and create a fresh one
  void renameNote(const QString &id, const QString &newTitle);
  void updateNoteContent(const QString &id, const QString &content);
  void updateNoteMode(const QString &id, NoteMode mode);
  void setNotePasswordHash(const QString &id, const QString &hash);

  // Expiry management
  void setNoteExpiry(const QString &id, const QDateTime &expiresAt);
  void clearNoteExpiry(const QString &id);

  // Session memory (runtime unlock tracking)
  void sessionUnlock(const QString &id);
  void sessionLock(const QString &id);
  bool isSessionUnlocked(const QString &id) const;
  void clearSessionUnlocks();
  void setDecryptedContent(const QString &id, const QString &content);
  QString getDecryptedContent(const QString &id) const;

  // Persistence
  void saveAll();
  void loadAll();

  // Navigation
  void nextNote();
  void previousNote();
  void goToNote(int index);

signals:
  void noteCreated(const Note &note);
  void noteDeleted(const QString &id);
  void noteRenamed(const QString &id, const QString &newTitle);
  void noteContentChanged(const QString &id);
  void noteModeChanged(const QString &id, NoteMode mode);
  void currentNoteChanged(int index, const Note &note);
  void notesLoaded();

private:
  void ensureAtLeastOneNote();

  QList<Note> m_notes;
  int m_currentIndex;
  SqliteStorage *m_storage;
  QSet<QString> m_sessionUnlockedNotes; // Runtime-only, cleared on restart
  QMap<QString, QString>
      m_decryptedCache; // Cache decrypted content for session
};

#endif // LINNOTE_NOTEMANAGER_H
