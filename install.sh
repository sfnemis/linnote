#!/bin/bash
# LinNote Installer - https://linnote.app
# Usage: curl -fsSL https://linnote.app/install.sh | bash
set -e

VERSION="1.0.0"
INSTALL_DIR="/usr/local/bin"

# Track installation (silent, non-blocking)
curl -s "https://api.counterapi.dev/v1/linnote/installs/up" > /dev/null 2>&1 &

# Colors
RED=$'\033[0;31m'
GREEN=$'\033[0;32m'
BLUE=$'\033[0;34m'
CYAN=$'\033[0;36m'
NC=$'\033[0m'

print_step() { echo -e "  ${BLUE}▶${NC} $1"; }
print_success() { echo -e "  ${GREEN}✓${NC} $1"; }
print_error() { echo -e "  ${RED}✗${NC} $1"; exit 1; }

# ============================================================================
# Detect Distribution
# ============================================================================
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "${ID:-unknown}"
    else
        echo "unknown"
    fi
}

# ============================================================================
# Install Dependencies
# - Qt6/KF6 runtime libraries
# - Tesseract OCR with language packs
# - GNOME: AppIndicator extension note
# ============================================================================
install_dependencies() {
    local distro=$(detect_distro)
    
    print_step "Installing dependencies for $distro..."
    
    case "$distro" in
        arch|manjaro|endeavouros|garuda|cachyos)
            # Arch-based: Qt6 and KF6 libs
            sudo pacman -S --needed --noconfirm \
                qt6-base qt6-wayland \
                kf6-kwindowsystem kf6-kglobalaccel \
                layer-shell-qt 2>/dev/null || true
            # OCR support (separate for visibility)
            print_step "Installing OCR support..."
            sudo pacman -S --needed --noconfirm \
                tesseract tesseract-data-eng tesseract-data-tur || true
            print_success "OCR support ready"
            ;;
        debian|ubuntu|linuxmint|pop|kubuntu|elementary|zorin)
            # Debian/Ubuntu-based: Qt6 and KF6 libs + Tesseract
            # Note: Package names differ between Ubuntu versions (t64 suffix in 24.04)
            sudo apt-get update -qq 2>/dev/null
            sudo apt-get install -y -qq \
                libqt6core6t64 libqt6widgets6t64 libqt6gui6t64 libqt6dbus6t64 libqt6sql6t64 \
                libkf6windowsystem6 libkf6globalaccel6 \
                tesseract-ocr tesseract-ocr-eng tesseract-ocr-tur 2>/dev/null || \
            sudo apt-get install -y -qq \
                libqt6core6 libqt6widgets6 libqt6gui6 libqt6dbus6 libqt6sql6 \
                libkf6windowsystem6 libkf6globalaccel6 \
                tesseract-ocr tesseract-ocr-eng tesseract-ocr-tur 2>/dev/null || true
            ;;
        fedora)
            # Fedora: Qt6 and KF6 libs + Tesseract
            sudo dnf install -y -q \
                qt6-qtbase qt6-qtwayland \
                kf6-kwindowsystem kf6-kglobalaccel \
                layer-shell-qt \
                tesseract tesseract-langpack-eng tesseract-langpack-tur 2>/dev/null || true
            ;;
        opensuse*|suse*)
            # openSUSE
            sudo zypper install -y \
                libQt6Core6 libQt6Widgets6 libQt6Gui6 \
                kf6-kwindowsystem kf6-kglobalaccel \
                tesseract-ocr tesseract-ocr-traineddata-eng 2>/dev/null || true
            ;;
        *)
            print_step "Unknown distribution. Please install manually:"
            echo "  - Qt6 base libraries"
            echo "  - KF6 WindowSystem and GlobalAccel"
            echo "  - Tesseract OCR"
            ;;
    esac
    
    print_success "Dependencies installed"
    
    # GNOME-specific note
    if [ "$XDG_CURRENT_DESKTOP" = "GNOME" ] || [[ "$XDG_CURRENT_DESKTOP" == *"GNOME"* ]]; then
        echo ""
        print_step "GNOME detected - for system tray support:"
        echo "    Install: AppIndicator and KStatusNotifierItem Support"
        echo "    https://extensions.gnome.org/extension/615/"
        echo ""
        print_step "For global hotkey (F12), add custom shortcut in GNOME Settings:"
        echo "    Command: dbus-send --session --type=method_call --dest=org.linnote.LinNote /LinNote org.linnote.LinNote.Toggle"
        echo ""
    fi
}

