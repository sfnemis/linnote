#include "NoteManager.h"
#include "Settings.h"
#include "storage/SqliteStorage.h"
#include <QDebug>

NoteManager::NoteManager(QObject *parent)
    : QObject(parent), m_currentIndex(-1), m_storage(new SqliteStorage(this)) {
  loadAll();
  ensureAtLeastOneNote();
}

NoteManager::~NoteManager() { saveAll(); }

QList<Note> NoteManager::notes() const { return m_notes; }

int NoteManager::noteCount() const { return m_notes.size(); }

Note NoteManager::noteAt(int index) const {
  if (index >= 0 && index < m_notes.size()) {
    return m_notes.at(index);
  }
  return Note();
}

Note NoteManager::noteById(const QString &id) const {
  for (const Note &note : m_notes) {
    if (note.id() == id) {
      return note;
    }
  }
  return Note();
}

int NoteManager::indexOfNote(const QString &id) const {
  for (int i = 0; i < m_notes.size(); ++i) {
    if (m_notes.at(i).id() == id) {
      return i;
    }
  }
  return -1;
}

QString NoteManager::noteContentAt(int index) const {
  if (index >= 0 && index < m_notes.size()) {
    return m_notes.at(index).content();
  }
  return QString();
}

void NoteManager::setNoteContentAt(int index, const QString &content) {
  if (index >= 0 && index < m_notes.size()) {
    m_notes[index].setContent(content);
    emit noteContentChanged(m_notes[index].id());
  }
}

Note NoteManager::currentNote() const { return noteAt(m_currentIndex); }

int NoteManager::currentIndex() const { return m_currentIndex; }

void NoteManager::setCurrentIndex(int index) {
  if (index >= 0 && index < m_notes.size() && index != m_currentIndex) {
    m_currentIndex = index;
    emit currentNoteChanged(m_currentIndex, currentNote());
  }
}

void NoteManager::setCurrentNoteById(const QString &id) {
  int index = indexOfNote(id);
  if (index >= 0) {
    setCurrentIndex(index);
  }
}

Note NoteManager::createNote(const QString &title) {
  Note note;
  if (!title.isEmpty()) {
    note.setTitle(title);
  } else {
    note.setTitle(QString("Page %1").arg(m_notes.size() + 1));
  }

  // Apply default mode from settings
  int defaultMode = Settings::instance()->defaultMode();
  note.setMode(static_cast<NoteMode>(defaultMode));

  m_notes.append(note);
  emit noteCreated(note);

  // Switch to new note
  setCurrentIndex(m_notes.size() - 1);

  saveAll();
  return note;
}

void NoteManager::deleteNote(const QString &id) {
  int index = indexOfNote(id);
  if (index >= 0) {
    deleteNoteAt(index);
  }
}

void NoteManager::deleteNoteAt(int index) {
  if (index < 0 || index >= m_notes.size()) {
    return;
  }

  QString deletedId = m_notes.at(index).id();
  m_notes.removeAt(index);
  emit noteDeleted(deletedId);

  // Adjust current index
  if (m_notes.isEmpty()) {
    m_currentIndex = -1;
    ensureAtLeastOneNote();
  } else if (m_currentIndex >= m_notes.size()) {
    setCurrentIndex(m_notes.size() - 1);
  } else if (m_currentIndex == index) {
    // Re-emit to update UI
    emit currentNoteChanged(m_currentIndex, currentNote());
  }

  saveAll();
}

void NoteManager::deleteAllNotes() {
  qDebug() << "NoteManager: Deleting all notes...";

  // Clear session unlocks and decrypted cache
  clearSessionUnlocks();

  // Delete all notes from storage first
  for (const Note &note : m_notes) {
    m_storage->deleteNote(note.id());
    emit noteDeleted(note.id());
  }

  // Clear the notes list
  m_notes.clear();
  m_currentIndex = -1;

  // Create a fresh note
  ensureAtLeastOneNote();

  qDebug() << "NoteManager: All notes deleted, created fresh note";
}

