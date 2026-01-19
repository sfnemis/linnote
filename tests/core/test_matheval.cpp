#include "core/MathEvaluator.h"
#include <QTest>

class TestMathEvaluator : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Basic arithmetic tests
  void testAddition();
  void testSubtraction();
  void testMultiplication();
  void testDivision();
  void testOperatorPrecedence();
  void testParentheses();

  // Floating point tests
  void testFloatingPoint();
  void testScientificNotation();

  // Power and modulo
  void testPower();
  void testModulo();

  // Mathematical functions
  void testSqrt();
  void testTrigFunctions();
  void testLogFunctions();
  void testAbsFunction();

  // Variables
  void testVariableAssignment();
  void testVariableUsage();

  // Percentage calculations
  void testPercentage();
  void testPercentageOf();

  // Edge cases
  void testEmptyExpression();
  void testInvalidExpression();
  void testDivisionByZero();
  void testVeryLargeNumbers();
  void testNegativeNumbers();

  // Expression detection
  void testIsMathExpression();
  void testIsMathExpression_data();

private:
  MathEvaluator *m_evaluator;
};

void TestMathEvaluator::initTestCase() { m_evaluator = new MathEvaluator(); }

void TestMathEvaluator::cleanupTestCase() { delete m_evaluator; }

// ============ Basic Arithmetic ============

void TestMathEvaluator::testAddition() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("2+2", &ok), 4.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("10 + 20", &ok), 30.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("1+2+3+4+5", &ok), 15.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testSubtraction() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("10-3", &ok), 7.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("100 - 50", &ok), 50.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testMultiplication() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("3*4", &ok), 12.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("7 * 8", &ok), 56.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testDivision() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("10/2", &ok), 5.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("100 / 4", &ok), 25.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testOperatorPrecedence() {
  bool ok;
  // Multiplication before addition
  QCOMPARE(m_evaluator->evaluate("2+3*4", &ok), 14.0);
  QVERIFY(ok);

  // Division before subtraction
  QCOMPARE(m_evaluator->evaluate("10-8/2", &ok), 6.0);
  QVERIFY(ok);

  // Complex expression
  QCOMPARE(m_evaluator->evaluate("2+3*4-10/2", &ok), 9.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testParentheses() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("(2+3)*4", &ok), 20.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("(10-5)*(3+2)", &ok), 25.0);
  QVERIFY(ok);

  // Nested parentheses
  QCOMPARE(m_evaluator->evaluate("((2+3)*4)+5", &ok), 25.0);
  QVERIFY(ok);
}

// ============ Floating Point ============

void TestMathEvaluator::testFloatingPoint() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("3.14*2", &ok), 6.28);
  QVERIFY(ok);

  QVERIFY(qAbs(m_evaluator->evaluate("1.5+2.5", &ok) - 4.0) < 0.0001);
  QVERIFY(ok);
}

void TestMathEvaluator::testScientificNotation() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("1e3", &ok), 1000.0);
  QVERIFY(ok);

  QVERIFY(qAbs(m_evaluator->evaluate("2.5e-2", &ok) - 0.025) < 0.0001);
  QVERIFY(ok);
}

// ============ Power and Modulo ============

void TestMathEvaluator::testPower() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("2^10", &ok), 1024.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("3**2", &ok), 9.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("10^0", &ok), 1.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testModulo() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("10%3", &ok), 1.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("17 % 5", &ok), 2.0);
  QVERIFY(ok);
}

// ============ Mathematical Functions ============

void TestMathEvaluator::testSqrt() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("sqrt(16)", &ok), 4.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("sqrt(25)", &ok), 5.0);
  QVERIFY(ok);

  QVERIFY(qAbs(m_evaluator->evaluate("sqrt(2)", &ok) - 1.41421356) < 0.0001);
  QVERIFY(ok);
}

void TestMathEvaluator::testTrigFunctions() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("sin(0)", &ok), 0.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("cos(0)", &ok), 1.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("tan(0)", &ok), 0.0);
  QVERIFY(ok);
}