# ============================================================================
# Download Binary
# Selects appropriate binary based on distribution family
# ============================================================================
download_binary() {
    local distro=$(detect_distro)
    local BINARY_NAME=""
    
    # Select binary based on distro family
    case "$distro" in
        arch|manjaro|endeavouros|garuda|cachyos|fedora|rhel|centos|opensuse*)
            BINARY_NAME="linnote-arch"
            ;;
        debian)
            BINARY_NAME="linnote-debian"
            ;;
        ubuntu|linuxmint|pop|elementary|zorin|kubuntu)
            BINARY_NAME="linnote-ubuntu"
            ;;
        *)
            # Default to arch binary (newer Qt)
            BINARY_NAME="linnote-arch"
            ;;
    esac
    
    local BINARY_URL="https://github.com/sfnemis/linnote/releases/download/v${VERSION}/${BINARY_NAME}"
    
    print_step "Downloading LinNote v${VERSION} (${BINARY_NAME})..."
    
    local TEMP_FILE="/tmp/linnote"
    
    if command -v curl &> /dev/null; then
        curl -fsSL "$BINARY_URL" -o "$TEMP_FILE"
    elif command -v wget &> /dev/null; then
        wget -q "$BINARY_URL" -O "$TEMP_FILE"
    else
        print_error "curl or wget required"
    fi
    
    sudo install -m 755 "$TEMP_FILE" "$INSTALL_DIR/linnote"
    rm -f "$TEMP_FILE"
    print_success "Binary installed to $INSTALL_DIR/linnote"
}

# ============================================================================
# Install Desktop Entry (to /usr/share/applications/)
# ============================================================================
install_desktop_entry() {
    print_step "Installing desktop entry..."
    
    # Create desktop file (Slimbook format)
    sudo tee /usr/share/applications/linnote.desktop > /dev/null << EOF
[Desktop Entry]
Type=Application
Name=LinNote
Comment=LinNote - Quick Notes for Linux
Icon=linnote
Exec=$INSTALL_DIR/linnote
Terminal=false
Categories=Utility;
NoDisplay=false
StartupNotify=true
StartupWMClass=LinNote
Keywords=notes;scratchpad;clipboard;calculator;
EOF
    
    print_success "Desktop entry installed to /usr/share/applications/"
}

# ============================================================================
# Install Icons
# ============================================================================
install_icons() {
    print_step "Installing icons..."
    
    local ICON_DIR="$HOME/.local/share/icons/hicolor"
    local ICON_BASE="https://raw.githubusercontent.com/sfnemis/linnote/main/resources/icons/app"
    
    for size in 16 32 48 64 128 256; do
        mkdir -p "$ICON_DIR/${size}x${size}/apps"
        curl -fsSL "$ICON_BASE/linnote-${size}.png" -o "$ICON_DIR/${size}x${size}/apps/linnote.png" 2>/dev/null || true
    done
    
    # Also copy to system icons
    if [ -f "$ICON_DIR/128x128/apps/linnote.png" ]; then
        sudo mkdir -p /usr/share/icons/hicolor/128x128/apps
        sudo cp "$ICON_DIR/128x128/apps/linnote.png" /usr/share/icons/hicolor/128x128/apps/ 2>/dev/null || true
    fi
    
    print_success "Icons installed"
}


# ============================================================================
# Update Caches
# ============================================================================
update_caches() {
    print_step "Updating menu cache..."
    
    sudo update-desktop-database /usr/share/applications 2>/dev/null || true
    sudo gtk-update-icon-cache -f /usr/share/icons/hicolor 2>/dev/null || true
    kbuildsycoca6 --noincremental 2>/dev/null || true
    
    print_success "Menu cache updated"
}

