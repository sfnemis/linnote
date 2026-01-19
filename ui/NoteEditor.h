#ifndef LINNOTE_NOTEEDITOR_H
#define LINNOTE_NOTEEDITOR_H

#include "core/NoteMode.h"
#include <QPlainTextEdit>

class ModeHelper;
class QLabel;
class CommandPopup;
class MarkdownHighlighter;
class CodeHighlighter;
class Settings;
class QMimeData;

/**
 * @brief QPlainTextEdit wrapper with LinNote-specific features
 *
 * Provides a text editor with:
 * - Monospace font by default
 * - Mode-specific behavior (Checklist, Math, Code)
 * - AutoPaste support (via signal)
 * - Slash command popup (/ triggers command menu)
 * - Keyword tutorial tooltips
 */
class NoteEditor : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit NoteEditor(QWidget *parent = nullptr);
  ~NoteEditor() override;

  // Content helpers
  QString content() const;
  void setContent(const QString &content);
  void appendContent(const QString &text);

  // Mode support
  void setMode(NoteMode mode);
  NoteMode mode() const;

  // AutoPaste
  void pasteFromClipboard();

  // Tutorial tooltip
  void showKeywordTutorial(const QString &keyword);
  void hideTutorial();

  // Font settings (call when settings change)
  void applyFontSettings();

  // Text analysis (sum, avg, count)
  void performTextAnalysis(const QString &type);

  // AutoPaste mode
  void startAutoPaste(const QString &delimiter = "\n");
  void stopAutoPaste();
  bool isAutoPasteActive() const { return m_autoPasteActive; }

public slots:
  void onClipboardContent(const QString &text);

signals:
  void contentChanged();
  void checkboxToggled();
  void commandExecuted(const QString &command);
  void autoPasteStarted();
  void autoPasteStopped();

protected:
  void keyPressEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void insertFromMimeData(const QMimeData *source) override;
  QMimeData *createMimeDataFromSelection() const override;

private slots:
  void updateMathOverlay();
  void onCommandSelected(const QString &command);
  void onPopupDismissed();
  void checkForKeywordTutorial();
  void checkForAutoConversion();

private:
  void setupAppearance();
  void setupMathOverlay();
  void setupCommandPopup();
  void setupTutorialLabel();
  void showCommandPopup();
  void hideCommandPopup();
  void updateGhostText(); // Ghost text autocomplete
  void clearGhostText();
  QString cleanupPastedText(const QString &text, Settings *s);

  ModeHelper *m_modeHelper;
  MarkdownHighlighter *m_markdownHighlighter; // Markdown syntax highlighting
  CodeHighlighter *m_codeHighlighter;         // Code syntax highlighting
  QLabel *m_mathOverlay;                      // Legacy, kept for compatibility
  QLabel *m_tutorialLabel;                    // Shows keyword tutorials
  QLabel *m_ghostLabel;                       // Ghost text autocomplete
  NoteMode m_currentMode;
  CommandPopup *m_commandPopup;
  bool m_popupActive;
  QString m_lastTutorialKeyword;
  QString m_lastConvertedLine; // Prevent duplicate conversion
  int m_tutorialStartPos;      // Tutorial text start position
  int m_tutorialLength;        // Tutorial text length
  bool m_tutorialActive;       // Is tutorial text currently shown
  QString m_tutorialContent;   // Actual tutorial text content
  QString m_ghostCompletion;   // Current ghost text completion
  bool m_autoPasteActive = false;
  QString m_autoPasteDelimiter;
};

#endif // LINNOTE_NOTEEDITOR_H
