#include "NoteEditor.h"
#include "CodeHighlighter.h"
#include "CommandPopup.h"
#include "MarkdownHighlighter.h"
#include "ModeHelper.h"
#include "core/CurrencyConverter.h"
#include "core/Settings.h"
#include "core/TextAnalyzer.h"
#include "core/UrlShortener.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QFont>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBlock>
#include <QUrl>

NoteEditor::NoteEditor(QWidget *parent)
    : QPlainTextEdit(parent), m_modeHelper(new ModeHelper(this, this)),
      m_markdownHighlighter(nullptr), m_codeHighlighter(nullptr),
      m_mathOverlay(nullptr), m_tutorialLabel(nullptr), m_ghostLabel(nullptr),
      m_currentMode(NoteMode::PlainText), m_commandPopup(nullptr),
      m_popupActive(false), m_tutorialStartPos(-1), m_tutorialLength(0),
      m_tutorialActive(false) {
  setupAppearance();
  setupMathOverlay();
  setupTutorialLabel();
  setupCommandPopup();

  // Setup ghost text label for autocomplete (child of viewport for correct
  // positioning)
  m_ghostLabel = new QLabel(viewport());
  m_ghostLabel->setStyleSheet(
      "color: rgba(166, 173, 200, 0.5); background: transparent;");
  m_ghostLabel->hide();

  // Setup Markdown syntax highlighting
  m_markdownHighlighter = new MarkdownHighlighter(document());

  // Forward text changes
  connect(this, &QPlainTextEdit::textChanged, this,
          &NoteEditor::contentChanged);
  connect(this, &QPlainTextEdit::textChanged, this,
          &NoteEditor::updateMathOverlay);
  connect(this, &QPlainTextEdit::textChanged, this,
          &NoteEditor::checkForKeywordTutorial);
  connect(this, &QPlainTextEdit::textChanged, this,
          &NoteEditor::updateGhostText);
}

NoteEditor::~NoteEditor() {}

void NoteEditor::setupAppearance() {
  // Apply font from Settings
  applyFontSettings();

  // Placeholder text
  setPlaceholderText(tr("Start typing... (try: list, calc, code, timer)"));

  // Tab settings
  setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);

  // Line wrap
  setLineWrapMode(QPlainTextEdit::WidgetWidth);

  // Minimal margins
  setContentsMargins(4, 4, 4, 4);

  // Remove frame for cleaner look
  setFrameShape(QFrame::NoFrame);

  // Make the entire scroll area transparent
  // This ensures the scrollbar track doesn't have a solid background
  QPalette pal = palette();
  pal.setBrush(QPalette::Base, Qt::transparent);
  setPalette(pal);

  // Ensure viewport and scrollbars are transparent
  viewport()->setAutoFillBackground(false);
  setAutoFillBackground(false);
  setAttribute(Qt::WA_TranslucentBackground, true);

  // Set horizontal scrollbar policy to always off (we use word wrap)
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // Set viewport margins to 0 - no extra space around viewport
  setViewportMargins(0, 0, 0, 0);

  // Apply transparent style directly to the vertical scrollbar widget
  // CRITICAL: Must include groove styling for the scrollbar track area
  verticalScrollBar()->setAutoFillBackground(false);
  verticalScrollBar()->setAttribute(Qt::WA_TranslucentBackground, true);
  verticalScrollBar()->setStyleSheet(R"(
    QScrollBar:vertical {
      background: transparent;
      width: 6px;
      margin: 0;
      padding: 0;
      border: none;
    }
    QScrollBar::groove:vertical {
      background: transparent;
      border: none;
      width: 6px;
    }
    QScrollBar::handle:vertical {
      background: rgba(128, 128, 128, 0.4);
      border-radius: 3px;
      min-height: 30px;
    }
    QScrollBar::handle:vertical:hover {
      background: rgba(128, 128, 128, 0.7);
    }
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
      background: transparent;
      height: 0px;
      border: none;
    }
    QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
      background: transparent;
      border: none;
    }
  )");

  // Also apply to the NoteEditor itself to ensure all scrollbar areas are
  // transparent
  setStyleSheet(styleSheet() + R"(
    NoteEditor {
      background: transparent;
    }
  )");
}

void NoteEditor::applyFontSettings() {
  Settings *s = Settings::instance();
  QString family = s->fontFamily();
  int size = s->fontSize();

  QFont editorFont(family, size);
  setFont(editorFont);

  // Update tab stop for new font
  setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);

  // Force visual update
  update();
  qDebug() << "NoteEditor: Applied font" << family << size << "pt";
}

void NoteEditor::performTextAnalysis(const QString &type) {
  // Get current text (excluding the command line itself)
  QString text = toPlainText();

  // Find and remove the command line (first line if it's the command)
  QStringList lines = text.split('\n');
  if (!lines.isEmpty()) {
    QString firstLine = lines.first().trimmed().toLower();
    if (firstLine == type || firstLine == "/" + type) {
      lines.removeFirst();
      text = lines.join('\n');
    }
  }

  TextAnalyzer analyzer;
  QString result;

  if (type == "sum") {
    result = analyzer.formatSum(text);
  } else if (type == "avg") {
    result = analyzer.formatAvg(text);
  } else if (type == "count") {
    result = analyzer.formatCount(text);
  }

  // Append result to end of text
  if (!result.isEmpty()) {
    moveCursor(QTextCursor::End);
    insertPlainText(result);
  }

  qDebug() << "NoteEditor: Text analysis" << type << "completed";
}

