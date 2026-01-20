#include "SlashCommand.h"
#include "NoteManager.h"
#include "NoteMode.h"
#include "Settings.h"
#include <QDebug>

// Helper: resolve custom alias to original keyword
static QString resolveAlias(const QString &input) {
  Settings *s = Settings::instance();
  QMap<QString, QString> aliases = s->keywordAliases();

  qDebug() << "SlashCommand: resolveAlias input:" << input
           << "aliases:" << aliases;

  // Check if input matches any alias, return original keyword
  for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
    if (!it.value().isEmpty() && it.value().toLower() == input.toLower()) {
      qDebug() << "SlashCommand: Found alias" << input << "->" << it.key();
      return it.key(); // Return original keyword
    }
  }
  return input; // No alias found, return as-is
}

SlashCommand::SlashCommand(NoteManager *manager, QObject *parent)
    : QObject(parent), m_manager(manager) {
  // Commands with slash prefix (for popup)
  m_commands << "/checklist" << "/code" << "/calc"
             << "/plain" << "/text" << "/settings"
             << "/list" << "/sum" << "/avg" << "/count" << "/paste"
             << "/timer" << "/ocr" << "/markdown" << "/md";

  // Keywords (without slash) - for tutorials
  m_keywords << "list" << "sum" << "avg" << "count"
             << "code" << "paste" << "timer" << "plain" << "settings"
             << "checklist" << "calc" << "text" << "ocr" << "markdown" << "md";
}

bool SlashCommand::isCommand(const QString &text) const {
  QString trimmed = text.trimmed().toLower();

  // Check with slash
  if (trimmed.startsWith('/')) {
    QString cmd = trimmed.mid(1);

    // Special case: commands with arguments (timer 5, paste(,), etc.)
    if (cmd.startsWith("timer") || cmd.startsWith("paste")) {
      return true;
    }

    QString resolved = resolveAlias(cmd);
    for (const QString &keyword : m_keywords) {
      if (resolved == keyword) {
        return true;
      }
    }
    for (const QString &command : m_commands) {
      if (trimmed == command) {
        return true;
      }
    }
  }

  // Special case: commands with arguments (without slash)
  if (trimmed.startsWith("timer") || trimmed.startsWith("paste")) {
    return true;
  }

  // Check keyword (without slash) - including aliases
  QString resolved = resolveAlias(trimmed);
  for (const QString &keyword : m_keywords) {
    if (resolved == keyword || trimmed == keyword) {
      return true;
    }
  }

  return false;
}

bool SlashCommand::isKeyword(const QString &text) const {
  QString trimmed = text.trimmed().toLower();
  for (const QString &keyword : m_keywords) {
    if (trimmed == keyword) {
      return true;
    }
  }
  return false;
}

QStringList SlashCommand::getCompletions(const QString &partial) const {
  QStringList results;
  QString lower = partial.toLower();

  for (const QString &cmd : m_commands) {
    if (cmd.startsWith(lower)) {
      results << cmd;
    }
  }

  // Also add keywords
  for (const QString &keyword : m_keywords) {
    if (keyword.startsWith(lower) && !results.contains("/" + keyword)) {
      results << keyword;
    }
  }

  return results;
}

QStringList SlashCommand::keywords() const { return m_keywords; }

