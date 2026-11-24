#include "MathEvaluator.h"
#include <cmath>
#include <QDebug>
#include <QRegularExpression>
#include <QtMath>

MathEvaluator::MathEvaluator() {}

bool MathEvaluator::isMathExpression(const QString &line) const {
  QString trimmed = line.trimmed();
  if (trimmed.isEmpty())
    return false;

  // Exclude file paths (starts with / or contains :/ or backslash)
  if (trimmed.startsWith('/') || trimmed.startsWith('~') ||
      trimmed.contains(":/") || trimmed.contains("\\")) {
    return false;
  }

  // Exclude URLs
  static QRegularExpression urlPattern(
      R"(^https?://|^ftp://|^file://)",
      QRegularExpression::CaseInsensitiveOption);
  if (urlPattern.match(trimmed).hasMatch()) {
    return false;
  }

  // Check for function calls like sqrt(16), sin(0), etc.
  static QRegularExpression funcPattern(
      R"(\b(sqrt|sin|cos|tan|asin|acos|atan|log|log10|ln|exp|ceil|floor|round|abs|sum|avg|average|min|max|count)\s*\()",
      QRegularExpression::CaseInsensitiveOption);
  if (funcPattern.match(trimmed).hasMatch()) {
    return true;
  }

  // Check for common math patterns
  static QRegularExpression mathPattern(
      R"(^[\d\s\+\-\*\/\%\^\(\)\.\,\=a-zA-Z_eE]+$)");

  if (!mathPattern.match(trimmed).hasMatch()) {
    return false;
  }

  // Must contain at least one operator or assignment
  static QRegularExpression hasOperator(R"([\+\*\/\%\^\=]|(?<!^)-)");
  return hasOperator.match(trimmed).hasMatch();
}

double MathEvaluator::evaluate(const QString &expression, bool *ok) {
  QString expr = expression.trimmed();
  if (ok)
    *ok = false;

  // Check for variable assignment: x = 10 OR name: value format
  static QRegularExpression assignment(R"(^([a-zA-Z_]\w*)\s*[:=]\s*(.+)$)");
  QRegularExpressionMatch match = assignment.match(expr);

  if (match.hasMatch()) {
    QString varName = match.captured(1);
    QString valueExpr = match.captured(2);

    bool evalOk = false;
    double value = evaluate(valueExpr, &evalOk);

    if (evalOk) {
      m_variables[varName] = value;
      m_values.append(value);
      if (ok)
        *ok = true;
      return value;
    }
    return 0;
  }

  // Check for special functions
  if (expr.toLower() == "sum()" || expr.toLower() == "sum") {
    double sum = 0;
    for (double v : m_values)
      sum += v;
    if (ok)
      *ok = true;
    return sum;
  }

  if (expr.toLower() == "avg()" || expr.toLower() == "avg" ||
      expr.toLower() == "average()" || expr.toLower() == "average") {
    if (m_values.isEmpty()) {
      if (ok)
        *ok = true;
      return 0;
    }
    double sum = 0;
    for (double v : m_values)
      sum += v;
    if (ok)
      *ok = true;
    return sum / m_values.size();
  }

  if (expr.toLower() == "min()" || expr.toLower() == "min") {
    if (m_values.isEmpty()) {
      if (ok)
        *ok = true;
      return 0;
    }
    double minVal = m_values.first();
    for (double v : m_values)
      if (v < minVal)
        minVal = v;
    if (ok)
      *ok = true;
    return minVal;
  }

  if (expr.toLower() == "max()" || expr.toLower() == "max") {
    if (m_values.isEmpty()) {
      if (ok)
        *ok = true;
      return 0;
    }
    double maxVal = m_values.first();
    for (double v : m_values)
      if (v > maxVal)
        maxVal = v;
    if (ok)
      *ok = true;
    return maxVal;
  }

  if (expr.toLower() == "count()" || expr.toLower() == "count") {
    if (ok)
      *ok = true;
    return m_values.size();
  }

  // Parse regular expression
  int pos = 0;
  double result = parseExpression(expr, pos, ok);

  // Store the result
  if (ok && *ok) {
    m_values.append(result);
  }

  return result;
}

