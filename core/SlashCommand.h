#ifndef LINNOTE_SLASHCOMMAND_H
#define LINNOTE_SLASHCOMMAND_H

#include <QObject>
#include <QString>
#include <QStringList>

class NoteManager;
class NoteEditor;

/**
 * @brief Handles slash commands and keyword commands in the editor
 *
 * Supported commands (with or without /):
 * - list/checklist - Switch to checklist mode
 * - code           - Switch to code mode
 * - calc/math      - Switch to calculator/math mode
 * - sum/avg/count  - Switch to math mode (specific functions)
 * - plain/text     - Switch to plain text mode
 * - settings       - Open settings dialog
 * - paste          - Toggle auto-paste
 * - timer          - Activate timer
 */
class SlashCommand : public QObject {
  Q_OBJECT

public:
  explicit SlashCommand(NoteManager *manager, QObject *parent = nullptr);

  /**
   * @brief Check if text is a command (with or without /)
   */
  bool isCommand(const QString &text) const;

  /**
   * @brief Check if text is a keyword (without /)
   */
  bool isKeyword(const QString &text) const;

  /**
   * @brief Get available command completions for partial input
   */
  QStringList getCompletions(const QString &partial) const;

  /**
   * @brief Get all keywords (for highlighting)
   */
  QStringList keywords() const;

  /**
   * @brief Execute a command
   * @return true if command was executed
   */
  bool execute(const QString &command);

signals:
  void openSettingsRequested();
  void ocrRequested();
  void modeChanged(int mode);
  void textAnalysisRequested(const QString &type); // sum, avg, count
  void autoPasteRequested(const QString &delimiter);
  // Timer signals
  void timerStartStopwatch();
  void timerStartCountdown(int seconds, const QString &name);
  void timerStartPomodoro(int workMin, int breakMin);
  void timerControl(const QString &action); // p=pause, r=reset, s=stop
  void timerStartBreathe();

private:
  NoteManager *m_manager;
  QStringList m_commands; // Commands with /
  QStringList m_keywords; // Keywords without /
};

#endif // LINNOTE_SLASHCOMMAND_H
