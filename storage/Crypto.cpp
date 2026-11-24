#include "Crypto.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>
#include <QPasswordDigestor>
#include <QStringList>
#include <QtGlobal>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Nothing in this file logs a password, a derived key or a plaintext. A
// debug line that prints decrypted content puts it in the journal, where
// it outlives the process and defeats the feature.

namespace {

constexpr int kSaltBytes = 16;
constexpr int kNonceBytes = 12;
constexpr int kTagBytes = 16;
constexpr int kKeyBytes = 32; // AES-256
constexpr int kIterations = 600000;
constexpr int kMinIterations = 1000;
constexpr int kMaxIterations = 10000000;
const char kVersionTag[] = "v2";

// Key derivation as LinNote 1.0.0 wrote it: 5000 rounds of a SHA-256
// chain over the raw salt bytes. Only ever used to read old notes.
QByteArray legacyDeriveKey(const QString &password, const QByteArray &salt) {
  QByteArray data = salt + password.toUtf8();
  QByteArray key = data;
  for (int i = 0; i < 5000; ++i) {
    key = QCryptographicHash::hash(key + data, QCryptographicHash::Sha256);
  }
  return key;
}

// Password hashing as LinNote 1.0.0 wrote it: 10000 rounds of a SHA-256
// chain over the salt's hex TEXT concatenated with the password. The salt
// is used as a string here and as raw bytes in legacyDeriveKey above;
// that difference is part of the old format and must be preserved.
QString legacyHashPassword(const QString &password, const QString &saltHex) {
  QByteArray data = (saltHex + password).toUtf8();
  QByteArray hash = data;
  for (int i = 0; i < 10000; ++i) {
    hash = QCryptographicHash::hash(hash + data, QCryptographicHash::Sha256);
  }
  return saltHex + ":" + QString::fromLatin1(hash.toHex());
}

bool constantTimeEquals(const QByteArray &a, const QByteArray &b) {
  if (a.size() != b.size()) {
    return false;
  }
  if (a.isEmpty()) {
    return true;
  }
  return CRYPTO_memcmp(a.constData(), b.constData(),
                       static_cast<size_t>(a.size())) == 0;
}

} // namespace

int Crypto::currentIterations() { return kIterations; }

QByteArray Crypto::randomBytes(int length) {
  if (length <= 0) {
    return QByteArray();
  }
  QByteArray out(length, Qt::Uninitialized);
  if (RAND_bytes(reinterpret_cast<unsigned char *>(out.data()), length) != 1) {
    qWarning() << "Crypto: the system random number generator failed";
    return QByteArray();
  }
  return out;
}

QString Crypto::generateSalt(int length) {
  const QByteArray salt = randomBytes(length);
  if (salt.isEmpty()) {
    return QString();
  }
  return QString::fromLatin1(salt.toHex());
}

QByteArray Crypto::deriveKey(const QString &password, const QByteArray &salt,
                             int iterations, int keyLength) {
  return QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256,
                                            password.toUtf8(), salt,
                                            iterations, keyLength);
}