void MathEvaluator::setVariable(const QString &name, double value) {
  m_variables[name] = value;
}

QList<double> MathEvaluator::allValues() const { return m_values; }

void MathEvaluator::clear() {
  m_variables.clear();
  m_values.clear();
}

QStringList MathEvaluator::getVariables() const { return m_variables.keys(); }

double MathEvaluator::parseExpression(const QString &expr, int &pos, bool *ok) {
  double result = parseTerm(expr, pos, ok);
  if (ok && !*ok)
    return 0;

  skipWhitespace(expr, pos);

  while (pos < expr.length()) {
    QChar ch = expr[pos];
    if (ch == '+') {
      pos++;
      double term = parseTerm(expr, pos, ok);
      if (ok && !*ok)
        return 0;
      result += term;
    } else if (ch == '-') {
      pos++;
      double term = parseTerm(expr, pos, ok);
      if (ok && !*ok)
        return 0;
      result -= term;
    } else {
      break;
    }
    skipWhitespace(expr, pos);
  }

  return result;
}

double MathEvaluator::parseTerm(const QString &expr, int &pos, bool *ok) {
  double result = parseFactor(expr, pos, ok);
  if (ok && !*ok)
    return 0;

  skipWhitespace(expr, pos);

  while (pos < expr.length()) {
    QChar ch = expr[pos];
    if (ch == '*') {
      pos++;
      double factor = parseFactor(expr, pos, ok);
      if (ok && !*ok)
        return 0;
      result *= factor;
    } else if (ch == '/') {
      pos++;
      double factor = parseFactor(expr, pos, ok);
      if (ok && !*ok)
        return 0;
      if (factor != 0) {
        result /= factor;
      } else {
        if (ok)
          *ok = false;
        return 0;
      }
    } else if (ch == '%') {
      pos++;
      double factor = parseFactor(expr, pos, ok);
      if (ok && !*ok)
        return 0;
      if (factor != 0) {
        result = std::fmod(result, factor);
      } else {
        if (ok)
          *ok = false;
        return 0;
      }
    } else {
      break;
    }
    skipWhitespace(expr, pos);
  }

  return result;
}

double MathEvaluator::parseFactor(const QString &expr, int &pos, bool *ok) {
  skipWhitespace(expr, pos);

  double result = 0;
  bool negative = false;

  // Handle unary minus
  if (pos < expr.length() && expr[pos] == '-') {
    negative = true;
    pos++;
    skipWhitespace(expr, pos);
  }

  if (pos < expr.length() && expr[pos] == '(') {
    pos++; // skip '('
    result = parseExpression(expr, pos, ok);
    if (ok && !*ok)
      return 0;
    skipWhitespace(expr, pos);
    if (pos < expr.length() && expr[pos] == ')') {
      pos++; // skip ')'
    }
  } else {
    result = parseNumber(expr, pos, ok);
  }

  // Handle power operator (^ or **)
  skipWhitespace(expr, pos);
  if (pos < expr.length() && expr[pos] == '^') {
    pos++;
    double exponent = parseFactor(expr, pos, ok);
    if (ok && !*ok)
      return 0;
    result = std::pow(result, exponent);
  } else if (pos + 1 < expr.length() && expr[pos] == '*' &&
             expr[pos + 1] == '*') {
    pos += 2; // Skip '**'
    double exponent = parseFactor(expr, pos, ok);
    if (ok && !*ok)
      return 0;
    result = std::pow(result, exponent);
  }

  return negative ? -result : result;
}