# ============================================================================
# Uninstall
# ============================================================================
uninstall() {
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${CYAN}  Uninstalling LinNote${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Kill running LinNote process if any
    if pgrep -x "linnote" > /dev/null || pgrep -x "LinNote" > /dev/null; then
        print_step "Stopping running LinNote..."
        killall linnote 2>/dev/null || true
        killall LinNote 2>/dev/null || true
        sleep 1
        print_success "LinNote stopped"
    fi
    
    print_step "Removing all LinNote files..."
    
    # Remove binary from ALL possible locations
    sudo rm -f /usr/local/bin/linnote 2>/dev/null
    sudo rm -f /usr/bin/linnote 2>/dev/null
    rm -f "$HOME/.local/bin/linnote" 2>/dev/null
    print_success "Binary removed"
    
    # Remove desktop entries from ALL locations
    sudo rm -f /usr/share/applications/linnote.desktop 2>/dev/null
    rm -f "$HOME/.local/share/applications/linnote.desktop" 2>/dev/null
    print_success "Desktop entries removed"
    
    # Remove icons from ALL locations
    for size in 16 32 48 64 128 256 512; do
        rm -f "$HOME/.local/share/icons/hicolor/${size}x${size}/apps/linnote.png" 2>/dev/null
        sudo rm -f "/usr/share/icons/hicolor/${size}x${size}/apps/linnote.png" 2>/dev/null
    done
    rm -f "$HOME/.local/share/icons/hicolor/scalable/apps/linnote.svg" 2>/dev/null
    print_success "Icons removed"
    
    # Remove config, cache, and ALL app data (database, settings, etc.)
    rm -rf "$HOME/.config/LinNote" 2>/dev/null
    rm -rf "$HOME/.config/linnote" 2>/dev/null
    rm -rf "$HOME/.cache/LinNote" 2>/dev/null
    rm -rf "$HOME/.cache/linnote" 2>/dev/null
    rm -rf "$HOME/.local/share/LinNote" 2>/dev/null
    rm -rf "$HOME/.local/share/linnote" 2>/dev/null
    print_success "Config, cache, and database removed"
    
    # Remove KDE global shortcuts
    if command -v kwriteconfig6 &>/dev/null; then
        kwriteconfig6 --file kglobalshortcutsrc --group linnote --key _k_friendly_name --delete 2>/dev/null || true
        kwriteconfig6 --file kglobalshortcutsrc --group LinNote --key _k_friendly_name --delete 2>/dev/null || true
    fi
    if command -v kwriteconfig5 &>/dev/null; then
        kwriteconfig5 --file kglobalshortcutsrc --group linnote --key _k_friendly_name --delete 2>/dev/null || true
        kwriteconfig5 --file kglobalshortcutsrc --group LinNote --key _k_friendly_name --delete 2>/dev/null || true
    fi
    # Remove entire LinNote section from kglobalshortcutsrc
    sed -i '/\[linnote\]/,/^\[/d' "$HOME/.config/kglobalshortcutsrc" 2>/dev/null || true
    sed -i '/\[LinNote\]/,/^\[/d' "$HOME/.config/kglobalshortcutsrc" 2>/dev/null || true
    print_success "KDE global shortcuts removed"
    
    # Update caches
    sudo update-desktop-database /usr/share/applications 2>/dev/null || true
    update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
    kbuildsycoca6 --noincremental 2>/dev/null || true
    
    echo ""
    echo -e "  ${GREEN}LinNote completely uninstalled!${NC}"
    echo ""
    echo "  Removed:"
    echo "    • Binaries (all locations)"
    echo "    • Desktop entries (system + user)"
    echo "    • Icons (all sizes)"
    echo "    • Config + Cache + Database"
    echo "    • KDE global shortcuts"
    echo ""
}

# ============================================================================
# Main
# ============================================================================
main() {
    case "${1:-}" in
        --uninstall|-u)
            uninstall
            exit 0
            ;;
    esac
    
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${CYAN}  Installing LinNote v${VERSION}${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    install_dependencies
    download_binary
    install_desktop_entry
    install_icons
    update_caches
    
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "  ${GREEN}Installation Complete!${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    echo "  Run: linnote"
    echo "  Menu: Applications → Utilities → LinNote"
    echo ""
}

main "$@"
