#include "CodeHighlighter.h"
#include <QColor>

CodeHighlighter::CodeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent), m_language(Generic) {
  setupFormats();
  setupRules();
}

void CodeHighlighter::setupFormats() {
  // Keywords - Mauve
  m_keywordFormat.setForeground(QColor("#cba6f7"));
  m_keywordFormat.setFontWeight(QFont::Bold);

  // Types - Yellow
  m_typeFormat.setForeground(QColor("#f9e2af"));

  // Functions - Blue
  m_functionFormat.setForeground(QColor("#89b4fa"));

  // Strings - Green
  m_stringFormat.setForeground(QColor("#a6e3a1"));

  // Numbers - Peach
  m_numberFormat.setForeground(QColor("#fab387"));

  // Comments - Overlay0
  m_commentFormat.setForeground(QColor("#6c7086"));
  m_commentFormat.setFontItalic(true);

  // Preprocessor - Pink
  m_preprocessorFormat.setForeground(QColor("#f5c2e7"));
}

void CodeHighlighter::setLanguage(Language lang) {
  if (m_language != lang) {
    m_language = lang;
    setupRules();
    rehighlight();
  }
}

CodeHighlighter::Language CodeHighlighter::detectLanguage(const QString &text) {
  QString firstLine = text.section('\n', 0, 0).trimmed();

  // Shebang detection
  if (firstLine.startsWith("#!")) {
    if (firstLine.contains("python"))
      return Python;
    if (firstLine.contains("node") || firstLine.contains("deno"))
      return JavaScript;
    if (firstLine.contains("bash") || firstLine.contains("sh"))
      return Bash;
  }

  // Content-based detection
  if (text.contains("def ") || text.contains("import ") ||
      text.contains("from ") || text.contains("print("))
    return Python;

  if (text.contains("function ") || text.contains("const ") ||
      text.contains("let ") || text.contains("=>") || text.contains("async "))
    return JavaScript;

  if (text.contains("#include") || text.contains("int main") ||
      text.contains("void ") || text.contains("::"))
    return CPP;

  if (text.contains("#!/") || text.contains("echo ") ||
      text.contains("export ") || text.contains("fi\n"))
    return Bash;

  if (text.trimmed().startsWith("{") || text.trimmed().startsWith("["))
    return JSON;

  return Generic;
}

void CodeHighlighter::setupRules() {
  m_rules.clear();

  switch (m_language) {
  case Python:
    setupPythonRules();
    break;
  case JavaScript:
    setupJavaScriptRules();
    break;
  case CPP:
    setupCppRules();
    break;
  case Bash:
    setupBashRules();
    break;
  case JSON:
    setupJsonRules();
    break;
  default:
    setupGenericRules();
    break;
  }
}