double MathEvaluator::parseNumber(const QString &expr, int &pos, bool *ok) {
  skipWhitespace(expr, pos);

  // Check for function or variable name
  if (pos < expr.length() && (expr[pos].isLetter() || expr[pos] == '_')) {
    QString name;
    while (pos < expr.length() &&
           (expr[pos].isLetterOrNumber() || expr[pos] == '_')) {
      name += expr[pos];
      pos++;
    }

    skipWhitespace(expr, pos);

    // Check if it's a function call (has parentheses)
    if (pos < expr.length() && expr[pos] == '(') {
      pos++; // skip '('

      QString funcName = name.toLower();

      // Check for multi-argument functions: sum, avg, min, max, count
      if (funcName == "sum" || funcName == "avg" || funcName == "average" ||
          funcName == "min" || funcName == "max" || funcName == "count") {
        QList<double> args;
        skipWhitespace(expr, pos);

        // Parse comma-separated arguments
        while (pos < expr.length() && expr[pos] != ')') {
          double arg = parseExpression(expr, pos, ok);
          if (ok && !*ok)
            return 0;
          args.append(arg);
          skipWhitespace(expr, pos);
          if (pos < expr.length() && isArgumentSeparator(expr[pos])) {
            pos++; // skip the separator
            skipWhitespace(expr, pos);
          }
        }

        skipWhitespace(expr, pos);
        if (pos < expr.length() && expr[pos] == ')') {
          pos++; // skip ')'
        }

        if (ok)
          *ok = true;

        if (args.isEmpty()) {
          // No arguments - use stored values
          if (funcName == "sum") {
            double sum = 0;
            for (double v : m_values)
              sum += v;
            return sum;
          } else if (funcName == "avg" || funcName == "average") {
            if (m_values.isEmpty())
              return 0;
            double sum = 0;
            for (double v : m_values)
              sum += v;
            return sum / m_values.size();
          } else if (funcName == "min") {
            if (m_values.isEmpty())
              return 0;
            double minVal = m_values.first();
            for (double v : m_values)
              if (v < minVal)
                minVal = v;
            return minVal;
          } else if (funcName == "max") {
            if (m_values.isEmpty())
              return 0;
            double maxVal = m_values.first();
            for (double v : m_values)
              if (v > maxVal)
                maxVal = v;
            return maxVal;
          } else if (funcName == "count") {
            return m_values.size();
          }
        } else {
          // Use provided arguments
          if (funcName == "sum") {
            double sum = 0;
            for (double v : args)
              sum += v;
            return sum;
          } else if (funcName == "avg" || funcName == "average") {
            double sum = 0;
            for (double v : args)
              sum += v;
            return sum / args.size();
          } else if (funcName == "min") {
            double minVal = args.first();
            for (double v : args)
              if (v < minVal)
                minVal = v;
            return minVal;
          } else if (funcName == "max") {
            double maxVal = args.first();
            for (double v : args)
              if (v > maxVal)
                maxVal = v;
            return maxVal;
          } else if (funcName == "count") {
            return args.size();
          }
        }
      }

      // Single-argument functions
      double arg = parseExpression(expr, pos, ok);
      if (ok && !*ok)
        return 0;
      skipWhitespace(expr, pos);
      if (pos < expr.length() && expr[pos] == ')') {
        pos++; // skip ')'
      }

      if (ok)
        *ok = true;

      // Mathematical functions
      if (funcName == "sqrt") {
        return std::sqrt(arg);
      } else if (funcName == "sin") {
        return std::sin(arg);
      } else if (funcName == "cos") {
        return std::cos(arg);
      } else if (funcName == "tan") {
        return std::tan(arg);
      } else if (funcName == "asin") {
        return std::asin(arg);
      } else if (funcName == "acos") {
        return std::acos(arg);
      } else if (funcName == "atan") {
        return std::atan(arg);
      } else if (funcName == "log" || funcName == "log10") {
        return std::log10(arg);
      } else if (funcName == "ln") {
        return std::log(arg);
      } else if (funcName == "exp") {
        return std::exp(arg);
      } else if (funcName == "ceil") {
        return std::ceil(arg);
      } else if (funcName == "floor") {
        return std::floor(arg);
      } else if (funcName == "round") {
        return std::round(arg);
      } else if (funcName == "abs") {
        return std::abs(arg);
      } else {
        // Unknown function
        if (ok)
          *ok = false;
        return 0;
      }
    }

    // It's a variable
    if (m_variables.contains(name)) {
      if (ok)
        *ok = true;
      return m_variables[name];
    } else {
      // Unknown variable
      if (ok)
        *ok = false;
      return 0;
    }
  }

  // Parse a number literal. Which character is the decimal point depends
  // on the configured separator; see MathEvaluator::setDecimalSeparator.
  QString numStr;
  bool sawComma = false;
  const bool commaIsDecimal = (m_decimalSeparator == QLatin1Char(','));
  while (pos < expr.length()) {
    const QChar c = expr[pos];
    if (c.isDigit()) {
      numStr += c;
      pos++;
      continue;
    }
    if (c == QLatin1Char('.')) {
      numStr += c;
      pos++;
      continue;
    }
    if (commaIsDecimal && c == QLatin1Char(',') && !sawComma &&
        pos + 1 < expr.length() && expr[pos + 1].isDigit()) {
      // A comma straight before a digit is this number's decimal point.
      // A comma anywhere else is left alone so it can separate arguments.
      sawComma = true;
      numStr += QLatin1Char('.');
      pos++;
      continue;
    }
    break;
  }

  if (commaIsDecimal && sawComma) {
    // "1.234,56": the dots were thousands separators. Only strip them when
    // the number really did carry a comma, so "1.5" in a note written
    // before this setting existed still reads as one and a half.
    const int decimalAt = numStr.lastIndexOf(QLatin1Char('.'));
    QString head = numStr.left(decimalAt);
    head.remove(QLatin1Char('.'));
    numStr = head + numStr.mid(decimalAt);
  }

  // Handle scientific notation (e.g., 1e3, 2.5e-2)
  if (pos < expr.length() && (expr[pos] == 'e' || expr[pos] == 'E')) {
    numStr += expr[pos];
    pos++;
    // Handle optional sign
    if (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-')) {
      numStr += expr[pos];
      pos++;
    }
    // Parse exponent digits
    while (pos < expr.length() && expr[pos].isDigit()) {
      numStr += expr[pos];
      pos++;
    }
  }

  if (numStr.isEmpty()) {
    if (ok)
      *ok = false;
    return 0;
  }

  bool parseOk = false;
  double value = numStr.toDouble(&parseOk);
  if (ok)
    *ok = parseOk;

  // Handle percentage postfix (e.g., 50% becomes 0.5)
  // But NOT if followed by a digit (that's modulo: 10%3)
  skipWhitespace(expr, pos);
  if (pos < expr.length() && expr[pos] == '%') {
    // Check if next char after % is a digit - if so, it's modulo, not
    // percentage
    int nextPos = pos + 1;
    while (nextPos < expr.length() && expr[nextPos].isSpace()) {
      nextPos++;
    }
    if (nextPos >= expr.length() || !expr[nextPos].isDigit()) {
      // It's percentage postfix
      pos++;
      value /= 100.0;
    }
    // Otherwise leave % for parseTerm to handle as modulo
  }

  return value;
}

