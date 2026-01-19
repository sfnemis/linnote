#include "NoteStorage.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

NoteStorage::NoteStorage(QObject *parent) : QObject(parent) {}

bool NoteStorage::save(const QList<Note> &notes, int currentIndex) {
  QString path = storagePath();
  if (path.isEmpty()) {
    qWarning() << "NoteStorage: Could not determine storage path";
    return false;
  }

  QJsonArray notesArray;
  for (const Note &note : notes) {
    notesArray.append(note.toJson());
  }

  QJsonObject root;
  root["version"] = 1;
  root["currentIndex"] = currentIndex;
  root["notes"] = notesArray;

  QJsonDocument doc(root);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    qWarning() << "NoteStorage: Could not open file for writing:" << path;
    return false;
  }

  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  qDebug() << "NoteStorage: Saved to" << path;
  return true;
}

QList<Note> NoteStorage::load(int &currentIndex) {
  QList<Note> notes;
  currentIndex = 0;

  QString path = storagePath();
  if (path.isEmpty()) {
    return notes;
  }

  QFile file(path);
  if (!file.exists()) {
    qDebug() << "NoteStorage: No saved data found";
    return notes;
  }

  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "NoteStorage: Could not open file for reading:" << path;
    return notes;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &error);

  if (error.error != QJsonParseError::NoError) {
    qWarning() << "NoteStorage: JSON parse error:" << error.errorString();
    return notes;
  }

  QJsonObject root = doc.object();
  currentIndex = root["currentIndex"].toInt(0);

  QJsonArray notesArray = root["notes"].toArray();
  for (const QJsonValue &value : notesArray) {
    notes.append(Note::fromJson(value.toObject()));
  }

  qDebug() << "NoteStorage: Loaded" << notes.size() << "notes from" << path;
  return notes;
}

QString NoteStorage::storagePath() const {
  QString dir = ensureStorageDir();
  if (dir.isEmpty()) {
    return QString();
  }
  return dir + "/notes.json";
}

QString NoteStorage::ensureStorageDir() const {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dataPath.isEmpty()) {
    qWarning() << "NoteStorage: No writable data location";
    return QString();
  }

  QDir dir(dataPath);
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      qWarning() << "NoteStorage: Could not create directory:" << dataPath;
      return QString();
    }
  }

  return dataPath;
}
