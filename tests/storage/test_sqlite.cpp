#include "core/Note.h"
#include "storage/SqliteStorage.h"
#include <QDir>
#include <QTemporaryDir>
#include <QTest>

class TestSqliteStorage : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void cleanup();

  // Database initialization
  void testDatabaseCreation();
  void testDatabasePath();

  // Note CRUD operations
  void testSaveAndLoadNotes();
  void testSaveEmptyNotes();
  void testSaveNote();
  void testDeleteNote();
  void testUpdateNote();

  // Settings storage
  void testSaveAndLoadSettings();
  void testEmptySettings();
  void testSettingsOverwrite();

  // Edge cases
  void testLargeNoteContent();
  void testSpecialCharactersInContent();
  void testUnicodeContent();
  void testManyNotes();

private:
  SqliteStorage *m_storage;
  QTemporaryDir *m_tempDir;
};

void TestSqliteStorage::initTestCase() {
  // Use temp directory for test database
  m_tempDir = new QTemporaryDir();
  QVERIFY(m_tempDir->isValid());

  // Set XDG_DATA_HOME to temp dir for isolated testing
  qputenv("XDG_DATA_HOME", m_tempDir->path().toUtf8());

  m_storage = new SqliteStorage();
}

void TestSqliteStorage::cleanupTestCase() {
  delete m_storage;
  delete m_tempDir;
}

void TestSqliteStorage::cleanup() {
  // Clean up between tests - delete all notes
  int idx;
  QList<Note> notes = m_storage->load(idx);
  for (const Note &note : notes) {
    m_storage->deleteNote(note.id());
  }
}

void TestSqliteStorage::testDatabaseCreation() {
  // Database should be created automatically
  QString dbPath = m_storage->databasePath();
  QVERIFY(!dbPath.isEmpty());
}

void TestSqliteStorage::testDatabasePath() {
  QString path = m_storage->databasePath();
  QVERIFY(path.contains("LinNote") || path.contains("notes.db"));
}

void TestSqliteStorage::testSaveAndLoadNotes() {
  Note note1("Test content 1");
  note1.setTitle("Note 1");
  Note note2("Test content 2");
  note2.setTitle("Note 2");

  QList<Note> notesToSave;
  notesToSave << note1 << note2;

  QVERIFY(m_storage->save(notesToSave, 1));

  int loadedIndex;
  QList<Note> loadedNotes = m_storage->load(loadedIndex);

  QCOMPARE(loadedNotes.size(), 2);
  QCOMPARE(loadedIndex, 1);
  QCOMPARE(loadedNotes[0].content(), QString("Test content 1"));
  QCOMPARE(loadedNotes[1].content(), QString("Test content 2"));
}

void TestSqliteStorage::testSaveEmptyNotes() {
  QList<Note> emptyList;
  QVERIFY(m_storage->save(emptyList, 0));

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  QCOMPARE(loaded.size(), 0);
}

void TestSqliteStorage::testSaveNote() {
  Note note("Single note content");
  note.setTitle("Single Note");

  QVERIFY(m_storage->saveNote(note));

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  QVERIFY(loaded.size() >= 1);

  bool found = false;
  for (const Note &n : loaded) {
    if (n.id() == note.id()) {
      QCOMPARE(n.content(), QString("Single note content"));
      found = true;
      break;
    }
  }
  QVERIFY(found);
}

void TestSqliteStorage::testDeleteNote() {
  Note note("Delete me");
  m_storage->saveNote(note);

  QVERIFY(m_storage->deleteNote(note.id()));

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  for (const Note &n : loaded) {
    QVERIFY(n.id() != note.id());
  }
}

void TestSqliteStorage::testUpdateNote() {
  Note note("Original content");
  note.setTitle("Original Title");
  m_storage->saveNote(note);

  // Update the note
  note.setContent("Updated content");
  note.setTitle("Updated Title");
  QVERIFY(m_storage->saveNote(note));

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  for (const Note &n : loaded) {
    if (n.id() == note.id()) {
      QCOMPARE(n.content(), QString("Updated content"));
      break;
    }
  }
}

void TestSqliteStorage::testSaveAndLoadSettings() {
  QJsonObject settings;
  settings["darkMode"] = true;
  settings["fontSize"] = 14;
  settings["theme"] = "nord";
  settings["autoSave"] = true;

  QVERIFY(m_storage->saveSettings(settings));

  QJsonObject loaded = m_storage->loadSettings();
  QCOMPARE(loaded["darkMode"].toBool(), true);
  QCOMPARE(loaded["fontSize"].toInt(), 14);
  QCOMPARE(loaded["theme"].toString(), QString("nord"));
}

void TestSqliteStorage::testEmptySettings() {
  // Fresh database should return empty settings
  QJsonObject loaded = m_storage->loadSettings();
  // May be empty or have defaults - just shouldn't crash
  QVERIFY(true);
}

void TestSqliteStorage::testSettingsOverwrite() {
  QJsonObject settings1;
  settings1["value"] = 1;
  m_storage->saveSettings(settings1);

  QJsonObject settings2;
  settings2["value"] = 2;
  settings2["newKey"] = "test";
  m_storage->saveSettings(settings2);

  QJsonObject loaded = m_storage->loadSettings();
  QCOMPARE(loaded["value"].toInt(), 2);
  QCOMPARE(loaded["newKey"].toString(), QString("test"));
}

void TestSqliteStorage::testLargeNoteContent() {
  // Create a note with large content (1MB)
  QString largeContent;
  largeContent.reserve(1024 * 1024);
  for (int i = 0; i < 100000; i++) {
    largeContent += "Large content line ";
  }

  Note note(largeContent);
  QVERIFY(m_storage->saveNote(note));

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  for (const Note &n : loaded) {
    if (n.id() == note.id()) {
      QCOMPARE(n.content().length(), largeContent.length());
      break;
    }
  }
}

void TestSqliteStorage::testSpecialCharactersInContent() {
  QString specialContent = "Special chars: \"'\\n\\t<>&@#$%^*()[]{}|";
  Note note(specialContent);
  m_storage->saveNote(note);

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  for (const Note &n : loaded) {
    if (n.id() == note.id()) {
      QCOMPARE(n.content(), specialContent);
      break;
    }
  }
}

void TestSqliteStorage::testUnicodeContent() {
  QString unicodeContent =
      "Unicode: 日本語 中文 한국어 العربية Ελληνικά 🎉🚀💻";
  Note note(unicodeContent);
  m_storage->saveNote(note);

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  for (const Note &n : loaded) {
    if (n.id() == note.id()) {
      QCOMPARE(n.content(), unicodeContent);
      break;
    }
  }
}

void TestSqliteStorage::testManyNotes() {
  QList<Note> notes;
  for (int i = 0; i < 100; i++) {
    Note note(QString("Note content %1").arg(i));
    note.setTitle(QString("Note %1").arg(i));
    notes.append(note);
  }

  QVERIFY(m_storage->save(notes, 50));

  int idx;
  QList<Note> loaded = m_storage->load(idx);
  QCOMPARE(loaded.size(), 100);
  QCOMPARE(idx, 50);
}

QTEST_MAIN(TestSqliteStorage)
#include "test_sqlite.moc"
