#ifndef LINNOTE_CODEHIGHLIGHTER_H
#define LINNOTE_CODEHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

/**
 * @brief Syntax highlighter for programming languages
 *
 * Supports: Python, JavaScript/TypeScript, C/C++, Bash, JSON
 */
class CodeHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  enum Language { Generic, Python, JavaScript, CPP, Bash, JSON };

  explicit CodeHighlighter(QTextDocument *parent = nullptr);

  void setLanguage(Language lang);
  Language language() const { return m_language; }

  // Auto-detect language from content
  static Language detectLanguage(const QString &text);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> m_rules;

  Language m_language;

  // Formats
  QTextCharFormat m_keywordFormat;
  QTextCharFormat m_typeFormat;
  QTextCharFormat m_functionFormat;
  QTextCharFormat m_stringFormat;
  QTextCharFormat m_numberFormat;
  QTextCharFormat m_commentFormat;
  QTextCharFormat m_preprocessorFormat;

  void setupFormats();
  void setupRules();
  void setupPythonRules();
  void setupJavaScriptRules();
  void setupCppRules();
  void setupBashRules();
  void setupJsonRules();
  void setupGenericRules();
};

#endif // LINNOTE_CODEHIGHLIGHTER_H