void NoteEditor::startAutoPaste(const QString &delimiter) {
  m_autoPasteActive = true;
  m_autoPasteDelimiter = delimiter;

  // Clear editor and show tutorial
  setPlainText("");

  // Show tutorial text
  QString tutorial =
      QString("━━━ AutoPaste Mode ━━━\n\n"
              "Clipboard monitoring started!\n"
              "Everything you copy will be added here.\n\n"
              "Press ESC to stop.\n\n"
              "Delimiter: %1")
          .arg(delimiter == "\n" ? "New line" : "\"" + delimiter + "\"");

  QTextCharFormat tutorialFormat;
  tutorialFormat.setForeground(QColor(90, 90, 100, 150));
  QTextCursor cursor = textCursor();
  cursor.insertText(tutorial, tutorialFormat);
  moveCursor(QTextCursor::End);

  emit autoPasteStarted();
  qDebug() << "NoteEditor: AutoPaste started with delimiter:" << delimiter;
}

void NoteEditor::stopAutoPaste() {
  if (!m_autoPasteActive) {
    return;
  }

  m_autoPasteActive = false;
  m_autoPasteDelimiter.clear();

  emit autoPasteStopped();
  qDebug() << "NoteEditor: AutoPaste stopped";
}

void NoteEditor::onClipboardContent(const QString &text) {
  if (!m_autoPasteActive || text.isEmpty()) {
    return;
  }

  // Move to end and append with delimiter
  moveCursor(QTextCursor::End);

  // If there's already content, add delimiter first
  QString currentText = toPlainText();
  if (!currentText.isEmpty() && !currentText.endsWith(m_autoPasteDelimiter)) {
    insertPlainText(m_autoPasteDelimiter);
  }

  insertPlainText(text);

  qDebug() << "NoteEditor: AutoPaste appended:" << text.left(30);
}

void NoteEditor::setupMathOverlay() {
  // Math overlay shows results on the right side
  m_mathOverlay = new QLabel(this);
  m_mathOverlay->setStyleSheet(R"(
    QLabel {
      color: #a6e3a1;
      background: transparent;
      font-family: monospace;
      font-size: 11pt;
    }
  )");
  m_mathOverlay->setAlignment(Qt::AlignTop | Qt::AlignRight);
  m_mathOverlay->hide();
}

QString NoteEditor::content() const { return toPlainText(); }

void NoteEditor::setContent(const QString &content) { setPlainText(content); }

void NoteEditor::appendContent(const QString &text) {
  if (text.isEmpty()) {
    return;
  }

  // If editor is empty, just set the text
  if (toPlainText().isEmpty()) {
    setPlainText(text);
  } else {
    // Append with newline separator
    appendPlainText(text);
  }

  // Move cursor to end
  moveCursor(QTextCursor::End);
}

void NoteEditor::setMode(NoteMode mode) {
  m_currentMode = mode;
  m_modeHelper->setMode(mode);

  // Math overlay no longer needed - we have inline results with =
  m_mathOverlay->hide();

  // Update placeholder based on mode
  switch (mode) {
  case NoteMode::Checklist:
    setPlaceholderText(tr("- [ ] Your first task\n- [ ] Another task"));
    break;
  case NoteMode::Math:
    setPlaceholderText(tr("Try: 2 + 3\nx = 10\nx * 5\n100 USD to TRY"));
    break;
  case NoteMode::Code: {
    setPlaceholderText(tr("// Paste your code here..."));
    // Activate code highlighting
    if (!m_codeHighlighter) {
      m_codeHighlighter = new CodeHighlighter(document());
    }
    // Detect language and apply highlighting
    QString text = toPlainText();
    auto lang = CodeHighlighter::detectLanguage(text);
    m_codeHighlighter->setLanguage(lang);
    break;
  }
  case NoteMode::Markdown: {
    setPlaceholderText(tr("# Heading\n**bold** *italic*\n- list item"));
    // Activate markdown highlighting
    if (!m_markdownHighlighter) {
      m_markdownHighlighter = new MarkdownHighlighter(document());
    }
    break;
  }
  default:
    setPlaceholderText(tr("Start typing... (try: list, calc, code, md)"));
    break;
  }
}

NoteMode NoteEditor::mode() const { return m_currentMode; }

void NoteEditor::pasteFromClipboard() {
  QClipboard *clipboard = QApplication::clipboard();
  QString text = clipboard->text();

  if (!text.isEmpty()) {
    // Apply paste cleanup based on settings
    Settings *s = Settings::instance();
    text = cleanupPastedText(text, s);
    appendContent(text);
  }
}

