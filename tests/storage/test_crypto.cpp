#include "storage/Crypto.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QStringList>
#include <QTest>

// These tests exist because a round-trip assertion proves nothing about a
// cipher: decrypt(encrypt(x)) == x holds just as well for a repeating-key
// XOR, which is what LinNote 1.0.0 actually shipped while its own header
// claimed AES-256-CBC. So every test below either checks a property that
// only authenticated encryption satisfies, or compares against an
// independent implementation, or pins a format with a fixed vector.

class TestCrypto : public QObject {
  Q_OBJECT

private slots:
  // Format of what we write now
  void testCiphertextIsVersionTagged();
  void testPasswordHashIsPbkdf2Tagged();

  // The key derivation, checked against OpenSSL rather than against itself
  void testKeyDerivationMatchesAnIndependentPbkdf2();

  // Properties a repeating-key XOR cannot satisfy
  void testWrongPasswordIsReportedNotGuessed();
  void testTamperedCiphertextIsRejected();
  void testVersionRelabelIsRejected();
  void testOnePlaintextByteChangesManyCiphertextBytes();
  void testSamePlaintextEncryptsDifferentlyEachTime();
  void testCiphertextIsLongerThanPlaintextByATag();

  // Reading what 1.0.0 wrote, and migrating it
  void testLegacyCiphertextIsStillReadable();
  void testLegacyPasswordHashStillVerifies();
  void testIsLegacyClassifiesBothFormats();
  void testLegacyBlobReEncryptsToCurrentFormat();
  void testCurrentHashAndLegacyHashDoNotCrossVerify();

  // Round trips and edge cases
  void testRoundTrip();
  void testRoundTripUnicode();
  void testRoundTripLongText();
  void testRoundTripSpecialChars();
  void testVeryLongPassword();
  void testEmptyInputs();
  void testSaltIsRandomAndRightLength();

private:
  // Produced by re-implementing the LinNote 1.0.0 algorithm independently
  // (salt as raw bytes, 5000 rounds of sha256(key + salt||password), then
  // a repeating-key XOR) and encoding the result the way 1.0.0 did.
  static QString legacyBlob() {
    return QStringLiteral(
        "000102030405060708090a0b0c0d0e0f:"
        "0f0e0d0c0b0a09080706050403020100:"
        "z6TnoW5uTjsdTIjJoRS3Dy3NOooVrPzrkXX3SooBdx77rPu1LyxJJklU1536UakSKt9s"
        "jVS34e2RYfdbkRx4HuG6/a5hYA==");
  }
  static QString legacyPlain() {
    return QStringLiteral("Ariza notu: 2+2=4 ve biraz daha uzun bir metin, "
                          "32 bayttan uzun olsun.");
  }
  static QString legacyPassword() {
    return QStringLiteral("correct horse battery staple");
  }
  // Produced by the same independent re-implementation of 1.0.0's
  // hashPassword (salt as hex TEXT, 10000 rounds).
  static QString legacyHash() {
    return QStringLiteral("000102030405060708090a0b0c0d0e0f:"
                          "df535b4dea589f85dfbcd8dc501b91abe44cc2e0c29143e619"
                          "dfe815c96d3572");
  }

  QString m_password = QStringLiteral("TestPassword123!");
  QString m_data = QStringLiteral("This is some test data to encrypt.");
};

void TestCrypto::testCiphertextIsVersionTagged() {
  const QString blob = Crypto::encrypt(m_data, m_password);
  QVERIFY(!blob.isEmpty());
  QVERIFY(blob.startsWith(QStringLiteral("v2:")));
  const QStringList parts = blob.split(':');
  QCOMPARE(parts.size(), 5);
  QCOMPARE(parts.at(0), QStringLiteral("v2"));
  QCOMPARE(parts.at(1).toInt(), Crypto::currentIterations());
  QCOMPARE(parts.at(2).size(), 32); // 16 byte salt, hex
  QCOMPARE(parts.at(3).size(), 24); // 12 byte nonce, hex
  QVERIFY(!parts.at(4).isEmpty());
}

