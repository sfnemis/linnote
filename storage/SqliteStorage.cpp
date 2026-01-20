#include "SqliteStorage.h"
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

SqliteStorage::SqliteStorage(QObject *parent)
    : QObject(parent), m_initialized(false) {
  initDatabase();
}

SqliteStorage::~SqliteStorage() {
  if (m_db.isOpen()) {
    m_db.close();
  }
}

bool SqliteStorage::initDatabase() {
  QString path = databasePath();
  if (path.isEmpty()) {
    qWarning() << "SqliteStorage: Could not determine database path";
    return false;
  }

  // Use unique connection name
  QString connectionName = "linnote_main";
  if (QSqlDatabase::contains(connectionName)) {
    m_db = QSqlDatabase::database(connectionName);
  } else {
    m_db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
  }

  m_db.setDatabaseName(path);

  if (!m_db.open()) {
    qWarning() << "SqliteStorage: Could not open database:"
               << m_db.lastError().text();
    return false;
  }

  if (!createTables()) {
    qWarning() << "SqliteStorage: Could not create tables";
    return false;
  }

  m_initialized = true;
  qDebug() << "SqliteStorage: Database initialized at" << path;
  return true;
}

bool SqliteStorage::createTables() {
  QSqlQuery query(m_db);

  // Notes table
  bool success = query.exec(R"(
    CREATE TABLE IF NOT EXISTS notes (
      id TEXT PRIMARY KEY,
      title TEXT NOT NULL,
      content TEXT,
      mode INTEGER DEFAULT 0,
      password_hash TEXT,
      expires_at TEXT,
      created_at TEXT,
      updated_at TEXT
    )
  )");

  if (!success) {
    qWarning() << "SqliteStorage: Failed to create notes table:"
               << query.lastError().text();
    return false;
  }

  // Migration: Add expires_at column if missing
  query.exec("ALTER TABLE notes ADD COLUMN expires_at TEXT");

  // Metadata table
  success = query.exec(R"(
    CREATE TABLE IF NOT EXISTS metadata (
      key TEXT PRIMARY KEY,
      value TEXT
    )
  )");

  if (!success) {
    qWarning() << "SqliteStorage: Failed to create metadata table:"
               << query.lastError().text();
    return false;
  }

  qDebug() << "SqliteStorage: Tables created successfully";
  return true;
}

bool SqliteStorage::save(const QList<Note> &notes, int currentIndex) {
  if (!m_initialized) {
    qWarning() << "SqliteStorage: Database not initialized";
    return false;
  }

  // Start transaction for performance
  m_db.transaction();

  // Clear existing notes and re-insert all
  QSqlQuery query(m_db);
  query.exec("DELETE FROM notes");

  for (const Note &note : notes) {
    if (!saveNote(note)) {
      m_db.rollback();
      return false;
    }
  }

  // Save current index to metadata
  query.prepare("INSERT OR REPLACE INTO metadata (key, value) VALUES "
                "('current_index', :value)");
  query.bindValue(":value", QString::number(currentIndex));
  if (!query.exec()) {
    qWarning() << "SqliteStorage: Failed to save current_index:"
               << query.lastError().text();
    m_db.rollback();
    return false;
  }

  m_db.commit();
  qDebug() << "SqliteStorage: Saved" << notes.size() << "notes";
  return true;
}

QList<Note> SqliteStorage::load(int &currentIndex) {
  QList<Note> notes;
  currentIndex = 0;

  if (!m_initialized) {
    qWarning() << "SqliteStorage: Database not initialized";
    return notes;
  }

  QSqlQuery query(m_db);

  // Load current index
  query.exec("SELECT value FROM metadata WHERE key = 'current_index'");
  if (query.next()) {
    currentIndex = query.value(0).toString().toInt();
  }

  // Load notes
  query.exec(
      "SELECT id, title, content, mode, password_hash, expires_at, created_at, "
      "updated_at FROM notes");
  while (query.next()) {
    QString id = query.value(0).toString();
    QString title = query.value(1).toString();
    QString content = query.value(2).toString();

    Note note(id, title, content);
    note.setMode(static_cast<NoteMode>(query.value(3).toInt()));
    note.setPasswordHash(query.value(4).toString());
    QString expiresStr = query.value(5).toString();
    if (!expiresStr.isEmpty()) {
      note.setExpiresAt(QDateTime::fromString(expiresStr, Qt::ISODate));
    }
    // created_at and updated_at stored but not used in Note class yet
    notes.append(note);
  }

  qDebug() << "SqliteStorage: Loaded" << notes.size() << "notes";
  return notes;
}