QString NoteEditor::cleanupPastedText(const QString &text, Settings *s) {
  if (!s)
    return text;

  qDebug() << "cleanupPastedText: Input:" << text.left(100);
  qDebug() << "cleanupPastedText: Settings - spaces:"
           << s->pasteRemoveLeadingSpaces()
           << "numbers:" << s->pasteRemoveNumbers()
           << "bullets:" << s->pasteRemoveBullets()
           << "markdown:" << s->pasteRemoveMarkdown()
           << "empty:" << s->pasteRemoveEmptyLines();

  QStringList lines = text.split('\n');
  QStringList result;

  for (QString line : lines) {
    // Remove empty lines (check first before other modifications)
    if (s->pasteRemoveEmptyLines() && line.trimmed().isEmpty()) {
      qDebug() << "cleanupPastedText: Skipping empty line";
      continue;
    }

    // Remove leading spaces/tabs
    if (s->pasteRemoveLeadingSpaces()) {
      line = line.trimmed();
    }

    // Remove leading numbers (1. 2. 3. etc)
    if (s->pasteRemoveNumbers()) {
      static QRegularExpression numPattern(R"(^\s*\d+[\.\)]\s*)");
      line.remove(numPattern);
    }

    // Remove bullets (-, *, •, etc)
    if (s->pasteRemoveBullets()) {
      static QRegularExpression bulletPattern(R"(^\s*[-\*•◦▪▸►→]\s*)");
      line.remove(bulletPattern);
    }

    // Remove markdown formatting
    if (s->pasteRemoveMarkdown()) {
      // Bold **text** -> text
      static QRegularExpression boldPattern(R"(\*\*([^\*]+)\*\*)");
      QRegularExpressionMatch m = boldPattern.match(line);
      while (m.hasMatch()) {
        line.replace(m.captured(0), m.captured(1));
        m = boldPattern.match(line);
      }
      // Italic _text_ -> text (single underscore)
      static QRegularExpression underscorePattern(R"(\b_([^_]+)_\b)");
      m = underscorePattern.match(line);
      while (m.hasMatch()) {
        line.replace(m.captured(0), m.captured(1));
        m = underscorePattern.match(line);
      }
      // Inline code `text` -> text
      static QRegularExpression codePattern(R"(`([^`]+)`)");
      m = codePattern.match(line);
      while (m.hasMatch()) {
        line.replace(m.captured(0), m.captured(1));
        m = codePattern.match(line);
      }
      // Links [text](url) -> text
      static QRegularExpression linkPattern(R"(\[([^\]]+)\]\([^\)]+\))");
      m = linkPattern.match(line);
      while (m.hasMatch()) {
        line.replace(m.captured(0), m.captured(1));
        m = linkPattern.match(line);
      }
      // Headings # ## ### -> remove
      static QRegularExpression headingPattern(R"(^#{1,6}\s*)");
      line.remove(headingPattern);
    }

    result.append(line);
  }

  QString output = result.join('\n');
  qDebug() << "cleanupPastedText: Output:" << output.left(100);
  return output;
}