bool SlashCommand::execute(const QString &command) {
  QString cmd = command.trimmed().toLower();

  // Remove leading slash if present
  if (cmd.startsWith('/')) {
    cmd = cmd.mid(1);
  }

  // Resolve custom alias to original keyword
  cmd = resolveAlias(cmd);

  // Checklist mode
  if (cmd == "checklist" || cmd == "list") {
    if (m_manager->currentIndex() >= 0) {
      Note note = m_manager->currentNote();
      m_manager->updateNoteMode(note.id(), NoteMode::Checklist);
      emit modeChanged(static_cast<int>(NoteMode::Checklist));
      qDebug() << "SlashCommand: Switched to Checklist mode";
    }
    return true;
  }

  // Code mode
  if (cmd == "code") {
    if (m_manager->currentIndex() >= 0) {
      Note note = m_manager->currentNote();
      m_manager->updateNoteMode(note.id(), NoteMode::Code);
      emit modeChanged(static_cast<int>(NoteMode::Code));
      qDebug() << "SlashCommand: Switched to Code mode";
    }
    return true;
  }

  // Math mode (calc only switches mode)
  if (cmd == "calc") {
    if (m_manager->currentIndex() >= 0) {
      Note note = m_manager->currentNote();
      m_manager->updateNoteMode(note.id(), NoteMode::Math);
      emit modeChanged(static_cast<int>(NoteMode::Math));
      qDebug() << "SlashCommand: Switched to Math mode";
    }
    return true;
  }

  // Text analysis commands: sum, avg, count
  if (cmd == "sum" || cmd == "avg" || cmd == "count") {
    emit textAnalysisRequested(cmd);
    qDebug() << "SlashCommand: Text analysis requested:" << cmd;
    return true;
  }

  // AutoPaste command: paste, paste(, ), paste(;)
  if (cmd.startsWith("paste")) {
    QString delimiter = "\n"; // Default: newline

    // Check for custom delimiter: paste(delimiter)
    QRegularExpression pasteRx("paste\\((.*)\\)");
    QRegularExpressionMatch match = pasteRx.match(cmd);
    if (match.hasMatch()) {
      delimiter = match.captured(1);
      if (delimiter.isEmpty()) {
        delimiter = "\n";
      }
    }

    emit autoPasteRequested(delimiter);
    qDebug() << "SlashCommand: AutoPaste requested with delimiter:"
             << delimiter;
    return true;
  }

  // Plain text mode
  if (cmd == "plain" || cmd == "text") {
    if (m_manager->currentIndex() >= 0) {
      Note note = m_manager->currentNote();
      m_manager->updateNoteMode(note.id(), NoteMode::PlainText);
      emit modeChanged(static_cast<int>(NoteMode::PlainText));
      qDebug() << "SlashCommand: Switched to Plain Text mode";
    }
    return true;
  }

  // Markdown mode
  if (cmd == "markdown" || cmd == "md") {
    if (m_manager->currentIndex() >= 0) {
      Note note = m_manager->currentNote();
      m_manager->updateNoteMode(note.id(), NoteMode::Markdown);
      emit modeChanged(static_cast<int>(NoteMode::Markdown));
      qDebug() << "SlashCommand: Switched to Markdown mode";
    }
    return true;
  }

  // Settings
  if (cmd == "settings") {
    emit openSettingsRequested();
    qDebug() << "SlashCommand: Settings requested";
    return true;
  }

  // Paste (toggle auto-paste) - placeholder
  if (cmd == "paste") {
    qDebug() << "SlashCommand: Paste toggle (not implemented yet)";
    return true;
  }

  // Timer commands - full parsing
  if (cmd.startsWith("timer")) {
    QString args = cmd.mid(5).trimmed(); // Everything after "timer"

    // Control commands: timer p, timer r, timer s
    if (args == "p" || args == "pause") {
      emit timerControl("pause");
      qDebug() << "SlashCommand: Timer pause/resume";
      return true;
    }
    if (args == "r" || args == "reset") {
      emit timerControl("reset");
      qDebug() << "SlashCommand: Timer reset";
      return true;
    }
    if (args == "s" || args == "stop") {
      emit timerControl("stop");
      qDebug() << "SlashCommand: Timer stop";
      return true;
    }

    // Explicit stopwatch: timer sw OR timer stopwatch
    if (args == "sw" || args == "stopwatch") {
      emit timerStartStopwatch();
      qDebug() << "SlashCommand: Stopwatch (explicit)";
      return true;
    }

    // Breathing exercise
    if (args == "breathe" || args == "breath") {
      emit timerStartBreathe();
      qDebug() << "SlashCommand: Breathe mode";
      return true;
    }

    // Pomodoro: timer pomo OR timer pomo:name OR timer 52 17
    if (args.startsWith("pomo")) {
      QString pomoArgs = args.mid(4).trimmed(); // After "pomo"
      QString pomoName;

      // Check for pomo:name format
      if (pomoArgs.startsWith(":")) {
        pomoName = pomoArgs.mid(1).trimmed();
      }

      // Use Settings for work/break times
      int workMin = Settings::instance()->pomodoroWorkMinutes();
      int breakMin = Settings::instance()->pomodoroBreakMinutes();
      emit timerStartPomodoro(workMin, breakMin);
      qDebug() << "SlashCommand: Pomodoro" << workMin << "/" << breakMin
               << "name:" << pomoName;
      return true;
    }

    // Custom pomodoro: timer 52 17
    QRegularExpression customPomoRx("^(\\d+)\\s+(\\d+)$");
    QRegularExpressionMatch pomoMatch = customPomoRx.match(args);
    if (pomoMatch.hasMatch()) {
      int workMin = pomoMatch.captured(1).toInt();
      int breakMin = pomoMatch.captured(2).toInt();
      emit timerStartPomodoro(workMin, breakMin);
      qDebug() << "SlashCommand: Custom Pomodoro" << workMin << "/" << breakMin;
      return true;
    }

    // Countdown: timer 5 OR timer 5s OR timer 5m OR timer 5:30 OR timer 5:30 :
    // laundry Format: number[s|m] OR minutes:seconds OR minutes:seconds : name
    // 5 = 5 minutes, 5m = 5 minutes, 5s = 5 seconds
    QRegularExpression countdownRx(
        "^(\\d+)(s|m)?(?::(\\d+))?(?:\\s*:\\s*(.+))?$");
    QRegularExpressionMatch cdMatch = countdownRx.match(args);
    if (cdMatch.hasMatch()) {
      int value = cdMatch.captured(1).toInt();
      QString suffix = cdMatch.captured(2).toLower();
      int extraSeconds =
          cdMatch.captured(3).isEmpty() ? 0 : cdMatch.captured(3).toInt();
      QString name = cdMatch.captured(4).trimmed();

      int totalSeconds;
      if (suffix == "s") {
        // timer 5s = 5 seconds
        totalSeconds = value;
      } else {
        // timer 5 or timer 5m = 5 minutes
        totalSeconds = value * 60 + extraSeconds;
      }

      emit timerStartCountdown(totalSeconds, name);
      qDebug() << "SlashCommand: Countdown" << totalSeconds
               << "sec, name:" << name;
      return true;
    }

    // Plain "timer" = stopwatch (if enabled in settings)
    if (args.isEmpty()) {
      if (Settings::instance()->timerAutoStartStopwatch()) {
        emit timerStartStopwatch();
        qDebug() << "SlashCommand: Stopwatch started";
      } else {
        qDebug() << "SlashCommand: Stopwatch auto-start disabled in settings";
      }
      return true;
    }

    // Fallback: switch to timer mode
    if (m_manager->currentIndex() >= 0) {
      Note note = m_manager->currentNote();
      m_manager->updateNoteMode(note.id(), NoteMode::Timer);
      emit modeChanged(static_cast<int>(NoteMode::Timer));
      qDebug() << "SlashCommand: Switched to Timer mode";
    }
    return true;
  }

  // OCR capture
  if (cmd == "ocr") {
    emit ocrRequested();
    qDebug() << "SlashCommand: OCR requested";
    return true;
  }

  return false;
}
