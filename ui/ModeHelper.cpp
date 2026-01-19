#include "ModeHelper.h"
#include "core/CurrencyConverter.h"
#include "core/Settings.h"
#include "core/UnitConverter.h"
#include <QDebug>
#include <QRegularExpression>
#include <QTextBlock>

// Static helper: Check if line is a keyword and return its color
static bool getKeywordColor(const QString &text, QColor &outColor) {
  QString line = text.trimmed();
  if (line.isEmpty())
    return false;

  QString checkLine = line;
  if (checkLine.startsWith('/')) {
    checkLine = checkLine.mid(1);
  }
  checkLine = checkLine.toLower();

  // Resolve alias
  Settings *s = Settings::instance();
  QMap<QString, QString> aliases = s->keywordAliases();
  QString resolved = checkLine;
  for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
    if (!it.value().isEmpty() && it.value().toLower() == checkLine) {
      resolved = it.key();
      break;
    }
  }

  // Keyword color mapping
  static QMap<QString, QString> colorMap = {
      {"list", "#a6e3a1"},     {"checklist", "#a6e3a1"},
      {"calc", "#89b4fa"},     {"sum", "#f5c2e7"},
      {"avg", "#cba6f7"},      {"count", "#fab387"},
      {"code", "#f9e2af"},     {"paste", "#94e2d5"},
      {"timer", "#f38ba8"},    {"plain", "#a6adc8"},
      {"text", "#a6adc8"},     {"settings", "#b4befe"},
      {"markdown", "#cba6f7"}, {"md", "#cba6f7"},
      {"ocr", "#94e2d5"}};

  if (colorMap.contains(resolved)) {
    outColor = QColor(colorMap[resolved]);
    return true;
  }
  if (colorMap.contains(checkLine)) {
    outColor = QColor(colorMap[checkLine]);
    return true;
  }
  return false;
}

// ============================================================================
// KeywordHighlighter
// ============================================================================

KeywordHighlighter::KeywordHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
  // Define keyword colors (Catppuccin Mocha palette)
  struct KeywordColor {
    QString keyword;
    QString color;
  };

  QList<KeywordColor> keywordColors = {
      {"list", "#a6e3a1"},      // Green
      {"checklist", "#a6e3a1"}, // Green
      {"calc", "#89b4fa"},      // Blue
      {"sum", "#f5c2e7"},       // Pink
      {"avg", "#cba6f7"},       // Mauve
      {"count", "#fab387"},     // Peach
      {"code", "#f9e2af"},      // Yellow
      {"paste", "#94e2d5"},     // Teal
      {"timer", "#f38ba8"},     // Red
      {"plain", "#a6adc8"},     // Overlay
      {"text", "#a6adc8"},      // Overlay
      {"settings", "#b4befe"}   // Lavender
  };

  for (const auto &kc : keywordColors) {
    KeywordFormat kf;
    kf.keyword = kc.keyword;
    kf.format.setForeground(QColor(kc.color));
    kf.format.setFontWeight(QFont::Bold);
    m_keywords.append(kf);
  }
}

void KeywordHighlighter::highlightBlock(const QString &text) {
  QString line = text.trimmed();

  // Skip empty lines
  if (line.isEmpty()) {
    return;
  }

  // Check if line is exactly a keyword (without or with /)
  QString checkLine = line;
  if (checkLine.startsWith('/')) {
    checkLine = checkLine.mid(1);
  }
  checkLine = checkLine.toLower();

  // Get aliases from settings
  Settings *s = Settings::instance();
  QMap<QString, QString> aliases = s->keywordAliases();

  // Check if the input matches an alias, resolve to original keyword
  QString resolvedKeyword = checkLine;
  for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
    if (!it.value().isEmpty() && it.value().toLower() == checkLine) {
      resolvedKeyword = it.key();
      break;
    }
  }

  for (const KeywordFormat &kf : m_keywords) {
    if (resolvedKeyword == kf.keyword || checkLine == kf.keyword) {
      // Highlight entire line
      setFormat(0, text.length(), kf.format);
      return;
    }
  }
}

// ============================================================================
// ChecklistHighlighter
// ============================================================================

