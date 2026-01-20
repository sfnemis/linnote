/**
 * @file DesktopHelper.h
 * @brief Runtime Desktop Environment Detection for Cross-Platform Compatibility
 *
 * This module provides runtime detection of the current desktop environment
 * to enable appropriate behavior adaptations for KDE, GNOME, and other DEs.
 *
 * Design Philosophy:
 * - All detection is done at RUNTIME, not compile-time
 * - Single universal binary works across all desktop environments
 * - Graceful degradation with professional alternatives when features
 *   are unavailable on certain platforms
 *
 * Feature Matrix:
 * ┌─────────────────────┬─────────────┬─────────────┬───────────────┐
 * │ Feature             │ KDE Plasma  │ GNOME       │ Other DEs     │
 * ├─────────────────────┼─────────────┼─────────────┼───────────────┤
 * │ Always-on-Top       │ KWin DBus   │ N/A*        │ WindowHint    │
 * │ Global Shortcuts    │ Portal+KGA  │ Portal      │ Portal        │
 * │ System Tray         │ Native      │ Extension   │ Varies        │
 * │ Theme Integration   │ Native Qt   │ Fusion      │ Native Qt     │
 * └─────────────────────┴─────────────┴─────────────┴───────────────┘
 *
 * * GNOME Wayland does not support programmatic always-on-top.
 *   LinNote provides "Stay Visible" mode as a professional alternative
 *   that prevents auto-hide when clicking outside.
 */

#ifndef LINNOTE_DESKTOP_HELPER_H
#define LINNOTE_DESKTOP_HELPER_H

#include <QString>
#include <QtGlobal> // for qgetenv

