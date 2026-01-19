#include "TextAnalyzer.h"
#include <QRegularExpression>
#include <QtMath>

TextAnalyzer::TextAnalyzer() {}

QList<double> TextAnalyzer::extractNumbers(const QString &text) const {
  QList<double> numbers;

  // Match numbers with optional currency symbols and thousand separators
  // Patterns: $25, €10, 3.14, -5, 1,000.50, 25.00, - 5 (minus with space)
  QRegularExpression regex(
      R"([\$€£¥]?\s*(-\s*\d{1,3}(?:,\d{3})*(?:\.\d+)?|-\s*\d+(?:\.\d+)?|\d{1,3}(?:,\d{3})*(?:\.\d+)?|\d+(?:\.\d+)?))");
  QRegularExpressionMatchIterator it = regex.globalMatch(text);
  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString numStr = match.captured(1);
    // Remove thousand separators and any internal spaces (for "- 5" format)
    numStr.remove(',');
    numStr.remove(' ');
    bool ok;
    double val = numStr.toDouble(&ok);
    if (ok) {
      numbers.append(val);
    }
  }

  return numbers;
}

double TextAnalyzer::sum(const QString &text) const {
  QList<double> nums = extractNumbers(text);
  double total = 0;
  for (double n : nums) {
    total += n;
  }
  return total;
}

double TextAnalyzer::avg(const QString &text) const {
  QList<double> nums = extractNumbers(text);
  if (nums.isEmpty())
    return 0;
  double total = 0;
  for (double n : nums) {
    total += n;
  }
  return total / nums.size();
}

double TextAnalyzer::min(const QString &text) const {
  QList<double> nums = extractNumbers(text);
  if (nums.isEmpty())
    return 0;
  double minVal = nums.first();
  for (double n : nums) {
    if (n < minVal)
      minVal = n;
  }
  return minVal;
}

double TextAnalyzer::max(const QString &text) const {
  QList<double> nums = extractNumbers(text);
  if (nums.isEmpty())
    return 0;
  double maxVal = nums.first();
  for (double n : nums) {
    if (n > maxVal)
      maxVal = n;
  }
  return maxVal;
}

int TextAnalyzer::countSyllables(const QString &word) const {
  QString w = word.toLower();
  int count = 0;
  bool prevVowel = false;

  for (int i = 0; i < w.length(); i++) {
    QChar c = w[i];
    bool isVowel =
        (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y');
    if (isVowel && !prevVowel) {
      count++;
    }
    prevVowel = isVowel;
  }

  // Handle silent e at end
  if (w.endsWith('e') && count > 1) {
    count--;
  }

  return qMax(1, count);
}

TextAnalyzer::TextStats TextAnalyzer::analyze(const QString &text) const {
  TextStats stats = {0, 0, 0, 0, 0.0, 0.0};

  // Items: non-empty lines
  QStringList lines = text.split('\n');
  for (const QString &line : lines) {
    if (!line.trimmed().isEmpty()) {
      stats.items++;
    }
  }

  // Words
  QRegularExpression wordRegex(R"(\b[a-zA-ZğüşöçıİĞÜŞÖÇ]+\b)");
  QRegularExpressionMatchIterator wordIt = wordRegex.globalMatch(text);
  int totalSyllables = 0;
  while (wordIt.hasNext()) {
    QRegularExpressionMatch match = wordIt.next();
    stats.words++;
    totalSyllables += countSyllables(match.captured());
  }

  // Characters (excluding whitespace)
  for (const QChar &c : text) {
    if (!c.isSpace()) {
      stats.characters++;
    }
  }

  // Sentences (count . ! ?)
  QRegularExpression sentenceRegex(R"([.!?]+)");
  QRegularExpressionMatchIterator sentIt = sentenceRegex.globalMatch(text);
  while (sentIt.hasNext()) {
    sentIt.next();
    stats.sentences++;
  }
  if (stats.sentences == 0 && stats.words > 0) {
    stats.sentences = 1;
  }

  // Flesch Reading Ease
  // 206.835 - 1.015*(words/sentences) - 84.6*(syllables/words)
  if (stats.words > 0 && stats.sentences > 0) {
    double avgWordsPerSentence = (double)stats.words / stats.sentences;
    double avgSyllablesPerWord = (double)totalSyllables / stats.words;
    stats.fleschReadingEase =
        206.835 - 1.015 * avgWordsPerSentence - 84.6 * avgSyllablesPerWord;
    stats.fleschReadingEase = qBound(0.0, stats.fleschReadingEase, 100.0);

    // Flesch-Kincaid Grade Level
    // 0.39*(words/sentences) + 11.8*(syllables/words) - 15.59
    stats.fleschKincaidGrade =
        0.39 * avgWordsPerSentence + 11.8 * avgSyllablesPerWord - 15.59;
    stats.fleschKincaidGrade = qMax(0.0, stats.fleschKincaidGrade);
  }

  return stats;
}

QString TextAnalyzer::formatSum(const QString &text) const {
  QList<double> nums = extractNumbers(text);
  if (nums.isEmpty()) {
    return "\nTotal: 0";
  }
  double total = sum(text);
  // Check if original text had currency symbols
  bool hasCurrency = text.contains('$') || text.contains(QChar(0x20AC)) ||
                     text.contains(QChar(0x00A3)) ||
                     text.contains(QChar(0x00A5));
  if (hasCurrency) {
    return QString("\nTotal: $%1").arg(total, 0, 'f', 2);
  }
  return QString("\nTotal: %1").arg(total, 0, 'f', 2);
}

QString TextAnalyzer::formatAvg(const QString &text) const {
  QList<double> nums = extractNumbers(text);
  if (nums.isEmpty()) {
    return "\nAvg: 0";
  }
  double average = avg(text);
  bool hasCurrency = text.contains('$') || text.contains(QChar(0x20AC)) ||
                     text.contains(QChar(0x00A3)) ||
                     text.contains(QChar(0x00A5));
  if (hasCurrency) {
    return QString("\nAvg: $%1").arg(average, 0, 'f', 2);
  }
  return QString("\nAvg: %1").arg(average, 0, 'f', 2);
}

QString TextAnalyzer::formatCount(const QString &text) const {
  TextStats s = analyze(text);

  QString result;
  result += QString("\nItems: %1").arg(s.items);
  result += QString("\nWords: %1").arg(s.words);
  result += QString("\nCharacters: %1").arg(s.characters);
  result += QString("\nSentences: %1").arg(s.sentences);
  result += QString("\nFlesch Reading Ease Score: %1")
                .arg(s.fleschReadingEase, 0, 'f', 2);
  result += QString("\nFlesch-Kincaid Grade Level: %1")
                .arg(s.fleschKincaidGrade, 0, 'f', 2);

  return result;
}