bool SqliteStorage::saveNote(const Note &note) {
  QSqlQuery query(m_db);
  query.prepare(R"(
    INSERT OR REPLACE INTO notes (id, title, content, mode, password_hash, expires_at, updated_at)
    VALUES (:id, :title, :content, :mode, :password_hash, :expires_at, datetime('now'))
  )");

  query.bindValue(":id", note.id());
  query.bindValue(":title", note.title());
  query.bindValue(":content", note.content());
  query.bindValue(":mode", static_cast<int>(note.mode()));
  query.bindValue(":password_hash", note.passwordHash());
  if (note.hasExpiry()) {
    query.bindValue(":expires_at", note.expiresAt().toString(Qt::ISODate));
  } else {
    query.bindValue(":expires_at", QVariant());
  }

  if (!query.exec()) {
    qWarning() << "SqliteStorage: Failed to save note:"
               << query.lastError().text();
    return false;
  }

  return true;
}

bool SqliteStorage::deleteNote(const QString &id) {
  QSqlQuery query(m_db);
  query.prepare("DELETE FROM notes WHERE id = :id");
  query.bindValue(":id", id);

  if (!query.exec()) {
    qWarning() << "SqliteStorage: Failed to delete note:"
               << query.lastError().text();
    return false;
  }

  return true;
}

bool SqliteStorage::saveSettings(const QJsonObject &settings) {
  if (!m_initialized) {
    qWarning() << "SqliteStorage: Database not initialized for settings";
    return false;
  }

  m_db.transaction();
  QSqlQuery query(m_db);

  // Delete old settings
  query.exec("DELETE FROM metadata WHERE key LIKE 'settings_%'");

  // Insert new settings
  for (auto it = settings.constBegin(); it != settings.constEnd(); ++it) {
    query.prepare(
        "INSERT OR REPLACE INTO metadata (key, value) VALUES (:key, :value)");
    query.bindValue(":key", "settings_" + it.key());

    // Convert value to string
    QJsonValue val = it.value();
    QString strValue;
    if (val.isBool()) {
      strValue = val.toBool() ? "true" : "false";
    } else if (val.isDouble()) {
      strValue = QString::number(val.toDouble());
    } else if (val.isString()) {
      strValue = val.toString();
    } else if (val.isObject() || val.isArray()) {
      // For nested objects/arrays, store as JSON string
      QJsonDocument doc;
      if (val.isObject()) {
        doc.setObject(val.toObject());
      } else {
        doc.setArray(val.toArray());
      }
      strValue = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    } else {
      strValue = "";
    }

    query.bindValue(":value", strValue);
    if (!query.exec()) {
      qWarning() << "SqliteStorage: Failed to save setting:" << it.key()
                 << query.lastError().text();
      m_db.rollback();
      return false;
    }
  }

  m_db.commit();
  qDebug() << "SqliteStorage: Saved" << settings.size() << "settings";
  return true;
}

QJsonObject SqliteStorage::loadSettings() {
  QJsonObject settings;

  if (!m_initialized) {
    qWarning() << "SqliteStorage: Database not initialized for settings";
    return settings;
  }

  QSqlQuery query(m_db);
  query.exec("SELECT key, value FROM metadata WHERE key LIKE 'settings_%'");

  while (query.next()) {
    QString key = query.value(0).toString();
    QString value = query.value(1).toString();

    // Remove "settings_" prefix
    key = key.mid(9);

    // Try to parse the value
    if (value == "true") {
      settings[key] = true;
    } else if (value == "false") {
      settings[key] = false;
    } else {
      // Try as number
      bool ok;
      double num = value.toDouble(&ok);
      if (ok && !value.contains('{') && !value.contains('[')) {
        // Check if it's an integer
        if (value.contains('.')) {
          settings[key] = num;
        } else {
          settings[key] = static_cast<int>(num);
        }
      } else if (value.startsWith('{') || value.startsWith('[')) {
        // Try to parse as JSON
        QJsonDocument doc = QJsonDocument::fromJson(value.toUtf8());
        if (doc.isObject()) {
          settings[key] = doc.object();
        } else if (doc.isArray()) {
          settings[key] = doc.array();
        } else {
          settings[key] = value;
        }
      } else {
        settings[key] = value;
      }
    }
  }

  qDebug() << "SqliteStorage: Loaded" << settings.size() << "settings";
  return settings;
}

QString SqliteStorage::databasePath() const {
  QString dir = ensureStorageDir();
  if (dir.isEmpty()) {
    return QString();
  }
  return dir + "/notes.db";
}

QString SqliteStorage::ensureStorageDir() const {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dataPath.isEmpty()) {
    qWarning() << "SqliteStorage: No writable data location";
    return QString();
  }

  QDir dir(dataPath);
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      qWarning() << "SqliteStorage: Could not create directory:" << dataPath;
      return QString();
    }
  }

  return dataPath;
}