void NoteEditor::keyPressEvent(QKeyEvent *event) {
  // Stop AutoPaste on ESC
  if (event->key() == Qt::Key_Escape && m_autoPasteActive) {
    stopAutoPaste();
    event->accept();
    return;
  }

  // Auto-delete tutorial text when user presses ANY key (except Escape)
  if (m_tutorialActive && m_tutorialStartPos >= 0 && m_tutorialLength > 0) {
    // Delete on any key press except Escape and arrow keys
    if (event->key() != Qt::Key_Escape && event->key() != Qt::Key_Up &&
        event->key() != Qt::Key_Down && event->key() != Qt::Key_Left &&
        event->key() != Qt::Key_Right) {

      // Block signals to prevent re-triggering
      blockSignals(true);

      // Delete tutorial text
      QTextCursor cursor = textCursor();
      cursor.setPosition(m_tutorialStartPos);
      cursor.setPosition(m_tutorialStartPos + m_tutorialLength,
                         QTextCursor::KeepAnchor);
      cursor.removeSelectedText();

      // Position cursor at end of keyword (where tutorial was removed)
      // DON'T add newline - let user continue typing on same line!
      cursor.setPosition(m_tutorialStartPos);
      setTextCursor(cursor);

      m_tutorialActive = false;
      m_tutorialStartPos = -1;
      m_tutorialLength = 0;
      m_tutorialContent.clear();
      // Don't clear m_lastTutorialKeyword - prevents tutorial from re-appearing

      blockSignals(false);

      // Don't process backspace/delete further (just delete tutorial)
      if (event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete) {
        return;
      }
    }
  }

  // Code mode: Bracket auto-close
  if (m_currentMode == NoteMode::Code) {
    QString ch = event->text();
    QTextCursor cursor = textCursor();

    // Auto-close brackets and quotes
    static const QMap<QString, QString> pairs = {
        {"(", ")"}, {"[", "]"}, {"{", "}"}, {"\"", "\""}, {"'", "'"}};

    if (pairs.contains(ch)) {
      QPlainTextEdit::keyPressEvent(event);
      cursor = textCursor();
      cursor.insertText(pairs[ch]);
      cursor.movePosition(QTextCursor::Left);
      setTextCursor(cursor);
      return;
    }

    // Skip closing bracket if next char is same
    if (ch == ")" || ch == "]" || ch == "}" || ch == "\"" || ch == "'") {
      QString lineText = cursor.block().text();
      int pos = cursor.positionInBlock();
      if (pos < lineText.length() && lineText[pos] == ch[0]) {
        cursor.movePosition(QTextCursor::Right);
        setTextCursor(cursor);
        return;
      }
    }

    // Smart backspace: delete both brackets if cursor is between empty pair
    if (event->key() == Qt::Key_Backspace) {
      QString lineText = cursor.block().text();
      int pos = cursor.positionInBlock();
      if (pos > 0 && pos < lineText.length()) {
        QChar prev = lineText[pos - 1];
        QChar next = lineText[pos];
        static const QMap<QChar, QChar> matchingPairs = {
            {'(', ')'}, {'[', ']'}, {'{', '}'}, {'"', '"'}, {'\'', '\''}};
        if (matchingPairs.contains(prev) && matchingPairs[prev] == next) {
          cursor.deletePreviousChar();
          cursor.deleteChar();
          setTextCursor(cursor);
          return;
        }
      }
    }

    // Auto-indent after { or :
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
      QString lineText = cursor.block().text();
      QString trimmed = lineText.trimmed();
      if (trimmed.endsWith("{") || trimmed.endsWith(":")) {
        // Get current indentation
        QString indent;
        for (QChar c : lineText) {
          if (c == ' ' || c == '\t')
            indent += c;
          else
            break;
        }
        QPlainTextEdit::keyPressEvent(event);
        cursor = textCursor();
        cursor.insertText(indent + "    "); // Add 4 spaces
        setTextCursor(cursor);
        return;
      }
    }
  }

  // Handle '=' key in Math mode - calculate and insert result
  if (event->text() == "=" && m_currentMode == NoteMode::Math) {
    QTextCursor cursor = textCursor();
    QString lineText = cursor.block().text();
    QString expression = lineText.trimmed();

    // Only pure math needs = key - skip if it's a unit/currency conversion
    // (conversions will auto-calculate on Enter)
    if (!expression.isEmpty() && !expression.contains("=")) {
      // Check if this is a conversion (has "to" keyword) - skip, let Enter
      // handle it
      if (expression.contains(" to ", Qt::CaseInsensitive)) {
        // Just insert the = normally, don't auto-calculate
        QPlainTextEdit::keyPressEvent(event);
        return;
      }

      // Pure math expression - calculate on =
      QPlainTextEdit::keyPressEvent(event);

      // Calculate result using ModeHelper
      QString result = m_modeHelper->calculateExpression(expression);
      if (!result.isEmpty()) {
        cursor = textCursor();
        cursor.insertText(" " + result);
        setTextCursor(cursor);
      }
      return;
    }
  }

  // Show command popup on '/' key (only at the start of a line or after space)
  if (event->text() == "/") {
    QTextCursor cursor = textCursor();
    QString lineText = cursor.block().text();
    int posInLine = cursor.positionInBlock();

    // Only trigger at start of line or empty line
    bool atLineStart =
        (posInLine == 0) || lineText.left(posInLine).trimmed().isEmpty();
    if (atLineStart) {
      // Insert the '/' first, then show popup
      QPlainTextEdit::keyPressEvent(event);
      showCommandPopup();
      return;
    }
  }

  // Handle Enter key for checklist mode and Math mode conversions
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    // Math mode - evaluate expressions (including variable definitions)
    if (m_currentMode == NoteMode::Math) {
      QTextCursor cursor = textCursor();
      QString currentLine = cursor.block().text().trimmed();

      // Skip empty lines or lines that already have result suffix (= result)
      static QRegularExpression resultSuffix(R"(=\s*[\d.,]+\s*$)");
      if (!currentLine.isEmpty() &&
          !resultSuffix.match(currentLine).hasMatch()) {
        // Check if this is a conversion (unit or currency)
        if (currentLine.contains(" to ", Qt::CaseInsensitive)) {
          QString result = m_modeHelper->calculateExpression(currentLine);
          if (!result.isEmpty()) {
            cursor.movePosition(QTextCursor::EndOfBlock);
            setTextCursor(cursor);
            cursor.insertText(" = " + result);
            setTextCursor(cursor);
            QPlainTextEdit::keyPressEvent(event);
            return;
          }
        }

        // Check if this is a variable assignment (name = value or name: value)
        static QRegularExpression varAssign(
            R"(^([a-zA-Z_]\w*)\s*[:=]\s*(.+)$)");
        QRegularExpressionMatch varMatch = varAssign.match(currentLine);
        if (varMatch.hasMatch()) {
          // Evaluate to store the variable
          m_modeHelper->calculateExpression(currentLine);
          // Just proceed with newline, no result display for assignments
          QPlainTextEdit::keyPressEvent(event);
          return;
        }

        // Regular math expression - evaluate and show result
        QString result = m_modeHelper->calculateExpression(currentLine);
        if (!result.isEmpty()) {
          cursor.movePosition(QTextCursor::EndOfBlock);
          setTextCursor(cursor);
          cursor.insertText(" = " + result);
          setTextCursor(cursor);
        }
      }
      QPlainTextEdit::keyPressEvent(event);
      return;
    }

    if (m_currentMode == NoteMode::Checklist) {
      // Check if auto-checkbox is enabled in settings
      if (!Settings::instance()->autoCheckboxEnabled()) {
        // Just insert newline without checkbox
        QPlainTextEdit::keyPressEvent(event);
        return;
      }

      QTextCursor cursor = textCursor();
      QString currentLine = cursor.block().text();

      // Check if current line already has a checkbox
      static QRegularExpression checklistPattern(R"(^(\s*)-\s*\[[ xX]\]\s*)");
      QRegularExpressionMatch match = checklistPattern.match(currentLine);

      if (match.hasMatch()) {
        // Continue with same indentation
        QString prefix = match.captured(1);
        cursor.insertText("\n" + prefix + "- [ ] ");
        setTextCursor(cursor);
        return;
      } else {
        // First checkbox or non-checkbox line - always add checkbox in
        // checklist mode
        cursor.insertText("\n- [ ] ");
        setTextCursor(cursor);
        return;
      }
    }
  }

  // Handle space to toggle checkbox
  if (event->key() == Qt::Key_Space && m_currentMode == NoteMode::Checklist) {
    QTextCursor cursor = textCursor();
    int posInLine = cursor.positionInBlock();
    QString lineText = cursor.block().text();

    // If cursor is near checkbox marker, toggle it
    static QRegularExpression checkboxPos(R"(\[[ xX]\])");
    QRegularExpressionMatch match = checkboxPos.match(lineText);
    if (match.hasMatch() && posInLine >= match.capturedStart() &&
        posInLine <= match.capturedEnd()) {
      m_modeHelper->toggleCheckboxAtCursor();
      emit checkboxToggled();
      return;
    }
  }

  // Calc mode: Tab for variable autocomplete (use ghost completion if
  // available)
  if (m_currentMode == NoteMode::Math && event->key() == Qt::Key_Tab) {
    if (!m_ghostCompletion.isEmpty()) {
      // Get current prefix and replace with full completion
      QTextCursor cursor = textCursor();
      QString lineText = cursor.block().text();
      int posInLine = cursor.positionInBlock();

      // Find word start
      int wordStart = posInLine;
      while (wordStart > 0 && (lineText[wordStart - 1].isLetterOrNumber() ||
                               lineText[wordStart - 1] == '_')) {
        wordStart--;
      }

      QString prefix = lineText.mid(wordStart, posInLine - wordStart);
      // Replace prefix with full completion
      cursor.setPosition(cursor.position() - prefix.length());
      cursor.setPosition(cursor.position() + prefix.length(),
                         QTextCursor::KeepAnchor);
      cursor.insertText(m_ghostCompletion);
      setTextCursor(cursor);
      clearGhostText();
      return;
    }
    // No ghost completion - insert normal tab
    QPlainTextEdit::keyPressEvent(event);
    return;
  }

  // Keep indentation on Enter (for non-special modes)
  if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
      Settings::instance()->keepIndentationOnEnter()) {
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text();

    // Extract leading whitespace
    QString indent;
    for (const QChar &ch : currentLine) {
      if (ch == ' ' || ch == '\t') {
        indent += ch;
      } else {
        break;
      }
    }

    // If there's indentation to preserve, insert it after newline
    if (!indent.isEmpty()) {
      cursor.insertText("\n" + indent);
      setTextCursor(cursor);
      return;
    }
  }

  // Default processing
  QPlainTextEdit::keyPressEvent(event);
}

