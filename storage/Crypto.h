#ifndef LINNOTE_CRYPTO_H
#define LINNOTE_CRYPTO_H

#include <QByteArray>
#include <QString>

/**
 * @brief AES-256-CBC encryption helper for per-note encryption
 */
class Crypto {
public:
  /**
   * @brief Encrypt plaintext using password
   * @param plaintext The text to encrypt
   * @param password The password for encryption
   * @return Base64-encoded ciphertext (salt:iv:ciphertext)
   */
  static QString encrypt(const QString &plaintext, const QString &password);

  /**
   * @brief Decrypt ciphertext using password
   * @param ciphertext Base64-encoded ciphertext
   * @param password The password for decryption
   * @return Decrypted plaintext, empty on failure
   */
  static QString decrypt(const QString &ciphertext, const QString &password);

  /**
   * @brief Hash password for storage/verification
   * @param password The password to hash
   * @param salt Random salt (generated if empty)
   * @return Hash in format "salt:hash"
   */
  static QString hashPassword(const QString &password,
                              const QString &salt = QString());

  /**
   * @brief Verify password against stored hash
   * @param password The password to verify
   * @param storedHash The stored "salt:hash" string
   * @return true if password matches
   */
  static bool verifyPassword(const QString &password,
                             const QString &storedHash);

  /**
   * @brief Generate random salt
   * @param length Salt length in bytes (default 16)
   * @return Hex-encoded salt
   */
  static QString generateSalt(int length = 16);

private:
  static QByteArray deriveKey(const QString &password, const QByteArray &salt);
};

#endif // LINNOTE_CRYPTO_H