void NoteManager::renameNote(const QString &id, const QString &newTitle) {
  int index = indexOfNote(id);
  if (index >= 0) {
    m_notes[index].setTitle(newTitle);
    emit noteRenamed(id, newTitle);
    saveAll();
  }
}

void NoteManager::updateNoteContent(const QString &id, const QString &content) {
  int index = indexOfNote(id);
  if (index >= 0) {
    m_notes[index].setContent(content);
    emit noteContentChanged(id);
    // Don't save on every keystroke - will be saved on exit or explicit save
  }
}

void NoteManager::updateNoteMode(const QString &id, NoteMode mode) {
  int index = indexOfNote(id);
  if (index >= 0) {
    m_notes[index].setMode(mode);
    emit noteModeChanged(id, mode);
    saveAll();
  }
}

void NoteManager::setNotePasswordHash(const QString &id, const QString &hash) {
  int index = indexOfNote(id);
  if (index >= 0) {
    m_notes[index].setPasswordHash(hash);
    saveAll();
  }
}

void NoteManager::setNoteExpiry(const QString &id, const QDateTime &expiresAt) {
  int index = indexOfNote(id);
  if (index >= 0) {
    m_notes[index].setExpiresAt(expiresAt);
    qDebug() << "NoteManager: Set expiry for note" << id << "to" << expiresAt;
    saveAll();
  }
}

void NoteManager::clearNoteExpiry(const QString &id) {
  int index = indexOfNote(id);
  if (index >= 0) {
    m_notes[index].clearExpiry();
    qDebug() << "NoteManager: Cleared expiry for note" << id;
    saveAll();
  }
}

// Session memory methods (runtime-only, cleared on restart)
void NoteManager::sessionUnlock(const QString &id) {
  m_sessionUnlockedNotes.insert(id);
  qDebug() << "NoteManager: Session unlocked note" << id;
}

void NoteManager::sessionLock(const QString &id) {
  m_sessionUnlockedNotes.remove(id);
  qDebug() << "NoteManager: Session locked note" << id;
}

bool NoteManager::isSessionUnlocked(const QString &id) const {
  return m_sessionUnlockedNotes.contains(id);
}

void NoteManager::clearSessionUnlocks() {
  m_sessionUnlockedNotes.clear();
  m_decryptedCache.clear();
  qDebug() << "NoteManager: Cleared all session unlocks and decrypted cache";
}

void NoteManager::setDecryptedContent(const QString &id,
                                      const QString &content) {
  m_decryptedCache[id] = content;
}

QString NoteManager::getDecryptedContent(const QString &id) const {
  return m_decryptedCache.value(id);
}

void NoteManager::saveAll() {
  m_storage->save(m_notes, m_currentIndex);
  qDebug() << "NoteManager: Saved" << m_notes.size() << "notes";
}

void NoteManager::loadAll() {
  int savedIndex = 0;
  m_notes = m_storage->load(savedIndex);
  m_currentIndex = savedIndex;

  qDebug() << "NoteManager: Loaded" << m_notes.size() << "notes";
  emit notesLoaded();

  if (!m_notes.isEmpty() && m_currentIndex >= 0) {
    emit currentNoteChanged(m_currentIndex, currentNote());
  }
}

void NoteManager::nextNote() {
  if (m_notes.size() > 1) {
    int next = (m_currentIndex + 1) % m_notes.size();
    setCurrentIndex(next);
  }
}

void NoteManager::previousNote() {
  if (m_notes.size() > 1) {
    int prev = (m_currentIndex - 1 + m_notes.size()) % m_notes.size();
    setCurrentIndex(prev);
  }
}

void NoteManager::goToNote(int index) { setCurrentIndex(index); }

void NoteManager::ensureAtLeastOneNote() {
  if (m_notes.isEmpty()) {
    Note note;
    note.setTitle("Page 1");
    m_notes.append(note);
    m_currentIndex = 0;
    emit noteCreated(note);
    emit currentNoteChanged(0, note);
  }
}