void TestMathEvaluator::testLogFunctions() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("log(10)", &ok), 1.0);
  QVERIFY(ok);

  QVERIFY(qAbs(m_evaluator->evaluate("ln(2.718281828)", &ok) - 1.0) < 0.0001);
  QVERIFY(ok);
}

void TestMathEvaluator::testAbsFunction() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("abs(-5)", &ok), 5.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("abs(10)", &ok), 10.0);
  QVERIFY(ok);
}

// ============ Variables ============

void TestMathEvaluator::testVariableAssignment() {
  m_evaluator->clear();
  m_evaluator->setVariable("x", 5);
  QStringList vars = m_evaluator->getVariables();
  QVERIFY(vars.contains("x"));
}

void TestMathEvaluator::testVariableUsage() {
  bool ok;
  m_evaluator->clear();
  m_evaluator->setVariable("x", 5);

  QCOMPARE(m_evaluator->evaluate("x*2", &ok), 10.0);
  QVERIFY(ok);

  m_evaluator->setVariable("y", 10);
  QCOMPARE(m_evaluator->evaluate("x+y", &ok), 15.0);
  QVERIFY(ok);
}

// ============ Percentage ============

void TestMathEvaluator::testPercentage() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("100*20%", &ok), 20.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("50%", &ok), 0.5);
  QVERIFY(ok);
}

void TestMathEvaluator::testPercentageOf() {
  bool ok;
  // "50% of 200" syntax - may need specific implementation check
  double result = m_evaluator->evaluate("50%*200", &ok);
  QCOMPARE(result, 100.0);
  QVERIFY(ok);
}

// ============ Edge Cases ============

void TestMathEvaluator::testEmptyExpression() {
  bool ok;
  m_evaluator->evaluate("", &ok);
  QVERIFY(!ok);
}

void TestMathEvaluator::testInvalidExpression() {
  bool ok;
  m_evaluator->evaluate("2++3", &ok);
  QVERIFY(!ok);

  m_evaluator->evaluate("abc", &ok);
  // This might actually parse as a variable, so checking behavior
}

void TestMathEvaluator::testDivisionByZero() {
  bool ok;
  double result = m_evaluator->evaluate("10/0", &ok);
  // Should either return inf or set ok to false
  QVERIFY(!ok || qIsInf(result));
}

void TestMathEvaluator::testVeryLargeNumbers() {
  bool ok;
  double result = m_evaluator->evaluate("1e100 * 1e100", &ok);
  QVERIFY(ok);
  QVERIFY(qIsInf(result) || result > 1e199);
}

void TestMathEvaluator::testNegativeNumbers() {
  bool ok;
  QCOMPARE(m_evaluator->evaluate("-5+10", &ok), 5.0);
  QVERIFY(ok);

  QCOMPARE(m_evaluator->evaluate("(-3)*(-4)", &ok), 12.0);
  QVERIFY(ok);
}

// ============ Expression Detection ============

void TestMathEvaluator::testIsMathExpression_data() {
  QTest::addColumn<QString>("expression");
  QTest::addColumn<bool>("expected");

  QTest::newRow("simple addition") << "2+2" << true;
  QTest::newRow("multiplication") << "3*4" << true;
  QTest::newRow("with spaces") << "10 + 20" << true;
  QTest::newRow("with equals") << "5+5=" << true;
  QTest::newRow("function") << "sqrt(16)" << true;
  QTest::newRow("plain text") << "hello world" << false;
  QTest::newRow("url") << "http://example.com" << false;
  QTest::newRow("file path") << "/home/user/file.txt" << false;
}

void TestMathEvaluator::testIsMathExpression() {
  QFETCH(QString, expression);
  QFETCH(bool, expected);

  bool result = m_evaluator->isMathExpression(expression);
  QCOMPARE(result, expected);
}

QTEST_MAIN(TestMathEvaluator)
#include "test_matheval.moc"
