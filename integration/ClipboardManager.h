#ifndef LINNOTE_CLIPBOARDMANAGER_H
#define LINNOTE_CLIPBOARDMANAGER_H

#include <QImage>
#include <QObject>
#include <QString>
#include <QTimer>

class QClipboard;

/**
 * @brief Clipboard access manager for Wayland
 *
 * Provides safe clipboard access via Qt's QClipboard.
 * Uses timer-based polling for AutoPaste (Wayland compatible).
 */
class ClipboardManager : public QObject {
  Q_OBJECT

public:
  explicit ClipboardManager(QObject *parent = nullptr);

  /**
   * @brief Read clipboard text once
   *
   * This is a one-shot read - does not set up any watchers.
   * Safe for Wayland where clipboard access is restricted.
   *
   * @return Current clipboard text, or empty string if unavailable
   */
  QString getTextOnce();

  /**
   * @brief Check if clipboard has text content
   */
  bool hasText() const;

  /**
   * @brief Check if clipboard has image content
   */
  bool hasImage() const;

  /**
   * @brief Get clipboard image
   * @return Current clipboard image, or null image if unavailable
   */
  QImage getImage() const;

signals:
  /**
   * @brief Emitted when clipboard content changes (optional, for Phase 6)
   */
  void clipboardChanged();

  /**
   * @brief Emitted when new content is received during monitoring
   */
  void contentReceived(const QString &text);

public slots:
  /**
   * @brief Start monitoring clipboard for AutoPaste
   */
  void startMonitoring();

  /**
   * @brief Stop monitoring clipboard
   */
  void stopMonitoring();

private slots:
  void pollClipboard();

private:
  QClipboard *m_clipboard;
  QTimer *m_pollTimer;
  QString m_lastContent;
  bool m_isMonitoring = false;
};

#endif // LINNOTE_CLIPBOARDMANAGER_H