void NoteEditor::mousePressEvent(QMouseEvent *event) {
  // Ctrl+Click to open URLs
  if (event->button() == Qt::LeftButton &&
      event->modifiers() & Qt::ControlModifier) {
    QTextCursor cursor = cursorForPosition(event->pos());
    QString lineText = cursor.block().text();

    // Find URL in line
    static QRegularExpression urlPattern(R"((https?://|www\.)[^\s<>\[\]]+)");
    QRegularExpressionMatchIterator it = urlPattern.globalMatch(lineText);

    int posInLine = cursor.positionInBlock();
    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      if (posInLine >= match.capturedStart() &&
          posInLine <= match.capturedEnd()) {
        QString url = match.captured();
        if (url.startsWith("www.")) {
          url = "https://" + url;
        }
        QDesktopServices::openUrl(QUrl(url));
        return;
      }
    }
  }

  // Handle click on checkbox
  if (m_currentMode == NoteMode::Checklist &&
      event->button() == Qt::LeftButton) {
    QTextCursor cursor = cursorForPosition(event->pos());
    QString lineText = cursor.block().text();
    int posInLine = cursor.positionInBlock();

    // Check if click is on checkbox
    static QRegularExpression checkboxPattern(R"(\[[ xX]\])");
    QRegularExpressionMatch match = checkboxPattern.match(lineText);
    if (match.hasMatch()) {
      int checkboxStart = match.capturedStart();
      int checkboxEnd = match.capturedEnd();

      // If click is within or near checkbox bounds
      if (posInLine >= checkboxStart - 2 && posInLine <= checkboxEnd + 1) {
        setTextCursor(cursor);
        m_modeHelper->toggleCheckboxAtCursor();
        emit checkboxToggled();
        return;
      }
    }
  }

  QPlainTextEdit::mousePressEvent(event);
}

void NoteEditor::insertFromMimeData(const QMimeData *source) {
  if (source->hasText()) {
    QString text = source->text();
    // Apply cleanup based on settings
    Settings *s = Settings::instance();
    text = cleanupPastedText(text, s);

    // Auto-shorten URLs on paste if enabled
    if (s->linkAutoShortenEnabled()) {
      // Pattern for long URLs (not already shortened)
      static QRegularExpression longUrlPattern(
          R"((https?://(?!is\.gd|v\.gd|tinyurl\.com|bit\.ly|t\.co|goo\.gl|ow\.ly|buff\.ly)[^\s<>\[\]]{40,}))",
          QRegularExpression::CaseInsensitiveOption);

      QRegularExpressionMatchIterator it = longUrlPattern.globalMatch(text);
      while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString longUrl = match.captured(1);

        // Shorten asynchronously and replace in editor after insert
        QString serviceName = s->urlShortenerService();
        UrlShortener::Service service =
            UrlShortener::stringToService(serviceName);

        auto *shortener = new UrlShortener(const_cast<NoteEditor *>(this));
        connect(shortener, &UrlShortener::shortened, this,
                [this, shortener, longUrl](const QString &shortUrl) {
                  // Find and replace the long URL in the text
                  QString content = toPlainText();
                  if (content.contains(longUrl)) {
                    content.replace(longUrl, shortUrl);
                    QTextCursor cursor = textCursor();
                    int pos = cursor.position();
                    setPlainText(content);
                    cursor.setPosition(qMin(pos, content.length()));
                    setTextCursor(cursor);
                  }
                  shortener->deleteLater();
                });
        connect(shortener, &UrlShortener::error, shortener,
                &QObject::deleteLater);
        shortener->shortenUrl(longUrl, service);
      }
    }

    // Insert cleaned text
    textCursor().insertText(text);
  } else {
    // Fall back to default handling for non-text (images, etc)
    QPlainTextEdit::insertFromMimeData(source);
  }
}

