#ifndef LINNOTE_MARKDOWNHIGHLIGHTER_H
#define LINNOTE_MARKDOWNHIGHLIGHTER_H

#include <QMap>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

/**
 * @brief Syntax highlighter for Markdown formatting
 *
 * Highlights common Markdown elements:
 * - Headings (# ## ### etc)
 * - Bold (**text** or __text__)
 * - Italic (*text* or _text_)
 * - Code (`code` and ```blocks```)
 * - Links [text](url)
 * - Lists (- item, * item, 1. item)
 * - Blockquotes (> text)
 * - Strikethrough (~~text~~)
 */
class MarkdownHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
    int captureGroup = 0; // Which capture group to format (0 = whole match)
  };
  QVector<HighlightingRule> m_rules;

  // Formats
  QTextCharFormat m_headingFormat;
  QTextCharFormat m_boldFormat;
  QTextCharFormat m_italicFormat;
  QTextCharFormat m_codeFormat;
  QTextCharFormat m_linkFormat;
  QTextCharFormat m_listFormat;
  QTextCharFormat m_blockquoteFormat;
  QTextCharFormat m_strikethroughFormat;
  QMap<QString, QTextCharFormat> m_keywordFormats;

  void setupFormats();
  void setupRules();
  void setupKeywordHighlighting();
};

#endif // LINNOTE_MARKDOWNHIGHLIGHTER_H
