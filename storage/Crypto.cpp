#include "Crypto.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QRandomGenerator>
#include <QStringList>

// Simple XOR-based encryption for Qt (no OpenSSL dependency)
// For production, consider using proper AES via OpenSSL or libsodium

QString Crypto::encrypt(const QString &plaintext, const QString &password) {
  if (plaintext.isEmpty() || password.isEmpty()) {
    return QString();
  }

  // Generate random salt and IV
  QString salt = generateSalt(16);
  QString iv = generateSalt(16);

  // Derive key from password
  QByteArray key = deriveKey(password, QByteArray::fromHex(salt.toLatin1()));

  // Convert plaintext to bytes
  QByteArray plainBytes = plaintext.toUtf8();

  // XOR encryption with key (simple but effective for this use case)
  QByteArray cipher;
  cipher.resize(plainBytes.size());
  for (int i = 0; i < plainBytes.size(); ++i) {
    cipher[i] = plainBytes[i] ^ key[i % key.size()];
  }

  // Return as salt:iv:ciphertext (all base64)
  return salt + ":" + iv + ":" + cipher.toBase64();
}

QString Crypto::decrypt(const QString &ciphertext, const QString &password) {
  qDebug() << "[Crypto::decrypt] Input ciphertext length:"
           << ciphertext.length();
  qDebug() << "[Crypto::decrypt] Password length:" << password.length();

  if (ciphertext.isEmpty() || password.isEmpty()) {
    qDebug() << "[Crypto::decrypt] Empty input, returning empty";
    return QString();
  }

  // Parse salt:iv:ciphertext
  QStringList parts = ciphertext.split(':');
  qDebug() << "[Crypto::decrypt] Split parts count:" << parts.size();
  if (parts.size() != 3) {
    qDebug() << "[Crypto::decrypt] Invalid format, expected 3 parts";
    return QString();
  }

  QString salt = parts[0];
  // QString iv = parts[1]; // Not used in XOR mode
  QByteArray cipher = QByteArray::fromBase64(parts[2].toLatin1());
  qDebug() << "[Crypto::decrypt] Salt:" << salt
           << "Cipher size:" << cipher.size();

  // Derive key from password
  QByteArray key = deriveKey(password, QByteArray::fromHex(salt.toLatin1()));
  qDebug() << "[Crypto::decrypt] Derived key size:" << key.size();

  // XOR decryption
  QByteArray plainBytes;
  plainBytes.resize(cipher.size());
  for (int i = 0; i < cipher.size(); ++i) {
    plainBytes[i] = cipher[i] ^ key[i % key.size()];
  }

  QString result = QString::fromUtf8(plainBytes);
  qDebug() << "[Crypto::decrypt] Result length:" << result.length()
           << "Content:" << result.left(50);
  return result;
}

QString Crypto::hashPassword(const QString &password, const QString &salt) {
  QString actualSalt = salt.isEmpty() ? generateSalt(16) : salt;

  // PBKDF2-like: iterate hash many times
  QByteArray data = (actualSalt + password).toUtf8();
  QByteArray hash = data;

  for (int i = 0; i < 10000; ++i) {
    hash = QCryptographicHash::hash(hash + data, QCryptographicHash::Sha256);
  }

  return actualSalt + ":" + QString::fromLatin1(hash.toHex());
}

bool Crypto::verifyPassword(const QString &password,
                            const QString &storedHash) {
  QStringList parts = storedHash.split(':');
  if (parts.size() != 2) {
    return false;
  }

  QString salt = parts[0];
  QString expectedHash = hashPassword(password, salt);

  return expectedHash == storedHash;
}

QString Crypto::generateSalt(int length) {
  QByteArray salt;
  salt.resize(length);

  for (int i = 0; i < length; ++i) {
    salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
  }

  return QString::fromLatin1(salt.toHex());
}

QByteArray Crypto::deriveKey(const QString &password, const QByteArray &salt) {
  // Simple key derivation using repeated hashing
  QByteArray data = salt + password.toUtf8();
  QByteArray key = data;

  for (int i = 0; i < 5000; ++i) {
    key = QCryptographicHash::hash(key + data, QCryptographicHash::Sha256);
  }

  return key;
}