QMimeData *NoteEditor::createMimeDataFromSelection() const {
  QMimeData *mimeData = QPlainTextEdit::createMimeDataFromSelection();
  if (!mimeData || !mimeData->hasText())
    return mimeData;

  Settings *s = Settings::instance();
  QString text = mimeData->text();
  bool modified = false;

  // Skip keywords on copy (math, calc, code, checklist, etc)
  if (s->skipKeywordsOnCopy()) {
    // Remove mode keywords at the start of lines (like "math", "calc", "code")
    static QRegularExpression keywordPattern(
        R"(^(math|calc|calculator|hesap|kod|code|checklist|liste|check|todo|"
        R"kontrol|currency|doviz|döviz|para|timer|zamanlayici|zamanlayıcı|"
        R"reminder|hatirlatici|hatırlatıcı)\s*\n?)",
        QRegularExpression::CaseInsensitiveOption |
            QRegularExpression::MultilineOption);
    QString cleaned = text;
    cleaned.remove(keywordPattern);
    if (cleaned != text) {
      text = cleaned;
      modified = true;
    }
  }

  // Skip triggers on copy (/x commands like /c, /m, /t etc)
  if (s->skipTriggersOnCopy()) {
    static QRegularExpression triggerPattern(
        R"(^\/[a-zA-Z]+\s*\n?)", QRegularExpression::MultilineOption);
    QString cleaned = text;
    cleaned.remove(triggerPattern);
    if (cleaned != text) {
      text = cleaned;
      modified = true;
    }
  }

  if (modified) {
    mimeData->setText(text.trimmed());
  }

  return mimeData;
}

void NoteEditor::updateMathOverlay() {
  if (m_currentMode != NoteMode::Math || !m_mathOverlay)
    return;

  QString results = m_modeHelper->getMathResults();
  m_mathOverlay->setText(results);

  // Position overlay on the right side
  int scrollWidth =
      verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
  int overlayWidth = 150;
  m_mathOverlay->setGeometry(width() - overlayWidth - scrollWidth - 10, 5,
                             overlayWidth, height() - 10);
}

void NoteEditor::setupCommandPopup() {
  m_commandPopup = new CommandPopup(this);

  connect(m_commandPopup, &CommandPopup::commandSelected, this,
          &NoteEditor::onCommandSelected);
  connect(m_commandPopup, &CommandPopup::dismissed, this,
          &NoteEditor::onPopupDismissed);
}

void NoteEditor::showCommandPopup() {
  if (!m_commandPopup)
    return;

  // Get cursor position in global coordinates
  QRect cursorRect = this->cursorRect();
  QPoint pos = mapToGlobal(cursorRect.bottomLeft());

  m_popupActive = true;
  m_commandPopup->show(pos, this);
  m_commandPopup->setFocus();

  qDebug() << "NoteEditor: Command popup shown at" << pos;
}

void NoteEditor::hideCommandPopup() {
  if (m_commandPopup) {
    m_commandPopup->hide();
  }
  m_popupActive = false;
}

void NoteEditor::onCommandSelected(const QString &command) {
  qDebug() << "NoteEditor: Command selected:" << command;

  // Remove the '/' that triggered the popup
  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
  if (cursor.selectedText() == "/") {
    cursor.removeSelectedText();
  }
  setTextCursor(cursor);

  m_popupActive = false;

  // Emit signal for MainWindow to execute the command
  emit commandExecuted(command);
}

void NoteEditor::onPopupDismissed() {
  m_popupActive = false;
  setFocus();
}

void NoteEditor::setupTutorialLabel() {
  m_tutorialLabel = new QLabel(this);
  m_tutorialLabel->setStyleSheet(R"(
    QLabel {
      color: #6c7086;
      background: rgba(30, 30, 46, 0.9);
      border: 1px solid #45475a;
      border-radius: 6px;
      padding: 8px 12px;
      font-family: monospace;
      font-size: 10pt;
    }
  )");
  m_tutorialLabel->setWordWrap(true);
  m_tutorialLabel->hide();
}

