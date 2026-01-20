#ifndef LINNOTE_MATHEVALUATOR_H
#define LINNOTE_MATHEVALUATOR_H

#include <QMap>
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

  QMap<QString, double> m_variables;
  QList<double> m_values;
};

#endif // LINNOTE_MATHEVALUATOR_H