QString Crypto::encrypt(const QString &plaintext, const QString &password) {
  if (plaintext.isEmpty() || password.isEmpty()) {
    return QString();
  }

  const QByteArray salt = randomBytes(kSaltBytes);
  const QByteArray nonce = randomBytes(kNonceBytes);
  if (salt.isEmpty() || nonce.isEmpty()) {
    return QString();
  }

  const QByteArray key = deriveKey(password, salt, kIterations, kKeyBytes);
  if (key.size() != kKeyBytes) {
    qWarning() << "Crypto: key derivation failed";
    return QString();
  }

  const QByteArray plain = plaintext.toUtf8();
  QByteArray cipher(plain.size() + kTagBytes, Qt::Uninitialized);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    return QString();
  }

  bool fine = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr,
                                 nullptr) == 1 &&
              EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kNonceBytes,
                                  nullptr) == 1 &&
              EVP_EncryptInit_ex(
                  ctx, nullptr, nullptr,
                  reinterpret_cast<const unsigned char *>(key.constData()),
                  reinterpret_cast<const unsigned char *>(nonce.constData())) ==
                  1;

  // Bind the version to the ciphertext, so a blob cannot be relabelled.
  if (fine) {
    fine = EVP_EncryptUpdate(
               ctx, nullptr, nullptr,
               reinterpret_cast<const unsigned char *>(kVersionTag),
               static_cast<int>(sizeof(kVersionTag) - 1)) == 1;
  }

  int written = 0;
  if (fine) {
    fine = EVP_EncryptUpdate(
               ctx, reinterpret_cast<unsigned char *>(cipher.data()), &written,
               reinterpret_cast<const unsigned char *>(plain.constData()),
               static_cast<int>(plain.size())) == 1;
  }

  int finalWritten = 0;
  if (fine) {
    fine = EVP_EncryptFinal_ex(
               ctx, reinterpret_cast<unsigned char *>(cipher.data()) + written,
               &finalWritten) == 1;
  }

  const int bodySize = written + finalWritten;
  if (fine) {
    fine = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kTagBytes,
                               cipher.data() + bodySize) == 1;
  }

  EVP_CIPHER_CTX_free(ctx);

  if (!fine) {
    qWarning() << "Crypto: encryption failed";
    return QString();
  }

  cipher.resize(bodySize + kTagBytes);

  return QString::fromLatin1(kVersionTag) + ":" +
         QString::number(kIterations) + ":" +
         QString::fromLatin1(salt.toHex()) + ":" +
         QString::fromLatin1(nonce.toHex()) + ":" +
         QString::fromLatin1(cipher.toBase64());
}

QString Crypto::decrypt(const QString &ciphertext, const QString &password,
                        bool *ok) {
  if (ok) {
    *ok = false;
  }
  if (ciphertext.isEmpty() || password.isEmpty()) {
    return QString();
  }

  if (!ciphertext.startsWith(QString::fromLatin1(kVersionTag) + ":")) {
    return decryptLegacy(ciphertext, password, ok);
  }

  const QStringList parts = ciphertext.split(':');
  if (parts.size() != 5) {
    qWarning() << "Crypto: malformed ciphertext, expected 5 fields, got"
               << parts.size();
    return QString();
  }

  bool iterationsParsed = false;
  const int iterations = parts.at(1).toInt(&iterationsParsed);
  if (!iterationsParsed || iterations < kMinIterations ||
      iterations > kMaxIterations) {
    qWarning() << "Crypto: ciphertext declares an unusable iteration count";
    return QString();
  }

  const QByteArray salt = QByteArray::fromHex(parts.at(2).toLatin1());
  const QByteArray nonce = QByteArray::fromHex(parts.at(3).toLatin1());
  const QByteArray blob = QByteArray::fromBase64(parts.at(4).toLatin1());
  if (salt.size() != kSaltBytes || nonce.size() != kNonceBytes ||
      blob.size() < kTagBytes) {
    qWarning() << "Crypto: ciphertext field sizes are wrong";
    return QString();
  }

  const QByteArray body = blob.left(blob.size() - kTagBytes);
  QByteArray tag = blob.right(kTagBytes);

  const QByteArray key = deriveKey(password, salt, iterations, kKeyBytes);
  if (key.size() != kKeyBytes) {
    qWarning() << "Crypto: key derivation failed";
    return QString();
  }

  QByteArray plain(body.size() + kTagBytes, Qt::Uninitialized);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    return QString();
  }

  bool fine = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr,
                                 nullptr) == 1 &&
              EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kNonceBytes,
                                  nullptr) == 1 &&
              EVP_DecryptInit_ex(
                  ctx, nullptr, nullptr,
                  reinterpret_cast<const unsigned char *>(key.constData()),
                  reinterpret_cast<const unsigned char *>(nonce.constData())) ==
                  1;

  if (fine) {
    fine = EVP_DecryptUpdate(
               ctx, nullptr, nullptr,
               reinterpret_cast<const unsigned char *>(kVersionTag),
               static_cast<int>(sizeof(kVersionTag) - 1)) == 1;
  }

  int written = 0;
  if (fine && !body.isEmpty()) {
    fine = EVP_DecryptUpdate(
               ctx, reinterpret_cast<unsigned char *>(plain.data()), &written,
               reinterpret_cast<const unsigned char *>(body.constData()),
               static_cast<int>(body.size())) == 1;
  }

  if (fine) {
    fine = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kTagBytes,
                               tag.data()) == 1;
  }

  int finalWritten = 0;
  // This is the authentication check. A wrong password, a flipped bit or a
  // relabelled version all land here, and all of them must fail.
  if (fine) {
    fine = EVP_DecryptFinal_ex(
               ctx, reinterpret_cast<unsigned char *>(plain.data()) + written,
               &finalWritten) == 1;
  }

  EVP_CIPHER_CTX_free(ctx);

  if (!fine) {
    return QString();
  }

  plain.resize(written + finalWritten);
  if (ok) {
    *ok = true;
  }
  return QString::fromUtf8(plain);
}

