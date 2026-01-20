#ifndef LINNOTE_TEXTANALYZER_H
#define LINNOTE_TEXTANALYZER_H

#include <QList>
#include <QString>

/**
 * @brief Text analysis utilities for sum, avg, count commands
 *
 * Extracts numbers from text (supports $, €, decimals)
 * Calculates text statistics (words, chars, sentences)
 * Computes readability scores
 */
class TextAnalyzer {
public:
  TextAnalyzer();

  /**
   * @brief Extract all numbers from text
   * Supports: $25, €10, 3.14, -5, 1,000.50
   */
  QList<double> extractNumbers(const QString &text) const;

  /**
   * @brief Sum of all numbers in text
   */
  double sum(const QString &text) const;

  /**
   * @brief Average of all numbers in text
   */
  double avg(const QString &text) const;

  /**
   * @brief Minimum of all numbers in text
   */
  double min(const QString &text) const;

  /**
   * @brief Maximum of all numbers in text
   */
  double max(const QString &text) const;

  // Text statistics
  struct TextStats {
    int items;                 // non-empty lines
    int words;                 // word count
    int characters;            // character count (excluding whitespace)
    int sentences;             // sentence count
    double fleschReadingEase;  // 0-100, higher = easier
    double fleschKincaidGrade; // US grade level
  };

  /**
   * @brief Compute text statistics
   */
  TextStats analyze(const QString &text) const;

  /**
   * @brief Format sum result for display
   */
  QString formatSum(const QString &text) const;

  /**
   * @brief Format avg result for display
   */
  QString formatAvg(const QString &text) const;

  /**
   * @brief Format count result for display
   */
  QString formatCount(const QString &text) const;

private:
  int countSyllables(const QString &word) const;
};

#endif // LINNOTE_TEXTANALYZER_H
