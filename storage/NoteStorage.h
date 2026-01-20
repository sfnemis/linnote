#ifndef LINNOTE_NOTESTORAGE_H
#define LINNOTE_NOTESTORAGE_H

#include "core/Note.h"
#include <QList>
#include <QObject>

/**
 * @brief JSON-based persistent storage for notes
 *
 * Stores notes in: ~/.local/share/LinNote/notes.json
 * Flatpak compatible via XDG_DATA_HOME
 */
class NoteStorage : public QObject {
  Q_OBJECT

public:
  explicit NoteStorage(QObject *parent = nullptr);

  /**
   * @brief Save all notes to storage
   * @param notes List of notes to save
   * @param currentIndex Currently selected note index
   * @return true if save was successful
   */
  bool save(const QList<Note> &notes, int currentIndex);

  /**
   * @brief Load all notes from storage
   * @param currentIndex Output: saved current index
   * @return List of loaded notes (empty if no saved data)
   */
  QList<Note> load(int &currentIndex);

  /**
   * @brief Get the storage file path
   */
  QString storagePath() const;

private:
  QString ensureStorageDir() const;
};

#endif // LINNOTE_NOTESTORAGE_H
