#ifndef LINNOTE_CRYPTO_H
#define LINNOTE_CRYPTO_H

#include <QByteArray>
#include <QString>

/**
 * @brief Authenticated encryption and password hashing for note locking.
 *
 * Ciphertext format, version 2:
 *   v2:<iterations>:<saltHex>:<nonceHex>:<base64(ciphertext||tag)>
 * AES-256-GCM, key derived with PBKDF2-HMAC-SHA256 over a 16 byte random
 * salt, 12 byte random nonce, 16 byte authentication tag. The literal
 * "v2" is authenticated as additional data, so the version cannot be
 * stripped without the tag failing.
 *
 * Password hash format, version 2:
 *   pbkdf2-sha256$<iterations>$<saltHex>$<hashHex>
 *
 * LinNote 1.0.0 wrote a repeating-key XOR ciphertext ("salt:iv:base64")
 * and a hand-rolled SHA-256 chain hash ("salt:hex"). Both are still
 * readable so existing notes can be opened and re-encrypted, and neither
 * is ever written again. Use isLegacy() to decide whether a successful
 * decrypt should be followed by a re-encrypt.
 */
class Crypto {
public:
  /**
   * @brief Encrypt plaintext with a password, always in the current format
   * @return "v2:..." blob, or an empty string if inputs are empty or the
   *         cipher failed
   */
  static QString encrypt(const QString &plaintext, const QString &password);

  /**
   * @brief Decrypt a blob in either the current or the 1.0.0 format
   * @param ok set to true only when the plaintext is authentic; for a
   *        version 2 blob this means the GCM tag verified, so an empty
   *        return with *ok == true is a genuinely empty note
   * @return plaintext, or an empty string on failure
   */
  static QString decrypt(const QString &ciphertext, const QString &password,
                         bool *ok = nullptr);

  /**
   * @brief Hash a password for storage, always in the current format
   * @param salt Hex-encoded salt, generated when empty
   */
  static QString hashPassword(const QString &password,
                              const QString &salt = QString());

  /**
   * @brief Verify a password against a hash in either format
   *
   * The comparison is constant time, so a wrong password cannot be found
   * byte by byte from the time it takes to reject it.
   */
  static bool verifyPassword(const QString &password,
                             const QString &storedHash);

  /**
   * @brief True when a stored ciphertext or password hash is in the 1.0.0
   *        format and should be rewritten after the password is known
   */
  static bool isLegacy(const QString &stored);

  /**
   * @brief Generate a cryptographically random salt
   * @param length Salt length in bytes
   * @return Hex-encoded salt, empty on RNG failure
   */
  static QString generateSalt(int length = 16);

  /// PBKDF2 iteration count written by this version.
  static int currentIterations();

private:
  static QByteArray randomBytes(int length);
  static QByteArray deriveKey(const QString &password, const QByteArray &salt,
                              int iterations, int keyLength);
  static QString decryptLegacy(const QString &ciphertext,
                                         const QString &password, bool *ok);
  static bool verifyLegacyPassword(const QString &password,
                                   const QString &storedHash);
};

#endif // LINNOTE_CRYPTO_H
