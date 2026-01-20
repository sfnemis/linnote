#include "ClipboardManager.h"
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMimeData>

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject(parent), m_clipboard(QApplication::clipboard()),
      m_pollTimer(new QTimer(this)) {
  // Setup polling timer for AutoPaste (Wayland doesn't reliably emit
  // dataChanged)
  connect(m_pollTimer, &QTimer::timeout, this,
          &ClipboardManager::pollClipboard);
}

QString ClipboardManager::getTextOnce() {
  if (!m_clipboard) {
    return QString();
  }

  // Try to get text from clipboard
  QString text = m_clipboard->text();

  // On Wayland, clipboard might not be immediately available
  // if the source application is not running. This is expected behavior.
  return text;
}

bool ClipboardManager::hasText() const {
  if (!m_clipboard) {
    return false;
  }

  const QMimeData *mimeData = m_clipboard->mimeData();
  return mimeData && mimeData->hasText();
}

bool ClipboardManager::hasImage() const {
  if (!m_clipboard) {
    return false;
  }

  const QMimeData *mimeData = m_clipboard->mimeData();
  return mimeData && mimeData->hasImage();
}

QImage ClipboardManager::getImage() const {
  if (!m_clipboard) {
    return QImage();
  }

  return m_clipboard->image();
}

void ClipboardManager::startMonitoring() {
  if (m_isMonitoring) {
    return;
  }

  m_isMonitoring = true;
  m_lastContent = m_clipboard ? m_clipboard->text() : QString();

  // Start polling every 500ms (Wayland compatible)
  m_pollTimer->start(500);

  qDebug() << "ClipboardManager: Started monitoring (polling mode)";
}

void ClipboardManager::stopMonitoring() {
  if (!m_isMonitoring) {
    return;
  }

  m_isMonitoring = false;
  m_pollTimer->stop();

  qDebug() << "ClipboardManager: Stopped monitoring";
}

void ClipboardManager::pollClipboard() {
  if (!m_isMonitoring || !m_clipboard) {
    return;
  }

  QString newContent = m_clipboard->text();

  // Only emit if content actually changed and is not empty
  if (!newContent.isEmpty() && newContent != m_lastContent) {
    m_lastContent = newContent;
    emit contentReceived(newContent);
    qDebug() << "ClipboardManager: New content received:"
             << newContent.left(50);
  }
}