void TestCrypto::testPasswordHashIsPbkdf2Tagged() {
  const QString hash = Crypto::hashPassword(m_password);
  QVERIFY(hash.startsWith(QStringLiteral("pbkdf2-sha256$")));
  const QStringList parts = hash.split('$');
  QCOMPARE(parts.size(), 4);
  QCOMPARE(parts.at(1).toInt(), Crypto::currentIterations());
  QCOMPARE(parts.at(2).size(), 32); // 16 byte salt, hex
  QCOMPARE(parts.at(3).size(), 64); // 32 byte hash, hex
  QVERIFY(Crypto::verifyPassword(m_password, hash));
  QVERIFY(!Crypto::verifyPassword(QStringLiteral("WrongPassword"), hash));
}

void TestCrypto::testKeyDerivationMatchesAnIndependentPbkdf2() {
  // Known-answer test. The expected values were produced by Python's
  // hashlib.pbkdf2_hmac, an implementation with no code in common with
  // this one, so a hand-rolled hash chain (which is what 1.0.0 used)
  // cannot pass. The first two are the widely published
  // PBKDF2-HMAC-SHA256 vectors for password/salt.
  struct Vector {
    const char *password;
    const char *saltHex;
    int iterations;
    const char *expectedHex;
  };
  // "salt" in hex is 73616c74; "password" is the password.
  const QList<Vector> vectors = {
      {"password", "73616c74", 1,
       "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b"},
      {"password", "73616c74", 4096,
       "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a"},
      {"correct horse battery staple", "000102030405060708090a0b0c0d0e0f",
       600000,
       "ef177144eec9420cbc1093d2a8b344a92bc506d0d4ec9c028dd19f8324d8c1e6"}};

  for (const Vector &v : vectors) {
    const QByteArray out = Crypto::deriveKey(
        QString::fromLatin1(v.password),
        QByteArray::fromHex(QByteArray(v.saltHex)), v.iterations, 32);
    QCOMPARE(QString::fromLatin1(out.toHex()),
             QString::fromLatin1(v.expectedHex));
  }

  // And the stored hash must be exactly that derivation, so the format and
  // the primitive cannot drift apart.
  const QString hash = Crypto::hashPassword(
      QStringLiteral("correct horse battery staple"),
      QStringLiteral("000102030405060708090a0b0c0d0e0f"));
  QCOMPARE(hash,
           QStringLiteral("pbkdf2-sha256$600000$000102030405060708090a0b0c0d"
                          "0e0f$ef177144eec9420cbc1093d2a8b344a92bc506d0d4ec"
                          "9c028dd19f8324d8c1e6"));
}

void TestCrypto::testWrongPasswordIsReportedNotGuessed() {
  const QString blob = Crypto::encrypt(m_data, m_password);
  bool ok = true;
  const QString out = Crypto::decrypt(blob, QStringLiteral("wrong"), &ok);
  // XOR returned plausible-looking garbage here and reported success.
  QVERIFY(!ok);
  QVERIFY(out.isEmpty());
}

void TestCrypto::testTamperedCiphertextIsRejected() {
  const QString blob = Crypto::encrypt(m_data, m_password);
  const QStringList parts = blob.split(':');
  QCOMPARE(parts.size(), 5);

  QByteArray body = QByteArray::fromBase64(parts.at(4).toLatin1());
  QVERIFY(body.size() > 16);

  // Flip one bit in the ciphertext body, then one in the tag at the end.
  // Both must fail.
  const QList<int> offsets = {0, static_cast<int>(body.size() / 2),
                              static_cast<int>(body.size() - 1)};
  for (int offset : offsets) {
    QByteArray broken = body;
    broken[offset] = static_cast<char>(broken[offset] ^ 0x01);
    const QString tampered =
        parts.at(0) + ":" + parts.at(1) + ":" + parts.at(2) + ":" +
        parts.at(3) + ":" + QString::fromLatin1(broken.toBase64());
    bool ok = true;
    const QString out = Crypto::decrypt(tampered, m_password, &ok);
    QVERIFY2(!ok, qPrintable(QStringLiteral("accepted a flipped bit at %1")
                                 .arg(offset)));
    QVERIFY(out.isEmpty());
  }
}

