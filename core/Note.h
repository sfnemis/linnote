#ifndef LINNOTE_NOTE_H
#define LINNOTE_NOTE_H

#include "NoteMode.h"
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QUuid>

/**
 * @brief Note data model with multi-note support
 *
 * Holds the content of a single note with:
 * - Unique ID (UUID)
 * - Title (smart auto-generated if empty)
 * - Content
 * - Timestamps
 * - JSON serialization
 */
class Note {
public:
  Note();
  explicit Note(const QString &content);
  Note(const QString &id, const QString &title, const QString &content);

  // Identity
  QString id() const;
  QString title() const;
  void setTitle(const QString &title);

  // Smart title (auto-generated from content if title is empty)
  QString smartTitle() const;

  // Mode
  NoteMode mode() const;
  void setMode(NoteMode mode);

  // Content access
  QString content() const;
  void setContent(const QString &content);

  // Timestamps
  QDateTime created() const;
  QDateTime modified() const;

  // State
  bool isEmpty() const;
  bool isModified() const;
  void markSaved();

  // Encryption
  bool isLocked() const;
  QString passwordHash() const;
  void setPasswordHash(const QString &hash);
  void clearPassword();

  // Expiry (for temporary notes)
  bool hasExpiry() const;
  bool isExpired() const;
  QDateTime expiresAt() const;
  void setExpiresAt(const QDateTime &dt);
  void clearExpiry();

  // Serialization
  QJsonObject toJson() const;
  static Note fromJson(const QJsonObject &json);

  // Comparison
  bool operator==(const Note &other) const;

private:
  QString m_id;
  QString m_title;
  QString m_content;
  NoteMode m_mode;
  QDateTime m_created;
  QDateTime m_modified;
  QDateTime m_expiresAt; // Empty = never expires
  bool m_dirty;
  QString m_passwordHash; // Empty = unlocked
};

#endif // LINNOTE_NOTE_H