void MathEvaluator::setDecimalSeparator(QChar separator) {
  m_decimalSeparator =
      (separator == QLatin1Char(',')) ? QLatin1Char(',') : QLatin1Char('.');
}

bool MathEvaluator::isArgumentSeparator(QChar c) const {
  // ';' always separates arguments, which is what a spreadsheet under a
  // comma locale uses. ',' separates too, except that parseNumber has
  // already consumed it when it was a decimal point.
  return c == QLatin1Char(';') || c == QLatin1Char(',');
}

QString MathEvaluator::formatNumber(double value, QChar separator,
                                    int precision) {
  QString out;
  const double magnitude = std::fabs(value);
  if (value != 0.0 && (magnitude < 1e-4 || magnitude >= 1e15)) {
    // Too small or too large to write out: leave it in exponent form.
    out = QString::number(value, 'g', precision + 2);
  } else {
    out = QString::number(value, 'f', precision);
    if (out.contains(QLatin1Char('.'))) {
      while (out.endsWith(QLatin1Char('0'))) {
        out.chop(1);
      }
      if (out.endsWith(QLatin1Char('.'))) {
        out.chop(1);
      }
    }
  }
  if (separator == QLatin1Char(',')) {
    out.replace(QLatin1Char('.'), QLatin1Char(','));
  }
  return out;
}

void MathEvaluator::skipWhitespace(const QString &expr, int &pos) {
  while (pos < expr.length() && expr[pos].isSpace()) {
    pos++;
  }
}
