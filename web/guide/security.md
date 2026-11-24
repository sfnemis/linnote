# Security

LinNote supports note encryption for sensitive data.

## Master Password

Protect notes with a master password.

### Setup

1. Go to **Settings** → **Security**
2. Click **Set Master Password**
3. Enter a strong password
4. Confirm password
5. **Save your recovery key!**

### Locking Notes

1. Select a note
2. Click the **Lock** icon (🔒)
3. Enter master password
4. Note is now encrypted

### Unlocking Notes

1. Click on a locked note
2. Enter master password
3. Note is decrypted for viewing

---

## Recovery Key

A 24-character recovery key is generated when you set up encryption.

> ⚠️ **Important:** Save this key somewhere safe! If you forget your master password, this is the only way to recover your notes.

### Using Recovery Key

1. Click **Forgot Password**
2. Enter your recovery key
3. Set a new master password

---

## Auto-Lock

Automatically lock LinNote after inactivity.

### Configure

1. **Settings** → **Security**
2. Enable **Auto-lock**
3. Set timeout (1, 5, 15, 30 minutes)

### Behavior

- App locks after timeout
- Requires master password to unlock
- Encrypted notes stay encrypted

---

## Encryption Details

- **Algorithm:** AES-256
- **Key Derivation:** PBKDF2
- **Storage:** Encrypted locally
- **No cloud sync:** Your data stays on your device

---

## Best Practices

1. Use a strong, unique master password
2. Save recovery key in a safe place (not on the same computer)
3. Enable auto-lock if using shared computer
4. Don't share your master password
