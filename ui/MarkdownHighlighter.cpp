#include "MarkdownHighlighter.h"
#include <QColor>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
  setupFormats();
  setupRules();
  setupKeywordHighlighting();
}

void MarkdownHighlighter::setupFormats() {
  // Heading format - larger, bold, accent color
  m_headingFormat.setForeground(QColor("#89b4fa")); // Blue accent
  m_headingFormat.setFontWeight(QFont::Bold);

  // Bold format
  m_boldFormat.setFontWeight(QFont::Bold);
  m_boldFormat.setForeground(QColor("#f5c2e7")); // Pink

  // Italic format
  m_italicFormat.setFontItalic(true);
  m_italicFormat.setForeground(QColor("#cba6f7")); // Mauve

  // Code format - monospace, background
  m_codeFormat.setForeground(QColor("#a6e3a1")); // Green
  m_codeFormat.setFontFamilies(QStringList{QStringLiteral("monospace")});

  // Link format
  m_linkFormat.setForeground(QColor("#74c7ec")); // Sapphire
  m_linkFormat.setFontUnderline(true);

  // List format
  m_listFormat.setForeground(QColor("#fab387")); // Peach

  // Blockquote format
  m_blockquoteFormat.setForeground(QColor("#9399b2")); // Overlay2
  m_blockquoteFormat.setFontItalic(true);

  // Strikethrough format
  m_strikethroughFormat.setFontStrikeOut(true);
  m_strikethroughFormat.setForeground(QColor("#6c7086")); // Overlay0
}

void MarkdownHighlighter::setupRules() {
  HighlightingRule rule;

  // === HEADINGS ===
  // # Heading 1, ## Heading 2, etc (must be at start of line)
  rule.pattern = QRegularExpression("^#{1,6}\\s+.*$");
  rule.format = m_headingFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === BOLD ===
  // **bold** or __bold__
  rule.pattern = QRegularExpression("\\*\\*(.+?)\\*\\*|__(.+?)__");
  rule.format = m_boldFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === ITALIC ===
  // *italic* or _italic_ (but not ** or __)
  rule.pattern = QRegularExpression("(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)|"
                                    "(?<!_)_(?!_)(.+?)(?<!_)_(?!_)");
  rule.format = m_italicFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === INLINE CODE ===
  // `code`
  rule.pattern = QRegularExpression("`([^`]+)`");
  rule.format = m_codeFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === LINKS ===
  // [text](url)
  rule.pattern = QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)");
  rule.format = m_linkFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === LISTS ===
  // - item, * item, + item, 1. item (at start of line)
  rule.pattern = QRegularExpression("^\\s*[-*+]\\s+|^\\s*\\d+\\.\\s+");
  rule.format = m_listFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === BLOCKQUOTES ===
  // > quote
  rule.pattern = QRegularExpression("^>\\s+.*$");
  rule.format = m_blockquoteFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === STRIKETHROUGH ===
  // ~~strikethrough~~
  rule.pattern = QRegularExpression("~~(.+?)~~");
  rule.format = m_strikethroughFormat;
  rule.captureGroup = 0;
  m_rules.append(rule);

  // === HORIZONTAL RULE ===
  // --- or *** or ___
  rule.pattern = QRegularExpression("^[-*_]{3,}\\s*$");
  rule.format = m_blockquoteFormat; // Reuse blockquote style
  rule.captureGroup = 0;
  m_rules.append(rule);
}

void MarkdownHighlighter::setupKeywordHighlighting() {
  // Define keyword colors matching CommandPopup (Catppuccin Mocha palette)
  QMap<QString, QString> keywordColors = {
      {"list", "#a6e3a1"},      // Green
      {"checklist", "#a6e3a1"}, // Green
      {"calc", "#89b4fa"},      // Blue
      {"sum", "#f5c2e7"},       // Pink
      {"avg", "#cba6f7"},       // Mauve
      {"count", "#fab387"},     // Peach
      {"code", "#f9e2af"},      // Yellow
      {"markdown", "#cba6f7"},  // Mauve
      {"md", "#cba6f7"},        // Mauve
      {"paste", "#94e2d5"},     // Teal
      {"timer", "#f38ba8"},     // Red
      {"plain", "#a6adc8"},     // Overlay
      {"text", "#a6adc8"},      // Overlay
      {"settings", "#b4befe"},  // Lavender
      {"ocr", "#74c7ec"}        // Sapphire
  };

  for (auto it = keywordColors.constBegin(); it != keywordColors.constEnd();
       ++it) {
    QTextCharFormat format;
    format.setForeground(QColor(it.value()));
    format.setFontWeight(QFont::Bold);
    m_keywordFormats[it.key()] = format;
  }
}

void MarkdownHighlighter::highlightBlock(const QString &text) {
  // Apply markdown rules
  for (const HighlightingRule &rule : m_rules) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      int start = match.capturedStart(rule.captureGroup);
      int length = match.capturedLength(rule.captureGroup);
      if (length > 0) {
        setFormat(start, length, rule.format);
      }
    }
  }

  // Highlight keywords (only if line is just the keyword)
  QString trimmed = text.trimmed().toLower();
  if (m_keywordFormats.contains(trimmed)) {
    // Find position of the keyword in the line
    int pos = text.indexOf(QRegularExpression(
        "\\b" + trimmed + "\\b", QRegularExpression::CaseInsensitiveOption));
    if (pos >= 0) {
      setFormat(pos, trimmed.length(), m_keywordFormats[trimmed]);
    }
  }
}