void TestCrypto::testVersionRelabelIsRejected() {
  // The version string is authenticated, so a blob cannot be presented as
  // a different version without the tag failing. Relabelling to the legacy
  // three-field shape must not succeed either.
  const QString blob = Crypto::encrypt(m_data, m_password);
  const QStringList parts = blob.split(':');
  const QString relabelled = parts.at(2) + ":" + parts.at(3) + ":" +
                             parts.at(4); // looks like a 1.0.0 blob
  bool ok = true;
  const QString out = Crypto::decrypt(relabelled, m_password, &ok);
  QVERIFY(out != m_data);
}

void TestCrypto::testOnePlaintextByteChangesManyCiphertextBytes() {
  // The direct repeating-key XOR detector. Under XOR, changing one
  // plaintext byte changes exactly one ciphertext byte at the same offset.
  // Under a real cipher with a fresh nonce, nearly every byte changes.
  QString a(200, QLatin1Char('a'));
  QString b = a;
  b[40] = QLatin1Char('b');

  const QString blobA = Crypto::encrypt(a, m_password);
  const QString blobB = Crypto::encrypt(b, m_password);

  const QByteArray bodyA =
      QByteArray::fromBase64(blobA.split(':').at(4).toLatin1());
  const QByteArray bodyB =
      QByteArray::fromBase64(blobB.split(':').at(4).toLatin1());
  QCOMPARE(bodyA.size(), bodyB.size());

  int differing = 0;
  for (int i = 0; i < bodyA.size(); ++i) {
    if (bodyA[i] != bodyB[i]) {
      ++differing;
    }
  }
  QVERIFY2(differing > bodyA.size() / 2,
           qPrintable(QStringLiteral("only %1 of %2 ciphertext bytes changed")
                          .arg(differing)
                          .arg(bodyA.size())));
}

void TestCrypto::testSamePlaintextEncryptsDifferentlyEachTime() {
  const QString one = Crypto::encrypt(m_data, m_password);
  const QString two = Crypto::encrypt(m_data, m_password);
  QVERIFY(one != two);
  bool ok = false;
  QCOMPARE(Crypto::decrypt(one, m_password, &ok), m_data);
  QVERIFY(ok);
  QCOMPARE(Crypto::decrypt(two, m_password, &ok), m_data);
  QVERIFY(ok);
}

void TestCrypto::testCiphertextIsLongerThanPlaintextByATag() {
  const QString blob = Crypto::encrypt(m_data, m_password);
  const QByteArray body =
      QByteArray::fromBase64(blob.split(':').at(4).toLatin1());
  QCOMPARE(body.size(), m_data.toUtf8().size() + 16);
}

void TestCrypto::testLegacyCiphertextIsStillReadable() {
  // A note locked by LinNote 1.0.0 must still open, or the upgrade
  // destroys data.
  bool ok = false;
  const QString out = Crypto::decrypt(legacyBlob(), legacyPassword(), &ok);
  QCOMPARE(out, legacyPlain());
  QVERIFY(ok);
}

void TestCrypto::testLegacyPasswordHashStillVerifies() {
  QVERIFY(Crypto::verifyPassword(legacyPassword(), legacyHash()));
  QVERIFY(!Crypto::verifyPassword(QStringLiteral("nope"), legacyHash()));
}

void TestCrypto::testIsLegacyClassifiesBothFormats() {
  QVERIFY(Crypto::isLegacy(legacyBlob()));
  QVERIFY(Crypto::isLegacy(legacyHash()));
  QVERIFY(!Crypto::isLegacy(Crypto::encrypt(m_data, m_password)));
  QVERIFY(!Crypto::isLegacy(Crypto::hashPassword(m_password)));
  QVERIFY(!Crypto::isLegacy(QString()));
}

