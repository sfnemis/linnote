#ifndef LINNOTE_NOTEMODE_H
#define LINNOTE_NOTEMODE_H

#include <QString>

/**
 * @brief Available note modes
 */
enum class NoteMode {
  PlainText = 0, // Default plain text
  Checklist = 1, // Todo list with checkboxes
  Code = 2,      // Code with syntax highlighting
  Math = 3,      // Calculator mode
  Timer = 4,     // Stopwatch/Countdown/Pomodoro
  Markdown = 5   // Markdown formatting
};

/**
 * @brief Convert NoteMode to string for serialization
 */
inline QString noteModeToString(NoteMode mode) {
  switch (mode) {
  case NoteMode::Checklist:
    return QStringLiteral("checklist");
  case NoteMode::Code:
    return QStringLiteral("code");
  case NoteMode::Math:
    return QStringLiteral("math");
  case NoteMode::Timer:
    return QStringLiteral("timer");
  case NoteMode::Markdown:
    return QStringLiteral("markdown");
  default:
    return QStringLiteral("plain");
  }
}

/**
 * @brief Convert string to NoteMode for deserialization
 */
inline NoteMode stringToNoteMode(const QString &str) {
  if (str == QLatin1String("checklist"))
    return NoteMode::Checklist;
  if (str == QLatin1String("code"))
    return NoteMode::Code;
  if (str == QLatin1String("math"))
    return NoteMode::Math;
  if (str == QLatin1String("timer"))
    return NoteMode::Timer;
  if (str == QLatin1String("markdown"))
    return NoteMode::Markdown;
  return NoteMode::PlainText;
}

/**
 * @brief Get display name for mode
 */
inline QString noteModeName(NoteMode mode) {
  switch (mode) {
  case NoteMode::Checklist:
    return QStringLiteral("Checklist");
  case NoteMode::Code:
    return QStringLiteral("Code");
  case NoteMode::Math:
    return QStringLiteral("Math");
  case NoteMode::Timer:
    return QStringLiteral("Timer");
  case NoteMode::Markdown:
    return QStringLiteral("Markdown");
  default:
    return QStringLiteral("Plain Text");
  }
}

/**
 * @brief Get icon/emoji for mode
 */
inline QString noteModeIcon(NoteMode mode) {
  switch (mode) {
  case NoteMode::Checklist:
    return QStringLiteral("☑");
  case NoteMode::Code:
    return QStringLiteral("</>");
  case NoteMode::Math:
    return QStringLiteral("∑");
  case NoteMode::Timer:
    return QStringLiteral("⏱");
  case NoteMode::Markdown:
    return QStringLiteral("📝");
  default:
    return QStringLiteral("📄");
  }
}

#endif // LINNOTE_NOTEMODE_H