namespace LinNote {

/**
 * @enum DesktopEnvironment
 * @brief Enumeration of supported desktop environments
 */
enum class DesktopEnvironment {
  Unknown,  ///< Unable to detect or unsupported DE
  KDE,      ///< KDE Plasma (5.x or 6.x)
  GNOME,    ///< GNOME Shell (includes Ubuntu GNOME, Pop!_OS)
  Xfce,     ///< Xfce desktop
  Cinnamon, ///< Linux Mint Cinnamon
  MATE,     ///< MATE desktop
  LXQt,     ///< LXQt desktop
  Budgie,   ///< Budgie desktop
  Pantheon, ///< elementary OS Pantheon
  Deepin,   ///< Deepin DDE
  Other     ///< Other recognized but unsupported DE
};

/**
 * @brief Detect the current desktop environment at runtime
 *
 * Detection is performed by checking environment variables:
 * - XDG_CURRENT_DESKTOP (primary)
 * - XDG_SESSION_DESKTOP (fallback)
 * - KDE_FULL_SESSION (KDE-specific)
 * - GNOME_DESKTOP_SESSION_ID (legacy GNOME)
 *
 * @return The detected DesktopEnvironment enum value
 */
inline DesktopEnvironment detectDesktopEnvironment() {
  // Read environment variables
  QByteArray xdgCurrent = qgetenv("XDG_CURRENT_DESKTOP");
  QByteArray xdgSession = qgetenv("XDG_SESSION_DESKTOP");
  QByteArray kdeSession = qgetenv("KDE_FULL_SESSION");
  QByteArray gnomeLegacy = qgetenv("GNOME_DESKTOP_SESSION_ID");

  // Normalize to uppercase for comparison
  QString current = QString::fromUtf8(xdgCurrent).toUpper();
  QString session = QString::fromUtf8(xdgSession).toUpper();

  // ═══════════════════════════════════════════════════════════════════
  // KDE Detection
  // XDG_CURRENT_DESKTOP can be "KDE" or "X-KDE" or include "PLASMA"
  // KDE_FULL_SESSION is set in all KDE sessions
  // ═══════════════════════════════════════════════════════════════════
  if (current.contains("KDE") || current.contains("PLASMA") ||
      session.contains("KDE") || session.contains("PLASMA") ||
      !kdeSession.isEmpty()) {
    return DesktopEnvironment::KDE;
  }

  // ═══════════════════════════════════════════════════════════════════
  // GNOME Detection
  // Includes: GNOME, ubuntu:GNOME, Unity, Pop (Pop!_OS)
  // ═══════════════════════════════════════════════════════════════════
  if (current.contains("GNOME") || current.contains("UNITY") ||
      current.contains("UBUNTU") || current.contains("POP") ||
      session.contains("GNOME") || !gnomeLegacy.isEmpty()) {
    return DesktopEnvironment::GNOME;
  }

  // ═══════════════════════════════════════════════════════════════════
  // Other Desktop Environments
  // ═══════════════════════════════════════════════════════════════════
  if (current.contains("XFCE") || session.contains("XFCE")) {
    return DesktopEnvironment::Xfce;
  }
  if (current.contains("CINNAMON") || session.contains("CINNAMON")) {
    return DesktopEnvironment::Cinnamon;
  }
  if (current.contains("MATE") || session.contains("MATE")) {
    return DesktopEnvironment::MATE;
  }
  if (current.contains("LXQT") || session.contains("LXQT")) {
    return DesktopEnvironment::LXQt;
  }
  if (current.contains("BUDGIE") || session.contains("BUDGIE")) {
    return DesktopEnvironment::Budgie;
  }
  if (current.contains("PANTHEON") || session.contains("PANTHEON")) {
    return DesktopEnvironment::Pantheon;
  }
  if (current.contains("DEEPIN") || current.contains("DDE") ||
      session.contains("DEEPIN")) {
    return DesktopEnvironment::Deepin;
  }

  // Check if we detected something but don't specifically support it
  if (!current.isEmpty() || !session.isEmpty()) {
    return DesktopEnvironment::Other;
  }

  return DesktopEnvironment::Unknown;
}

/**
 * @brief Check if running on KDE Plasma
 * @return true if desktop environment is KDE
 */
inline bool isKdeDesktop() {
  return detectDesktopEnvironment() == DesktopEnvironment::KDE;
}

/**
 * @brief Check if running on GNOME or GNOME-based DE
 * @return true if desktop environment is GNOME
 */
inline bool isGnomeDesktop() {
  return detectDesktopEnvironment() == DesktopEnvironment::GNOME;
}

/**
 * @brief Check if running on a GTK-based desktop environment
 *
 * GTK-based DEs may require Fusion Qt style to prevent
 * theme collision artifacts (popup ghosting, dropdown bugs).
 *
 * @return true if DE is GTK-based (GNOME, Xfce, Cinnamon, MATE, Budgie,
 * Pantheon)
 */
inline bool isGtkBasedDesktop() {
  DesktopEnvironment de = detectDesktopEnvironment();
  switch (de) {
  case DesktopEnvironment::GNOME:
  case DesktopEnvironment::Xfce:
  case DesktopEnvironment::Cinnamon:
  case DesktopEnvironment::MATE:
  case DesktopEnvironment::Budgie:
  case DesktopEnvironment::Pantheon:
    return true;
  default:
    return false;
  }
}

/**
 * @brief Check if the DE supports programmatic always-on-top
 *
 * On Wayland, only KDE Plasma supports programmatic window stacking
 * via KWin DBus scripting. GNOME's Mutter compositor intentionally
 * blocks this for security reasons.
 *
 * For DEs without support, LinNote provides "Stay Visible" mode
 * as a professional alternative.
 *
 * @return true if programmatic always-on-top is supported
 */
inline bool supportsAlwaysOnTop() {
  // Only KDE Plasma supports this on Wayland via KWin scripting
  // X11 session would support it via NET::KeepAbove, but we
  // can't reliably detect X11 vs Wayland here, so we're conservative
  return isKdeDesktop();
}

/**
 * @brief Get human-readable name of the desktop environment
 * @return QString with the DE name (e.g., "KDE Plasma", "GNOME")
 */
inline QString desktopEnvironmentName() {
  switch (detectDesktopEnvironment()) {
  case DesktopEnvironment::KDE:
    return QStringLiteral("KDE Plasma");
  case DesktopEnvironment::GNOME:
    return QStringLiteral("GNOME");
  case DesktopEnvironment::Xfce:
    return QStringLiteral("Xfce");
  case DesktopEnvironment::Cinnamon:
    return QStringLiteral("Cinnamon");
  case DesktopEnvironment::MATE:
    return QStringLiteral("MATE");
  case DesktopEnvironment::LXQt:
    return QStringLiteral("LXQt");
  case DesktopEnvironment::Budgie:
    return QStringLiteral("Budgie");
  case DesktopEnvironment::Pantheon:
    return QStringLiteral("Pantheon");
  case DesktopEnvironment::Deepin:
    return QStringLiteral("Deepin");
  case DesktopEnvironment::Other:
    return QStringLiteral("Other");
  case DesktopEnvironment::Unknown:
  default:
    return QStringLiteral("Unknown");
  }
}

/**
 * @brief Get the appropriate "pin" feature name for the current DE
 *
 * KDE: "Always on Top" (actual z-order control)
 * GNOME and others: "Stay Visible" (prevents auto-hide)
 *
 * This provides honest, accurate terminology that reflects what
 * the feature actually does on each platform.
 *
 * @param enabled Whether the pin mode is currently enabled
 * @return Localization-ready tooltip string
 */
inline QString getPinFeatureTooltip(bool enabled) {
  if (supportsAlwaysOnTop()) {
    // KDE: Full always-on-top support
    if (enabled) {
      return QStringLiteral("Always on Top: ON (Ctrl+T)");
    } else {
      return QStringLiteral("Always on Top: OFF (Ctrl+T)");
    }
  } else {
    // GNOME and others: Stay Visible mode
    if (enabled) {
      return QStringLiteral(
          "Stay Visible: ON (Ctrl+T)\n"
          "Window won't auto-hide when clicking outside.\n"
          "(True always-on-top is not available on this desktop)");
    } else {
      return QStringLiteral("Stay Visible: OFF (Ctrl+T)");
    }
  }
}

} // namespace LinNote

#endif // LINNOTE_DESKTOP_HELPER_H
