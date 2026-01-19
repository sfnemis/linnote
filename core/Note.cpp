#include "Note.h"
#include <QJsonObject>
#include <QRegularExpression>

Note::Note()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)), m_title(""),
      m_mode(NoteMode::PlainText), m_created(QDateTime::currentDateTime()),
      m_modified(m_created), m_dirty(false) {}

Note::Note(const QString &content)
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces)), m_title(""),
      m_content(content), m_mode(NoteMode::PlainText),
      m_created(QDateTime::currentDateTime()), m_modified(m_created),
      m_dirty(false) {}

Note::Note(const QString &id, const QString &title, const QString &content)
    : m_id(id), m_title(title), m_content(content), m_mode(NoteMode::PlainText),
      m_created(QDateTime::currentDateTime()), m_modified(m_created),
      m_dirty(false) {}

QString Note::id() const { return m_id; }

QString Note::title() const { return m_title; }

void Note::setTitle(const QString &title) {
  if (m_title != title) {
    m_title = title;
    m_modified = QDateTime::currentDateTime();
    m_dirty = true;
  }
}

QString Note::smartTitle() const {
  // Check settings for title mode (need to include Settings.h)
  // 0 = Smart (content-based with fallbacks)
  // 1 = DateTime only
  // 2 = FirstLine only

  // Note: We can't include Settings.h here due to circular dependency
  // The mode will be checked directly by external callers if needed
  // For now, keep smart behavior but document the expected external handling

  // If title is explicitly set and NOT a generic pattern, use it
  static QRegularExpression pagePattern(
      R"(^(Page\s*\d+|Untitled)$)", QRegularExpression::CaseInsensitiveOption);

  if (!m_title.isEmpty() && !pagePattern.match(m_title).hasMatch()) {
    return m_title;
  }

  // Generate smart title from content
  if (m_content.isEmpty()) {
    // Use creation time as fallback
    return m_created.toString("ddd hh:mm");
  }

  QString text = m_content.trimmed();

  // Check for mode-specific prefixes
  if (text.startsWith("- [")) {
    return QString("✅ Checklist");
  }

  // Check for math expressions
  static QRegularExpression mathPattern(R"(^\d+\s*[\+\-\*\/])");
  if (mathPattern.match(text).hasMatch()) {
    return QString("🧮 Calculations");
  }

  // Check for code patterns
  if (text.startsWith("//") || text.startsWith("/*") ||
      text.startsWith("#include") || text.startsWith("def ") ||
      text.startsWith("function ") || text.startsWith("class ")) {
    return QString("💻 Code");
  }

  // Check for currency patterns
  static QRegularExpression currencyPattern(
      R"(\d+\s*(USD|EUR|TRY|GBP|JPY|CAD|AUD|CHF|CNY|ALL|BTC|ETH|USDT|USDC|XRP|BNB|SOL|ADA|DOGE|TRX))",
      QRegularExpression::CaseInsensitiveOption);
  if (currencyPattern.match(text).hasMatch()) {
    return QString("💰 Currency");
  }

  // Extract first meaningful line (up to 20 chars)
  QString firstLine = text.split('\n').first().trimmed();

  // Remove slash commands
  if (firstLine.startsWith("/")) {
    QStringList lines = text.split('\n');
    if (lines.size() > 1) {
      firstLine = lines.at(1).trimmed();
    } else {
      firstLine = "";
    }
  }

  // Remove checkbox markers
  static QRegularExpression checkboxPattern(R"(^-\s*\[[ xX]\]\s*)");
  firstLine.replace(checkboxPattern, "");

  if (firstLine.isEmpty()) {
    return m_created.toString("ddd hh:mm");
  }

  // Truncate if too long
  if (firstLine.length() > 18) {
    firstLine = firstLine.left(16) + "…";
  }

  return firstLine;
}

NoteMode Note::mode() const { return m_mode; }

void Note::setMode(NoteMode mode) {
  if (m_mode != mode) {
    m_mode = mode;
    m_modified = QDateTime::currentDateTime();
    m_dirty = true;
  }
}

QString Note::content() const { return m_content; }

void Note::setContent(const QString &content) {
  if (m_content != content) {
    m_content = content;
    m_modified = QDateTime::currentDateTime();
    m_dirty = true;
  }
}

QDateTime Note::created() const { return m_created; }

QDateTime Note::modified() const { return m_modified; }

bool Note::isEmpty() const { return m_content.isEmpty(); }

bool Note::isModified() const { return m_dirty; }

void Note::markSaved() { m_dirty = false; }

bool Note::isLocked() const { return !m_passwordHash.isEmpty(); }

QString Note::passwordHash() const { return m_passwordHash; }

void Note::setPasswordHash(const QString &hash) {
  m_passwordHash = hash;
  m_modified = QDateTime::currentDateTime();
  m_dirty = true;
}

void Note::clearPassword() {
  m_passwordHash.clear();
  m_modified = QDateTime::currentDateTime();
  m_dirty = true;
}

// Expiry functions
bool Note::hasExpiry() const { return m_expiresAt.isValid(); }

bool Note::isExpired() const {
  return m_expiresAt.isValid() && m_expiresAt <= QDateTime::currentDateTime();
}

QDateTime Note::expiresAt() const { return m_expiresAt; }

void Note::setExpiresAt(const QDateTime &dt) {
  m_expiresAt = dt;
  m_modified = QDateTime::currentDateTime();
  m_dirty = true;
}

void Note::clearExpiry() {
  m_expiresAt = QDateTime();
  m_modified = QDateTime::currentDateTime();
  m_dirty = true;
}

QJsonObject Note::toJson() const {
  QJsonObject json;
  json["id"] = m_id;
  json["title"] = m_title;
  json["content"] = m_content;
  json["mode"] = noteModeToString(m_mode);
  json["created"] = m_created.toString(Qt::ISODate);
  json["modified"] = m_modified.toString(Qt::ISODate);
  json["passwordHash"] = m_passwordHash;
  if (m_expiresAt.isValid()) {
    json["expiresAt"] = m_expiresAt.toString(Qt::ISODate);
  }
  return json;
}

Note Note::fromJson(const QJsonObject &json) {
  Note note;
  note.m_id = json["id"].toString();
  note.m_title = json["title"].toString();
  note.m_content = json["content"].toString();
  note.m_mode = stringToNoteMode(json["mode"].toString());
  note.m_created =
      QDateTime::fromString(json["created"].toString(), Qt::ISODate);
  note.m_modified =
      QDateTime::fromString(json["modified"].toString(), Qt::ISODate);
  note.m_passwordHash = json["passwordHash"].toString();
  if (json.contains("expiresAt")) {
    note.m_expiresAt =
        QDateTime::fromString(json["expiresAt"].toString(), Qt::ISODate);
  }
  note.m_dirty = false;
  return note;
}

bool Note::operator==(const Note &other) const { return m_id == other.m_id; }