void NoteEditor::showKeywordTutorial(const QString &keyword) {
  if (m_lastTutorialKeyword == keyword) {
    return; // Already showing this tutorial
  }

  m_lastTutorialKeyword = keyword;

  // Define inline tutorial placeholder text for each keyword
  QString placeholder;
  QString lowerKeyword = keyword.toLower();

  if (lowerKeyword == "calc") {
    // Auto-refresh currency/crypto rates when entering calc mode
    CurrencyConverter::instance()->refreshRates();

    placeholder = "\n━━━ CALC MODE ━━━\n\n"
                  "CALCULATE: Add '=' at end of line\n"
                  "  2 + 3 =           → 5\n"
                  "  100 * 0.18 =      → 18\n"
                  "  sqrt(16) =        → 4\n\n"
                  "UNIT CONVERSION:\n"
                  "  50 km to mile     → 31.07\n"
                  "  100 USD to TRY    → (live rate)\n"
                  "  25 C to F         → 77°F\n\n"
                  "VARIABLES:\n"
                  "  price : 100       → assign variable\n"
                  "  price * 1.18 =    → 118\n\n"
                  "FUNCTIONS: sum() avg() min() max() count()\n"
                  "           sqrt() sin() cos() log() abs()";
  } else if (lowerKeyword == "list" || lowerKeyword == "checklist") {
    placeholder = "\n━━━ LIST MODE ━━━\n\n"
                  "NEW ITEM: Press Enter\n"
                  "TOGGLE: Space or add /x at end\n\n"
                  "EXAMPLES:\n"
                  "- [ ] Buy groceries\n"
                  "- [x] Send email  (checked)\n"
                  "- [ ] Meeting notes\n\n"
                  "HEADING: Start with #\n"
                  "# Today's Tasks\n\n"
                  "COMMENT: Start with //\n"
                  "// This is a note";
  } else if (lowerKeyword == "code") {
    placeholder = "\n━━━ CODE MODE ━━━\n\n"
                  "Monospace font enabled.\n"
                  "Perfect for code snippets.\n\n"
                  "function hello() {\n"
                  "  console.log('Hello!');\n"
                  "}\n\n"
                  "const x = 42;";
  } else if (lowerKeyword == "timer") {
    placeholder = "\ntimer sw - start stopwatch\n\n"
                  "timer 5 - 5 minutes countdown\n"
                  "timer 5s - 5 seconds countdown\n"
                  "timer 5:30 : laundry - named countdown\n\n"
                  "timer pomo - pomodoro (25m/5m)\n"
                  "timer 52 17 - custom pomodoro\n\n"
                  "timer p - pause/resume\n"
                  "timer r - reset\n"
                  "timer s - stop\n\n"
                  "timer breathe - breathing exercise\n\n"
                  "Click to pause, Double-click to stop";
  } else if (lowerKeyword == "paste") {
    placeholder = "\n━━━ AutoPaste MODE ━━━\n\n"
                  "Enter to start AutoPaste.\n"
                  "Everything you copy will be added here.\n\n"
                  "Press ESC to stop.\n\n"
                  "Type paste(, ) to separate each item\n"
                  "with a comma and space instead of a new line.\n\n"
                  "Or use whatever you want in the parentheses\n"
                  "for a delimiter.";
  } else if (lowerKeyword == "markdown" || lowerKeyword == "md") {
    placeholder = "\n━━━ MARKDOWN MODE ━━━\n\n"
                  "HEADINGS:\n"
                  "  # H1  ## H2  ### H3\n\n"
                  "TEXT FORMATTING:\n"
                  "  **bold**    *italic*    ~~strike~~\n"
                  "  `inline code`\n\n"
                  "LISTS:\n"
                  "  - Bullet item\n"
                  "  1. Numbered item\n\n"
                  "LINKS & IMAGES:\n"
                  "  [text](url)\n"
                  "  ![alt](image.png)\n\n"
                  "CODE BLOCK:\n"
                  "  ```language\n"
                  "  code here\n"
                  "  ```\n\n"
                  "QUOTE:\n"
                  "  > This is a quote";
  } else if (lowerKeyword == "plain" || lowerKeyword == "text") {
    placeholder = "\n━━━ PLAIN TEXT MODE ━━━\n\n"
                  "Simple text without formatting.\n\n"
                  "• No syntax highlighting\n"
                  "• No special processing\n"
                  "• Just pure text\n\n"
                  "Perfect for quick notes,\n"
                  "copy-paste content,\n"
                  "or simple drafts.";
  } else {
    return;
  }

  // Record tutorial position before inserting
  QTextCursor cursor = textCursor();
  cursor.movePosition(QTextCursor::End);
  m_tutorialStartPos = cursor.position();

  // CRITICAL: Block signals to prevent textChanged from re-triggering
  // during tutorial text insertion
  blockSignals(true);

  // Insert placeholder text with faded gray color (placeholder style)
  QTextCharFormat placeholderFormat;
  placeholderFormat.setForeground(QColor(90, 90, 100, 150)); // Faded gray
  cursor.insertText(placeholder, placeholderFormat);

  m_tutorialLength = placeholder.length();
  m_tutorialContent = placeholder; // Save content for find/remove
  m_tutorialActive = true;

  // Move cursor to right after keyword (before tutorial)
  cursor.setPosition(m_tutorialStartPos);
  setTextCursor(cursor);

  blockSignals(false);

  // Hide the popup label (no longer needed)
  m_tutorialLabel->hide();
}

void NoteEditor::hideTutorial() {
  // Remove inline tutorial text if active - use content search instead of
  // position
  if (m_tutorialActive && !m_tutorialContent.isEmpty()) {
    // CRITICAL: Block signals to prevent textChanged from re-triggering
    // showKeywordTutorial while we're removing the tutorial text
    blockSignals(true);

    QString content = toPlainText();
    int pos = content.indexOf(m_tutorialContent);
    if (pos >= 0) {
      QTextCursor cursor = textCursor();
      int cursorPos = cursor.position(); // Save cursor position
      cursor.setPosition(pos);
      cursor.setPosition(pos + m_tutorialContent.length(),
                         QTextCursor::KeepAnchor);
      cursor.removeSelectedText();

      // Restore cursor to original position (adjusted for removed text)
      int newPos = qMin(cursorPos, toPlainText().length());
      cursor.setPosition(newPos);
      setTextCursor(cursor);
    }
    m_tutorialActive = false;
    m_tutorialLength = 0;
    m_tutorialContent.clear();

    blockSignals(false);
  }

  m_tutorialLabel->hide();
  m_lastTutorialKeyword.clear();
}