ChecklistHighlighter::ChecklistHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent) {
  // Unchecked items: normal color
  m_uncheckedFormat.setForeground(QColor("#cdd6f4"));

  // Checked items: strikethrough and dimmed
  m_checkedFormat.setForeground(QColor("#6c7086"));
  m_checkedFormat.setFontStrikeOut(true);

  // Checkbox markers
  m_checkboxFormat.setForeground(QColor("#a6e3a1"));
  m_checkboxFormat.setFontWeight(QFont::Bold);

  // Comment format (// for comments)
  m_commentFormat.setForeground(QColor("#6c7086"));
  m_commentFormat.setFontItalic(true);

  // Heading format (# for headings)
  m_headingFormat.setForeground(QColor("#f5c2e7"));
  m_headingFormat.setFontWeight(QFont::Bold);
}

void ChecklistHighlighter::highlightBlock(const QString &text) {
  // Always check for keyword first
  QColor keywordColor;
  if (getKeywordColor(text, keywordColor)) {
    QTextCharFormat fmt;
    fmt.setForeground(keywordColor);
    fmt.setFontWeight(QFont::Bold);
    setFormat(0, text.length(), fmt);
    return;
  }

  QString trimmed = text.trimmed();

  // # Heading lines
  if (trimmed.startsWith('#')) {
    setFormat(0, text.length(), m_headingFormat);
    return;
  }

  // // Comment lines
  if (trimmed.startsWith("//")) {
    setFormat(0, text.length(), m_commentFormat);
    return;
  }

  // Pattern for checked items with /x at end: "text /x"
  static QRegularExpression checkTriggerPattern(
      R"(^(.*)\s+/x\s*$)", QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch triggerMatch = checkTriggerPattern.match(text);
  if (triggerMatch.hasMatch()) {
    // Treat as checked item
    setFormat(0, text.length(), m_checkedFormat);
    return;
  }

  // Pattern: "- [ ] text" or "- [x] text" or "- [X] text"
  static QRegularExpression uncheckedPattern(R"(^(\s*-\s*\[\s*\])\s*(.*)$)");
  static QRegularExpression checkedPattern(R"(^(\s*-\s*\[[xX]\])\s*(.*)$)");

  QRegularExpressionMatch checkedMatch = checkedPattern.match(text);
  if (checkedMatch.hasMatch()) {
    // Highlight checkbox
    setFormat(checkedMatch.capturedStart(1), checkedMatch.capturedLength(1),
              m_checkboxFormat);
    // Highlight text (strikethrough)
    setFormat(checkedMatch.capturedStart(2), checkedMatch.capturedLength(2),
              m_checkedFormat);
    return;
  }

  QRegularExpressionMatch uncheckedMatch = uncheckedPattern.match(text);
  if (uncheckedMatch.hasMatch()) {
    // Highlight checkbox
    setFormat(uncheckedMatch.capturedStart(1), uncheckedMatch.capturedLength(1),
              m_checkboxFormat);
    // Normal text format (already default)
  }
}

// ============================================================================
// MathHighlighter
// ============================================================================

MathHighlighter::MathHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), m_evaluator(nullptr) {
  m_numberFormat.setForeground(QColor("#fab387"));   // Orange for numbers
  m_operatorFormat.setForeground(QColor("#89b4fa")); // Blue for operators
  m_resultFormat.setForeground(QColor("#a6e3a1"));   // Green for results
  m_resultFormat.setFontWeight(QFont::Bold);
  m_currencyFormat.setForeground(QColor("#f5c2e7")); // Pink for currency
}

void MathHighlighter::setEvaluator(MathEvaluator *evaluator) {
  m_evaluator = evaluator;
}

