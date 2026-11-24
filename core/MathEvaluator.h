#ifndef LINNOTE_MATHEVALUATOR_H
#define LINNOTE_MATHEVALUATOR_H

#include <QMap>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QVariant>

/**
 * @brief Simple math expression evaluator
 *
 * Supports:
 * - Basic operators: + - * / % ^
 * - Parentheses: ( )
 * - Variables: x = 10 or name: 100, then x * 2
 * - Functions: sum(), avg(), min(), max()
 */
class MathEvaluator {
public:
  MathEvaluator();

  /**
   * @brief Evaluate a math expression
   * @param expression The expression to evaluate
   * @param ok Set to true if evaluation succeeded
   * @return The result as a double
   */
  double evaluate(const QString &expression, bool *ok = nullptr);

  /**
   * @brief Choose which character separates the integer and fractional part
   *
   * '.' is the default and the only behaviour LinNote 1.0.0 had. With ','
   * a comma immediately followed by a digit is a decimal point, so
   * "1,5 + 2,3" is 3.8; a comma that is not followed by a digit stays an
   * argument separator, so "sum(1, 2)" keeps working, and ';' separates
   * arguments as it does in a spreadsheet under a European locale. Inside
   * one number a '.' is then a group separator, but only when that number
   * also contains a comma, so "1.5" written in an older note is still 1.5
   * while "1.234,56" is 1234.56.
   */
  void setDecimalSeparator(QChar separator);
  QChar decimalSeparator() const { return m_decimalSeparator; }

  /**
   * @brief Render a result with the configured separator
   *
   * One helper for math, unit and currency output, so a result never
   * disagrees with the notation the user typed.
   */
  static QString formatNumber(double value, QChar separator,
                              int precision = 4);

  /**
   * @brief Check if a line contains a math expression
   */
  bool isMathExpression(const QString &line) const;

  /**
   * @brief Set a variable value
   */
  void setVariable(const QString &name, double value);

  /**
   * @brief Get all stored values for sum/avg
   */
  QList<double> allValues() const;

  /**
   * @brief Clear all variables and values
   */
  void clear();

  /**
   * @brief Get all variable names (for autocomplete)
   */
  QStringList getVariables() const;

private:
  double parseExpression(const QString &expr, int &pos, bool *ok);
  double parseTerm(const QString &expr, int &pos, bool *ok);
  double parseFactor(const QString &expr, int &pos, bool *ok);
  double parseNumber(const QString &expr, int &pos, bool *ok);
  void skipWhitespace(const QString &expr, int &pos);

  bool isArgumentSeparator(QChar c) const;

  QMap<QString, double> m_variables;
  QList<double> m_values;
  QChar m_decimalSeparator = QLatin1Char('.');
};

#endif // LINNOTE_MATHEVALUATOR_H