void NoteEditor::checkForKeywordTutorial() {
  // Get current line (where cursor is)
  QTextCursor cursor = textCursor();
  QString rawLineText = cursor.block().text();
  QString lineText = rawLineText.trimmed();

  qDebug() << "=== checkForKeywordTutorial ===";
  qDebug() << "  rawLineText:" << rawLineText;
  qDebug() << "  lineText (trimmed):" << lineText;
  qDebug() << "  m_tutorialActive:" << m_tutorialActive;
  qDebug() << "  cursor.position():" << cursor.position();

  // Check if current line is exactly a keyword (nothing else)
  static QStringList keywords = {"calc",  "list",  "checklist", "code",
                                 "timer", "paste", "markdown",  "md",
                                 "plain", "text"};

  QString checkLine = lineText;
  if (checkLine.startsWith('/')) {
    checkLine = checkLine.mid(1);
  }
  checkLine = checkLine.toLower();
  qDebug() << "  checkLine:" << checkLine;

  // CRITICAL: If tutorial is active and user typed ANYTHING after keyword,
  // hide tutorial and let them type (prevents re-showing loop)
  if (m_tutorialActive) {
    // DON'T trim! We need to detect trailing space: "timer " vs "timer"
    QString rawCheck = rawLineText.toLower();
    if (rawCheck.startsWith('/')) {
      rawCheck = rawCheck.mid(1);
    }
    qDebug() << "  rawCheck (for active tutorial):" << rawCheck
             << "len:" << rawCheck.length();
    // Check if line has more than just the keyword (e.g., "timer " or "timer
    // 5")
    for (const QString &kw : keywords) {
      if (rawCheck.startsWith(kw) && rawCheck.length() > kw.length()) {
        // User typed something after keyword - hide tutorial and return
        qDebug() << "  >>> Hiding tutorial! kw=" << kw
                 << " rawCheck=" << rawCheck;
        hideTutorial();
        return;
      }
    }
    qDebug() << "  Tutorial active but no hide triggered";
  }

  // Get aliases and try to resolve
  Settings *s = Settings::instance();
  QMap<QString, QString> aliases = s->keywordAliases();
  QString resolvedKeyword = checkLine;
  for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it) {
    if (!it.value().isEmpty() && it.value().toLower() == checkLine) {
      resolvedKeyword = it.key();
      break;
    }
  }

  for (const QString &kw : keywords) {
    if (resolvedKeyword == kw || checkLine == kw) {
      // Show tutorial every time the keyword is typed EXACTLY
      showKeywordTutorial(kw);
      return;
    }
  }

  // For timer/paste with arguments (timer 5, paste(,)), clear tutorial and
  // allow typing
  if (checkLine.startsWith("timer ") || checkLine.startsWith("paste(") ||
      checkLine.startsWith("paste ")) {
    // User is typing arguments, clear the tutorial
    if (m_tutorialActive) {
      hideTutorial();
    }
    m_lastTutorialKeyword.clear();
    return;
  }

  // Only clear last keyword when line is empty (allows showing tutorial again)
  if (checkLine.isEmpty()) {
    m_lastTutorialKeyword.clear();
  }
}

void NoteEditor::checkForAutoConversion() {
  // Only in Math mode
  if (m_currentMode != NoteMode::Math) {
    return;
  }

  // Get current line
  QTextCursor cursor = textCursor();
  QString lineText = cursor.block().text();
  QString trimmed = lineText.trimmed();

  // Skip if line already has result or is empty
  if (trimmed.isEmpty() || trimmed.contains("=")) {
    return;
  }

  // Check if this is a conversion (has "to" keyword)
  if (!trimmed.contains(" to ", Qt::CaseInsensitive)) {
    return;
  }

  // Skip if we already converted this exact line
  if (trimmed == m_lastConvertedLine) {
    return;
  }

  // Try to calculate
  QString result = m_modeHelper->calculateExpression(trimmed);
  if (result.isEmpty()) {
    return;
  }

  // Block signals to prevent infinite recursion
  blockSignals(true);

  // Move to end of current block and insert result
  cursor.movePosition(QTextCursor::EndOfBlock);
  cursor.insertText(" = " + result);
  setTextCursor(cursor);

  // Remember this line to prevent re-conversion
  m_lastConvertedLine = trimmed;

  blockSignals(false);
}

void NoteEditor::updateGhostText() {
  if (m_currentMode != NoteMode::Math) {
    clearGhostText();
    return;
  }

  // Get current word (from start of word to cursor)
  QTextCursor cursor = textCursor();
  QString lineText = cursor.block().text();
  int posInLine = cursor.positionInBlock();

  // Find word start
  int wordStart = posInLine;
  while (wordStart > 0 && (lineText[wordStart - 1].isLetterOrNumber() ||
                           lineText[wordStart - 1] == '_')) {
    wordStart--;
  }

  QString prefix = lineText.mid(wordStart, posInLine - wordStart);
  if (prefix.length() >= 1) {
    QStringList variables = m_modeHelper->getVariables();
    // Find first matching variable
    for (const QString &var : variables) {
      if (var.startsWith(prefix, Qt::CaseInsensitive) && var != prefix) {
        // Show ghost text (remainder after prefix)
        QString remainder = var.mid(prefix.length());
        m_ghostCompletion = var;

        // Position ghost label inline right after cursor
        QRect cursorRect = this->cursorRect();
        m_ghostLabel->setText(remainder);
        m_ghostLabel->setFont(font());
        m_ghostLabel->adjustSize();
        // Position at cursor location within viewport
        int x = cursorRect.right() + 1; // Just after cursor
        int y = cursorRect.top();       // Same line as cursor
        m_ghostLabel->move(x, y);
        m_ghostLabel->raise(); // Ensure it's on top
        m_ghostLabel->show();
        return;
      }
    }
  }

  clearGhostText();
}

void NoteEditor::clearGhostText() {
  m_ghostLabel->hide();
  m_ghostCompletion.clear();
}