void MathHighlighter::highlightBlock(const QString &text) {
  // Always check for keyword first
  QColor keywordColor;
  if (getKeywordColor(text, keywordColor)) {
    QTextCharFormat fmt;
    fmt.setForeground(keywordColor);
    fmt.setFontWeight(QFont::Bold);
    setFormat(0, text.length(), fmt);
    return;
  }

  // Highlight numbers
  static QRegularExpression numberPattern(R"(\b\d+(?:[.,]\d+)?\b)");
  QRegularExpressionMatchIterator numIt = numberPattern.globalMatch(text);
  while (numIt.hasNext()) {
    QRegularExpressionMatch match = numIt.next();
    setFormat(match.capturedStart(), match.capturedLength(), m_numberFormat);
  }

  // Highlight operators
  static QRegularExpression opPattern(R"([\+\-\*\/\%\^\=])");
  QRegularExpressionMatchIterator opIt = opPattern.globalMatch(text);
  while (opIt.hasNext()) {
    QRegularExpressionMatch match = opIt.next();
    setFormat(match.capturedStart(), match.capturedLength(), m_operatorFormat);
  }

  // Highlight currency codes (fiat + crypto)
  static QRegularExpression currencyPattern(
      R"(\b(USD|EUR|TRY|GBP|JPY|CNY|RUB|AUD|CAD|CHF|INR|KRW|BRL|MXN|PLN|SEK|NOK|DKK|SGD|HKD|NZD|ZAR|THB|AED|SAR|ALL|BTC|ETH|USDT|USDC|XRP|BNB|SOL|ADA|DOGE|TRX|DOT|MATIC|LTC)\b)",
      QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator currIt = currencyPattern.globalMatch(text);
  while (currIt.hasNext()) {
    QRegularExpressionMatch match = currIt.next();
    setFormat(match.capturedStart(), match.capturedLength(), m_currencyFormat);
  }

  // Highlight currency symbols (€$₺£¥₽₿Ξ)
  static QRegularExpression symbolPattern(R"([$€₺£¥₽₿Ξ])");
  QRegularExpressionMatchIterator symIt = symbolPattern.globalMatch(text);
  while (symIt.hasNext()) {
    QRegularExpressionMatch match = symIt.next();
    setFormat(match.capturedStart(), match.capturedLength(), m_currencyFormat);
  }

  // Highlight result comments (= result)
  static QRegularExpression resultPattern(R"(=\s*[\d.,]+\s*$)");
  QRegularExpressionMatch resultMatch = resultPattern.match(text);
  if (resultMatch.hasMatch()) {
    setFormat(resultMatch.capturedStart(), resultMatch.capturedLength(),
              m_resultFormat);
  }
}

// ============================================================================
// ModeHelper
// ============================================================================

ModeHelper::ModeHelper(QPlainTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor), m_mode(NoteMode::PlainText),
      m_keywordHighlighter(nullptr), m_checklistHighlighter(nullptr),
      m_mathHighlighter(nullptr) {
  // Always enable keyword highlighting
  m_keywordHighlighter = new KeywordHighlighter(m_editor->document());
}

void ModeHelper::setMode(NoteMode mode) {
  if (m_mode == mode)
    return;

  // CRITICAL: Disconnect highlighters from document BEFORE deleting
  // This prevents Qt from accessing deleted memory during rehighlight
  if (m_checklistHighlighter) {
    m_checklistHighlighter->setDocument(nullptr);
    delete m_checklistHighlighter;
    m_checklistHighlighter = nullptr;
  }
  if (m_mathHighlighter) {
    m_mathHighlighter->setDocument(nullptr);
    delete m_mathHighlighter;
    m_mathHighlighter = nullptr;
  }

  m_mode = mode;
  m_evaluator.clear();

  // Apply new highlighter based on mode
  switch (mode) {
  case NoteMode::Checklist:
    m_checklistHighlighter = new ChecklistHighlighter(m_editor->document());
    break;

  case NoteMode::Math:
    m_mathHighlighter = new MathHighlighter(m_editor->document());
    m_mathHighlighter->setEvaluator(&m_evaluator);
    break;

  case NoteMode::Code:
    // Basic code mode - could add syntax highlighting later
    break;

  case NoteMode::PlainText:
  default:
    // No special highlighting
    break;
  }

  qDebug() << "ModeHelper: Set mode to" << noteModeName(mode);
}

NoteMode ModeHelper::mode() const { return m_mode; }

void ModeHelper::handleEnterKey() {
  if (m_mode != NoteMode::Checklist)
    return;

  QTextCursor cursor = m_editor->textCursor();
  QString currentLine = cursor.block().text();

  // If current line starts with "- [ ]" or "- [x]", continue the pattern
  static QRegularExpression checklistPattern(R"(^(\s*-\s*\[[ xX]\])\s*)");
  QRegularExpressionMatch match = checklistPattern.match(currentLine);

  if (match.hasMatch()) {
    // Get the prefix whitespace
    QString prefix;
    int i = 0;
    while (i < currentLine.length() && currentLine[i].isSpace()) {
      prefix += currentLine[i];
      i++;
    }

    // Insert new checkbox item
    cursor.insertText("\n" + prefix + "- [ ] ");
  }
}

void ModeHelper::toggleCheckboxAtCursor() {
  if (m_mode != NoteMode::Checklist)
    return;

  QTextCursor cursor = m_editor->textCursor();
  cursor.select(QTextCursor::LineUnderCursor);
  QString line = cursor.selectedText();

  // Toggle [ ] <-> [x]
  if (line.contains("[ ]")) {
    line.replace("[ ]", "[x]");
  } else if (line.contains("[x]") || line.contains("[X]")) {
    line.replace(QRegularExpression(R"(\[[xX]\])"), "[ ]");
  } else {
    return; // Not a checkbox line
  }

  cursor.insertText(line);
}

QString ModeHelper::getMathResults() const {
  if (m_mode != NoteMode::Math)
    return QString();

  QString results;
  // Use mutable evaluator for proper variable tracking
  MathEvaluator &eval = const_cast<MathEvaluator &>(m_evaluator);

  QString text = m_editor->toPlainText();
  QStringList lines = text.split('\n');

  for (const QString &line : lines) {
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty() || trimmed == "---") {
      results += "\n";
      continue;
    }

    // Skip if already has result (= at end)
    static QRegularExpression resultPattern(R"(=\s*[\d.,]+\s*$)");
    if (resultPattern.match(trimmed).hasMatch()) {
      results += "\n";
      continue;
    }

    // Try unit conversion first (e.g., "5 km to mile")
    if (UnitConverter::instance()->isConversion(trimmed)) {
      QString unitResult = UnitConverter::instance()->convert(trimmed);
      if (!unitResult.isEmpty()) {
        results += QString(" = %1\n").arg(unitResult);
        continue;
      }
    }

    // Try currency conversion (e.g., "100 USD to EUR")
    double currencyResult;
    QString fromCur, toCur;
    if (CurrencyConverter::instance()->parseAndConvert(trimmed, currencyResult,
                                                       fromCur, toCur)) {
      results +=
          QString(" = %1 %2\n").arg(currencyResult, 0, 'f', 2).arg(toCur);
      continue;
    }

    // Try math evaluation (using member evaluator for variable support)
    bool ok;
    double mathResult = eval.evaluate(trimmed, &ok);
    if (ok) {
      // Format nicely
      if (mathResult == static_cast<int>(mathResult)) {
        results += QString(" = %1\n").arg(static_cast<int>(mathResult));
      } else {
        results += QString(" = %1\n").arg(mathResult, 0, 'f', 4);
      }
    } else {
      results += "\n";
    }
  }

  return results;
}

QString ModeHelper::calculateExpression(const QString &expression) {
  QString trimmed = expression.trimmed();

  if (trimmed.isEmpty()) {
    return QString();
  }

  // Try unit conversion first (e.g., "5 km to mile")
  if (UnitConverter::instance()->isConversion(trimmed)) {
    QString unitResult = UnitConverter::instance()->convert(trimmed);
    if (!unitResult.isEmpty()) {
      return unitResult;
    }
  }

  // Try currency conversion (e.g., "100 USD to EUR")
  double currencyResult;
  QString fromCur, toCur;
  if (CurrencyConverter::instance()->parseAndConvert(trimmed, currencyResult,
                                                     fromCur, toCur)) {
    return QString("%1 %2").arg(currencyResult, 0, 'f', 2).arg(toCur);
  }

  // Try math evaluation
  bool ok;
  double mathResult = m_evaluator.evaluate(trimmed, &ok);
  if (ok) {
    // Format nicely
    if (mathResult == static_cast<int>(mathResult)) {
      return QString::number(static_cast<int>(mathResult));
    } else {
      return QString::number(mathResult, 'f', 4);
    }
  }

  return QString();
}

QStringList ModeHelper::getVariables() const {
  return m_evaluator.getVariables();
}