QString Crypto::decryptLegacy(const QString &ciphertext,
                                        const QString &password, bool *ok) {
  const QStringList parts = ciphertext.split(':');
  if (parts.size() != 3) {
    qWarning() << "Crypto: unrecognised ciphertext format";
    return QString();
  }

  const QByteArray salt = QByteArray::fromHex(parts.at(0).toLatin1());
  // parts.at(1) is the 1.0.0 "iv" field. It was written but never used.
  const QByteArray cipher = QByteArray::fromBase64(parts.at(2).toLatin1());
  if (salt.isEmpty() || cipher.isEmpty()) {
    return QString();
  }

  const QByteArray key = legacyDeriveKey(password, salt);
  if (key.isEmpty()) {
    return QString();
  }

  QByteArray plain(cipher.size(), Qt::Uninitialized);
  for (int i = 0; i < cipher.size(); ++i) {
    plain[i] = cipher[i] ^ key[i % key.size()];
  }

  // The old format carries no authentication tag, so a wrong password
  // yields garbage rather than a detectable failure. The caller gates this
  // on verifyPassword, and the result is re-encrypted with a tag.
  if (ok) {
    *ok = true;
  }
  return QString::fromUtf8(plain);
}

QString Crypto::hashPassword(const QString &password, const QString &salt) {
  const QString saltHex = salt.isEmpty() ? generateSalt(kSaltBytes) : salt;
  if (saltHex.isEmpty()) {
    return QString();
  }

  const QByteArray saltBytes = QByteArray::fromHex(saltHex.toLatin1());
  if (saltBytes.isEmpty()) {
    return QString();
  }

  const QByteArray hash =
      deriveKey(password, saltBytes, kIterations, kKeyBytes);
  if (hash.size() != kKeyBytes) {
    qWarning() << "Crypto: password hashing failed";
    return QString();
  }

  return QStringLiteral("pbkdf2-sha256$%1$%2$%3")
      .arg(kIterations)
      .arg(saltHex)
      .arg(QString::fromLatin1(hash.toHex()));
}

bool Crypto::verifyPassword(const QString &password,
                            const QString &storedHash) {
  if (storedHash.isEmpty()) {
    return false;
  }

  if (!storedHash.contains('$')) {
    return verifyLegacyPassword(password, storedHash);
  }

  const QStringList parts = storedHash.split('$');
  if (parts.size() != 4 || parts.at(0) != QStringLiteral("pbkdf2-sha256")) {
    return false;
  }

  bool iterationsParsed = false;
  const int iterations = parts.at(1).toInt(&iterationsParsed);
  if (!iterationsParsed || iterations < kMinIterations ||
      iterations > kMaxIterations) {
    return false;
  }

  const QByteArray saltBytes = QByteArray::fromHex(parts.at(2).toLatin1());
  const QByteArray expected = QByteArray::fromHex(parts.at(3).toLatin1());
  if (saltBytes.isEmpty() || expected.isEmpty()) {
    return false;
  }

  const QByteArray actual =
      deriveKey(password, saltBytes, iterations, expected.size());
  return constantTimeEquals(actual, expected);
}

bool Crypto::verifyLegacyPassword(const QString &password,
                                  const QString &storedHash) {
  const QStringList parts = storedHash.split(':');
  if (parts.size() != 2) {
    return false;
  }
  const QString recomputed = legacyHashPassword(password, parts.at(0));
  return constantTimeEquals(recomputed.toLatin1(), storedHash.toLatin1());
}

bool Crypto::isLegacy(const QString &stored) {
  if (stored.isEmpty()) {
    return false;
  }
  if (stored.startsWith(QString::fromLatin1(kVersionTag) + ":")) {
    return false;
  }
  if (stored.contains('$')) {
    return false;
  }
  return true;
}