void CodeHighlighter::setupPythonRules() {
  HighlightingRule rule;

  // Keywords
  QStringList keywords = {
      "and",      "as",     "assert", "async",    "await", "break",  "class",
      "continue", "def",    "del",    "elif",     "else",  "except", "False",
      "finally",  "for",    "from",   "global",   "if",    "import", "in",
      "is",       "lambda", "None",   "nonlocal", "not",   "or",     "pass",
      "raise",    "return", "True",   "try",      "while", "with",   "yield"};
  for (const QString &kw : keywords) {
    rule.pattern = QRegularExpression("\\b" + kw + "\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
  }

  // Functions
  rule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_]*(?=\\s*\\()");
  rule.format = m_functionFormat;
  m_rules.append(rule);

  // Numbers
  rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
  rule.format = m_numberFormat;
  m_rules.append(rule);

  // Strings (single and double quotes)
  rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'");
  rule.format = m_stringFormat;
  m_rules.append(rule);

  // Comments
  rule.pattern = QRegularExpression("#.*$");
  rule.format = m_commentFormat;
  m_rules.append(rule);
}

void CodeHighlighter::setupJavaScriptRules() {
  HighlightingRule rule;

  // Keywords
  QStringList keywords = {
      "async",  "await",    "break",   "case",       "catch",  "class",
      "const",  "continue", "default", "delete",     "do",     "else",
      "export", "extends",  "false",   "finally",    "for",    "function",
      "if",     "import",   "in",      "instanceof", "let",    "new",
      "null",   "of",       "return",  "static",     "super",  "switch",
      "this",   "throw",    "true",    "try",        "typeof", "undefined",
      "var",    "void",     "while",   "with",       "yield"};
  for (const QString &kw : keywords) {
    rule.pattern = QRegularExpression("\\b" + kw + "\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
  }

  // Arrow functions and functions
  rule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_]*(?=\\s*\\()");
  rule.format = m_functionFormat;
  m_rules.append(rule);

  // Numbers
  rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
  rule.format = m_numberFormat;
  m_rules.append(rule);

  // Strings
  rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'|`[^`]*`");
  rule.format = m_stringFormat;
  m_rules.append(rule);

  // Comments
  rule.pattern = QRegularExpression("//.*$");
  rule.format = m_commentFormat;
  m_rules.append(rule);
}

void CodeHighlighter::setupCppRules() {
  HighlightingRule rule;

  // Preprocessor
  rule.pattern = QRegularExpression("^\\s*#[a-z]+.*$");
  rule.format = m_preprocessorFormat;
  m_rules.append(rule);

  // Keywords
  QStringList keywords = {
      "alignas",  "alignof",   "and",     "and_eq",   "asm",       "auto",
      "bitand",   "bitor",     "bool",    "break",    "case",      "catch",
      "char",     "class",     "compl",   "const",    "constexpr", "continue",
      "default",  "delete",    "do",      "double",   "else",      "enum",
      "explicit", "export",    "extern",  "false",    "float",     "for",
      "friend",   "goto",      "if",      "inline",   "int",       "long",
      "mutable",  "namespace", "new",     "noexcept", "not",       "not_eq",
      "nullptr",  "operator",  "or",      "or_eq",    "private",   "protected",
      "public",   "register",  "return",  "short",    "signed",    "sizeof",
      "static",   "struct",    "switch",  "template", "this",      "throw",
      "true",     "try",       "typedef", "typeid",   "typename",  "union",
      "unsigned", "using",     "virtual", "void",     "volatile",  "while",
      "xor",      "xor_eq"};
  for (const QString &kw : keywords) {
    rule.pattern = QRegularExpression("\\b" + kw + "\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
  }

  // Types
  QStringList types = {"QString", "QObject", "QWidget", "QList", "QVector",
                       "QMap",    "QHash",   "QSet",    "QPair", "std"};
  for (const QString &t : types) {
    rule.pattern = QRegularExpression("\\b" + t + "\\b");
    rule.format = m_typeFormat;
    m_rules.append(rule);
  }

  // Functions
  rule.pattern = QRegularExpression("\\b[a-zA-Z_][a-zA-Z0-9_]*(?=\\s*\\()");
  rule.format = m_functionFormat;
  m_rules.append(rule);

  // Numbers
  rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*[fFlL]?\\b");
  rule.format = m_numberFormat;
  m_rules.append(rule);

  // Strings
  rule.pattern = QRegularExpression("\"[^\"]*\"");
  rule.format = m_stringFormat;
  m_rules.append(rule);

  // Comments
  rule.pattern = QRegularExpression("//.*$");
  rule.format = m_commentFormat;
  m_rules.append(rule);
}

void CodeHighlighter::setupBashRules() {
  HighlightingRule rule;

  // Shebang
  rule.pattern = QRegularExpression("^#!.*$");
  rule.format = m_preprocessorFormat;
  m_rules.append(rule);

  // Keywords
  QStringList keywords = {"if",   "then",  "else",   "elif",     "fi",
                          "case", "esac",  "for",    "in",       "do",
                          "done", "while", "until",  "function", "return",
                          "exit", "local", "export", "source",   "readonly"};
  for (const QString &kw : keywords) {
    rule.pattern = QRegularExpression("\\b" + kw + "\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
  }

  // Variables
  rule.pattern = QRegularExpression("\\$[a-zA-Z_][a-zA-Z0-9_]*|\\$\\{[^}]+\\}");
  rule.format = m_typeFormat;
  m_rules.append(rule);

  // Numbers
  rule.pattern = QRegularExpression("\\b\\d+\\b");
  rule.format = m_numberFormat;
  m_rules.append(rule);

  // Strings
  rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'");
  rule.format = m_stringFormat;
  m_rules.append(rule);

  // Comments
  rule.pattern = QRegularExpression("#.*$");
  rule.format = m_commentFormat;
  m_rules.append(rule);
}

void CodeHighlighter::setupJsonRules() {
  HighlightingRule rule;

  // Keys
  rule.pattern = QRegularExpression("\"[^\"]+\"(?=\\s*:)");
  rule.format = m_functionFormat;
  m_rules.append(rule);

  // String values
  rule.pattern = QRegularExpression(":\\s*\"[^\"]*\"");
  rule.format = m_stringFormat;
  m_rules.append(rule);

  // Numbers
  rule.pattern = QRegularExpression(":\\s*-?\\d+\\.?\\d*");
  rule.format = m_numberFormat;
  m_rules.append(rule);

  // Booleans and null
  rule.pattern = QRegularExpression("\\b(true|false|null)\\b");
  rule.format = m_keywordFormat;
  m_rules.append(rule);
}

void CodeHighlighter::setupGenericRules() {
  HighlightingRule rule;

  // Generic keywords
  QStringList keywords = {"if",    "else", "for",      "while", "return",
                          "class", "def",  "function", "const", "let",
                          "var",   "int",  "void",     "true",  "false"};
  for (const QString &kw : keywords) {
    rule.pattern = QRegularExpression("\\b" + kw + "\\b");
    rule.format = m_keywordFormat;
    m_rules.append(rule);
  }

  // Numbers
  rule.pattern = QRegularExpression("\\b\\d+\\.?\\d*\\b");
  rule.format = m_numberFormat;
  m_rules.append(rule);

  // Strings
  rule.pattern = QRegularExpression("\"[^\"]*\"|'[^']*'");
  rule.format = m_stringFormat;
  m_rules.append(rule);

  // Comments (// and #)
  rule.pattern = QRegularExpression("(//|#).*$");
  rule.format = m_commentFormat;
  m_rules.append(rule);
}

void CodeHighlighter::highlightBlock(const QString &text) {
  for (const HighlightingRule &rule : m_rules) {
    QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}