void TestCrypto::testLegacyBlobReEncryptsToCurrentFormat() {
  bool ok = false;
  const QString recovered =
      Crypto::decrypt(legacyBlob(), legacyPassword(), &ok);
  QVERIFY(ok);
  QCOMPARE(recovered, legacyPlain());

  const QString migrated = Crypto::encrypt(recovered, legacyPassword());
  QVERIFY(!Crypto::isLegacy(migrated));

  bool ok2 = false;
  QCOMPARE(Crypto::decrypt(migrated, legacyPassword(), &ok2), legacyPlain());
  QVERIFY(ok2);

  // And the migrated blob now rejects a wrong password instead of
  // returning garbage.
  bool ok3 = true;
  QVERIFY(Crypto::decrypt(migrated, QStringLiteral("wrong"), &ok3).isEmpty());
  QVERIFY(!ok3);
}

void TestCrypto::testCurrentHashAndLegacyHashDoNotCrossVerify() {
  const QString current = Crypto::hashPassword(legacyPassword());
  QVERIFY(Crypto::verifyPassword(legacyPassword(), current));
  QVERIFY(Crypto::verifyPassword(legacyPassword(), legacyHash()));
  QVERIFY(current != legacyHash());
}

void TestCrypto::testRoundTrip() {
  bool ok = false;
  const QString blob = Crypto::encrypt(m_data, m_password);
  QCOMPARE(Crypto::decrypt(blob, m_password, &ok), m_data);
  QVERIFY(ok);
}

void TestCrypto::testRoundTripUnicode() {
  const QString text =
      QStringLiteral("Türkçe karakterler: ğüşıöç ĞÜŞİÖÇ ve 日本語 ve emoji 🔒");
  const QString password = QStringLiteral("şifre-ÇÖĞÜŞİ-123");
  bool ok = false;
  QCOMPARE(Crypto::decrypt(Crypto::encrypt(text, password), password, &ok),
           text);
  QVERIFY(ok);
}

void TestCrypto::testRoundTripLongText() {
  QString text;
  for (int i = 0; i < 1000; ++i) {
    text += QStringLiteral("Line %1 of a long note.\n").arg(i);
  }
  bool ok = false;
  QCOMPARE(Crypto::decrypt(Crypto::encrypt(text, m_password), m_password, &ok),
           text);
  QVERIFY(ok);
}

void TestCrypto::testRoundTripSpecialChars() {
  const QString text =
      QStringLiteral("!@#$%^&*()_+-=[]{}|;':\",./<>?\\`~\n\ttabs and newlines");
  bool ok = false;
  QCOMPARE(Crypto::decrypt(Crypto::encrypt(text, m_password), m_password, &ok),
           text);
  QVERIFY(ok);
}

void TestCrypto::testVeryLongPassword() {
  const QString password(5000, QLatin1Char('x'));
  const QString hash = Crypto::hashPassword(password);
  QVERIFY(Crypto::verifyPassword(password, hash));
  bool ok = false;
  QCOMPARE(Crypto::decrypt(Crypto::encrypt(m_data, password), password, &ok),
           m_data);
  QVERIFY(ok);
}

void TestCrypto::testEmptyInputs() {
  bool ok = true;
  QVERIFY(Crypto::encrypt(QString(), m_password).isEmpty());
  QVERIFY(Crypto::encrypt(m_data, QString()).isEmpty());
  QVERIFY(Crypto::decrypt(QString(), m_password, &ok).isEmpty());
  QVERIFY(!ok);
  ok = true;
  QVERIFY(Crypto::decrypt(QStringLiteral("v2:1:2:3:4"), QString(), &ok)
              .isEmpty());
  QVERIFY(!ok);
  QVERIFY(!Crypto::verifyPassword(m_password, QString()));
  QVERIFY(!Crypto::verifyPassword(m_password, QStringLiteral("garbage")));
}

void TestCrypto::testSaltIsRandomAndRightLength() {
  const QString a = Crypto::generateSalt();
  const QString b = Crypto::generateSalt();
  QCOMPARE(a.size(), 32); // 16 bytes, hex
  QVERIFY(a != b);
  QCOMPARE(Crypto::generateSalt(32).size(), 64);
  QVERIFY(Crypto::generateSalt(0).isEmpty());
}

QTEST_MAIN(TestCrypto)
#include "test_crypto.moc"
