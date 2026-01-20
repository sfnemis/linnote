#ifndef LINNOTE_MODEHELPER_H
#define LINNOTE_MODEHELPER_H

#include "core/MathEvaluator.h"
#include "core/NoteMode.h"
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextDocument>

class CurrencyConverter;

/**
 * @brief Syntax highlighter for command keywords
 * Highlights keywords like list, math, code in their respective colors
 */
class KeywordHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit KeywordHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct KeywordFormat {
    QString keyword;
    QTextCharFormat format;
  };
  QList<KeywordFormat> m_keywords;
};

/**
 * @brief Syntax highlighter for checklist mode
 */
class ChecklistHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit ChecklistHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  QTextCharFormat m_uncheckedFormat;
  QTextCharFormat m_checkedFormat;
  QTextCharFormat m_checkboxFormat;
  QTextCharFormat m_commentFormat; // // comments
  QTextCharFormat m_headingFormat; // # headings
};

/**
 * @brief Syntax highlighter for math mode
 * Shows calculation results inline
 */
class MathHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit MathHighlighter(QTextDocument *parent = nullptr);

  void setEvaluator(MathEvaluator *evaluator);

protected:
  void highlightBlock(const QString &text) override;

private:
  QTextCharFormat m_numberFormat;
  QTextCharFormat m_operatorFormat;
  QTextCharFormat m_resultFormat;
  QTextCharFormat m_currencyFormat;
  MathEvaluator *m_evaluator;
};

/**
 * @brief Helper class for mode-specific editor behavior
 */
class ModeHelper : public QObject {
  Q_OBJECT

public:
  explicit ModeHelper(QPlainTextEdit *editor, QObject *parent = nullptr);

  void setMode(NoteMode mode);
  NoteMode mode() const;

  // Called when Enter is pressed
  void handleEnterKey();

  // Called when a checkbox is clicked
  void toggleCheckboxAtCursor();

  // Get math results overlay text
  QString getMathResults() const;

  // Calculate a single expression and return result string
  QString calculateExpression(const QString &expression);

  // Get defined variable names (for autocomplete)
  QStringList getVariables() const;

private:
  QPlainTextEdit *m_editor;
  NoteMode m_mode;
  MathEvaluator m_evaluator;
  KeywordHighlighter *m_keywordHighlighter;
  ChecklistHighlighter *m_checklistHighlighter;
  MathHighlighter *m_mathHighlighter;
};

#endif // LINNOTE_MODEHELPER_H
