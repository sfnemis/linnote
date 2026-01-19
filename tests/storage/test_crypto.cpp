#include "storage/Crypto.h"
#include <QTest>

class TestCrypto : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();

  // Password hashing tests
  void testHashPassword();
  void testHashPasswordConsistency();
  void testHashPasswordDifferentInputs();

  // Password verification tests
  void testVerifyPassword();
  void testVerifyPasswordWrong();

  // Encryption tests
  void testEncryptDecrypt();
  void testEncryptDecryptDifferentKeys();
  void testEncryptDecryptLongText();
  void testEncryptDecryptSpecialChars();

  // Salt generation tests
  void testGenerateSalt();
  void testGenerateSaltUniqueness();

  // Edge cases
  void testEmptyPassword();
  void testEmptyData();
  void testVeryLongPassword();
  void testUnicodePassword();
  void testUnicodeText();

private:
  QString m_testPassword = "TestPassword123!";
  QString m_testData = "This is some test data to encrypt.";
};

void TestCrypto::initTestCase() {
  // Nothing to initialize
}

void TestCrypto::cleanupTestCase() {
  // Nothing to cleanup
}

// ============ Password Hashing ============

void TestCrypto::testHashPassword() {
  QString hash = Crypto::hashPassword(m_testPassword);
  QVERIFY(!hash.isEmpty());
  QVERIFY(hash != m_testPassword);
  QVERIFY(hash.contains(":")); // Format is salt:hash
}

void TestCrypto::testHashPasswordConsistency() {
  // With the same salt, should produce same hash
  QString salt = Crypto::generateSalt();
  QString hash1 = Crypto::hashPassword(m_testPassword, salt);
  QString hash2 = Crypto::hashPassword(m_testPassword, salt);
  QCOMPARE(hash1, hash2);
}

void TestCrypto::testHashPasswordDifferentInputs() {
  QString hash1 = Crypto::hashPassword("password1");
  QString hash2 = Crypto::hashPassword("password2");
  QVERIFY(hash1 != hash2);
}

// ============ Password Verification ============

void TestCrypto::testVerifyPassword() {
  QString hash = Crypto::hashPassword(m_testPassword);
  QVERIFY(Crypto::verifyPassword(m_testPassword, hash));
}

void TestCrypto::testVerifyPasswordWrong() {
  QString hash = Crypto::hashPassword(m_testPassword);
  QVERIFY(!Crypto::verifyPassword("WrongPassword", hash));
}

// ============ Encryption ============

void TestCrypto::testEncryptDecrypt() {
  QString encrypted = Crypto::encrypt(m_testData, m_testPassword);
  QVERIFY(!encrypted.isEmpty());
  QVERIFY(encrypted != m_testData);

  QString decrypted = Crypto::decrypt(encrypted, m_testPassword);
  QCOMPARE(decrypted, m_testData);
}

void TestCrypto::testEncryptDecryptDifferentKeys() {
  QString encrypted = Crypto::encrypt(m_testData, "key1");
  QString decrypted = Crypto::decrypt(encrypted, "key2");

  // Decryption with wrong key should fail or produce garbage
  QVERIFY(decrypted != m_testData);
}

void TestCrypto::testEncryptDecryptLongText() {
  QString longText;
  for (int i = 0; i < 1000; i++) {
    longText += "This is line " + QString::number(i) + " of the test.\n";
  }

  QString encrypted = Crypto::encrypt(longText, m_testPassword);
  QVERIFY(!encrypted.isEmpty());

  QString decrypted = Crypto::decrypt(encrypted, m_testPassword);
  QCOMPARE(decrypted, longText);
}

void TestCrypto::testEncryptDecryptSpecialChars() {
  QString specialData = "Special chars: !@#$%^&*(){}[]|\\:\";<>?,./~`";

  QString encrypted = Crypto::encrypt(specialData, m_testPassword);
  QString decrypted = Crypto::decrypt(encrypted, m_testPassword);
  QCOMPARE(decrypted, specialData);
}

// ============ Salt Generation ============

void TestCrypto::testGenerateSalt() {
  QString salt = Crypto::generateSalt();
  QVERIFY(!salt.isEmpty());
  QVERIFY(salt.length() >= 16); // At least 16 chars for 8 bytes hex-encoded
}

void TestCrypto::testGenerateSaltUniqueness() {
  QString salt1 = Crypto::generateSalt();
  QString salt2 = Crypto::generateSalt();
  QVERIFY(salt1 != salt2);
}

// ============ Edge Cases ============

void TestCrypto::testEmptyPassword() {
  QString hash = Crypto::hashPassword("");
  QVERIFY(!hash.isEmpty());
}

void TestCrypto::testEmptyData() {
  QString encrypted = Crypto::encrypt("", m_testPassword);
  QString decrypted = Crypto::decrypt(encrypted, m_testPassword);
  QCOMPARE(decrypted, QString(""));
}

void TestCrypto::testVeryLongPassword() {
  QString longPassword;
  for (int i = 0; i < 1000; i++) {
    longPassword += "a";
  }

  QString hash = Crypto::hashPassword(longPassword);
  QVERIFY(!hash.isEmpty());
  QVERIFY(Crypto::verifyPassword(longPassword, hash));
}

void TestCrypto::testUnicodePassword() {
  QString unicodePassword = "密码測試パスワード";

  QString hash = Crypto::hashPassword(unicodePassword);
  QVERIFY(!hash.isEmpty());
  QVERIFY(Crypto::verifyPassword(unicodePassword, hash));
}

void TestCrypto::testUnicodeText() {
  QString unicodeText = "Türkçe: Merhaba Dünya, العربية: مرحبا, 中文: 你好世界";

  QString encrypted = Crypto::encrypt(unicodeText, m_testPassword);
  QString decrypted = Crypto::decrypt(encrypted, m_testPassword);
  QCOMPARE(decrypted, unicodeText);
}

QTEST_MAIN(TestCrypto)
#include "test_crypto.moc"
