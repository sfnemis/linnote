#!/bin/bash
# LinNote Installer - https://linnote.app
# Usage: curl -fsSL https://linnote.app/install.sh | bash
set -e

VERSION="2.0.1"
ASSUME_YES="no"
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

# Detect Distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "${ID:-unknown}"
    else
        echo "unknown"
    fi
}

# Install Dependencies
# - Qt6/KF6 runtime libraries
# - Tesseract OCR with language packs
# - GNOME: AppIndicator extension note
install_dependencies() {
    local distro=$(detect_distro)
    
    print_step "Installing dependencies for $distro..."
    
    case "$distro" in
        arch|manjaro|endeavouros|garuda|cachyos)
            # Arch-based: Qt6 and KF6 libs
            sudo pacman -S --needed --noconfirm \
                qt6-base qt6-wayland \
                kf6-kwindowsystem kf6-kglobalaccel \
                layer-shell-qt openssl 2>/dev/null || true
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
                libqt6sql6-sqlite \
                libkf6windowsystem6 libkf6globalaccel6 \
                tesseract-ocr tesseract-ocr-eng tesseract-ocr-tur 2>/dev/null || \
            sudo apt-get install -y -qq \
                libqt6core6 libqt6widgets6 libqt6gui6 libqt6dbus6 libqt6sql6 \
                libqt6sql6-sqlite \
                libkf6windowsystem6 libkf6globalaccel6 \
                tesseract-ocr tesseract-ocr-eng tesseract-ocr-tur 2>/dev/null || true
            ;;
        fedora)
            # Fedora: Qt6 and KF6 libs + Tesseract
            sudo dnf install -y -q \
                qt6-qtbase qt6-qtwayland \
                kf6-kwindowsystem kf6-kglobalaccel \
                layer-shell-qt openssl-libs \
                tesseract tesseract-langpack-eng tesseract-langpack-tur 2>/dev/null || true
            ;;
        opensuse*|suse*)
            # openSUSE
            sudo zypper install -y \
                libQt6Core6 libQt6Widgets6 libQt6Gui6 \
                libQt6Sql6 kf6-kwindowsystem kf6-kglobalaccel \
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

# Download Binary
# Selects appropriate binary based on distribution family
download_binary() {
    local distro=$(detect_distro)
    local BINARY_NAME=""

    # Releases carry x86_64 builds only. Without this check the installer
    # downloads one onto an arm64 machine, reports success, and leaves a
    # program that dies with "cannot execute binary file".
    local machine
    machine=$(uname -m)
    if [ "$machine" != "x86_64" ] && [ "$machine" != "amd64" ]; then
        echo ""
        print_step "No prebuilt binary for $machine."
        echo "    Build from source instead:"
        echo "      git clone https://github.com/sfnemis/linnote.git"
        echo "      cd linnote && cmake -B build -DCMAKE_BUILD_TYPE=Release"
        echo "      cmake --build build -j\$(nproc)"
        echo "      sudo cmake --install build"
        echo ""
        print_error "Unsupported architecture: $machine"
    fi
    
    # Each binary is built on the distribution it is named after and is only
    # offered there. LinNote 1.0.0 handed the Arch build to Fedora, RHEL and
    # openSUSE as well; those carry different Qt and glibc versions, so the
    # binary could fail to start with a symbol error. Arch has its own AUR
    # package and is not served a foreign binary at all.
    case "$distro" in
        fedora|rhel|centos|rocky|almalinux)
            BINARY_NAME="linnote-fedora"
            ;;
        debian|ubuntu|linuxmint|pop|elementary|zorin|kubuntu)
            BINARY_NAME="linnote-ubuntu"
            ;;
        arch|manjaro|endeavouros|garuda|cachyos)
            echo ""
            print_step "Arch and its derivatives have a package of their own:"
            echo "      yay -S linnote"
            echo ""
            print_error "No binary is published for $distro, use the AUR package"
            ;;
        *)
            echo ""
            print_step "No binary is published for $distro. Two options:"
            echo "      flatpak install flathub app.linnote.LinNote"
            echo "    or build from source, see https://github.com/sfnemis/linnote"
            echo ""
            print_error "Unsupported distribution: $distro"
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

# Install Desktop Entry (to /usr/share/applications/)
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

# Install Icons
install_icons() {
    print_step "Installing icons..."
    
    local ICON_DIR="$HOME/.local/share/icons/hicolor"
    local ICON_BASE="https://raw.githubusercontent.com/sfnemis/linnote/main/resources/icons/app"

    # Every size, in both the user and the system theme. Installing only the
    # 128px icon system-wide, which is what LinNote 1.0.0 did, leaves the
    # desktop scaling one bitmap up to 256 and beyond: that is the blurry
    # icon reported in GitHub issue #4.
    for size in 16 32 48 64 128 256 512; do
        mkdir -p "$ICON_DIR/${size}x${size}/apps"
        if curl -fsSL "$ICON_BASE/linnote-${size}.png" \
                -o "$ICON_DIR/${size}x${size}/apps/linnote.png" 2>/dev/null; then
            sudo mkdir -p "/usr/share/icons/hicolor/${size}x${size}/apps"
            sudo cp "$ICON_DIR/${size}x${size}/apps/linnote.png" \
                "/usr/share/icons/hicolor/${size}x${size}/apps/" 2>/dev/null || true
        fi
    done
    
    print_success "Icons installed"
}


# Update Caches
update_caches() {
    print_step "Updating menu cache..."
    
    sudo update-desktop-database /usr/share/applications 2>/dev/null || true
    sudo gtk-update-icon-cache -f /usr/share/icons/hicolor 2>/dev/null || true
    kbuildsycoca6 --noincremental 2>/dev/null || true
    
    print_success "Menu cache updated"
}

# Uninstall
uninstall() {
    local PURGE="${1:-no}"
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${CYAN}  Uninstalling LinNote${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""

    # Say what goes and what stays, and get a yes, before touching anything.
    # Until 2.0.0 this function deleted the notes database on the first
    # keystroke with no confirmation and no backup.
    local DATA_DIRS=""
    for d in "$HOME/.local/share/LinNote" "$HOME/.local/share/linnote" \
             "$HOME/.config/LinNote" "$HOME/.config/linnote"; do
        [ -d "$d" ] && DATA_DIRS="$DATA_DIRS  $d\n"
    done

    echo "  Will be removed:"
    echo "    the linnote program, its desktop entry and its icons"
    if [ "$PURGE" = "yes" ]; then
        echo "    YOUR NOTES AND SETTINGS:"
        [ -n "$DATA_DIRS" ] && printf "    %b" "$DATA_DIRS" || echo "      (none found)"
    fi
    echo ""
    echo "  Will be kept:"
    if [ "$PURGE" = "yes" ]; then
        echo "    nothing"
    else
        echo "    your notes and settings:"
        [ -n "$DATA_DIRS" ] && printf "    %b" "$DATA_DIRS" || echo "      (none found)"
        echo "    run with --purge as well if you want those deleted too"
    fi
    echo ""
    local answer=""
    if [ "$ASSUME_YES" = "yes" ]; then
        answer="yes"
        echo "  --yes given, continuing without asking."
    elif [ -t 0 ]; then
        printf "  Type yes to continue: "
        read -r answer
    elif { exec 3</dev/tty; } 2>/dev/null; then
        # Piped from curl, so stdin is the script. Ask on the terminal.
        # Opening the descriptor first is the only reliable test: a plain
        # redirect can still fail at read time on a box with no
        # controlling terminal, and then bash prints its own error.
        printf "  Type yes to continue: " > /dev/tty
        read -r answer <&3
        exec 3<&-
    else
        echo "  There is no terminal to confirm on, so nothing was removed."
        echo "  Run it from a terminal, or add --yes if you are scripting it."
        exit 1
    fi
    if [ "$answer" != "yes" ]; then
        echo ""
        echo "  Nothing was removed."
        exit 0
    fi
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
    
    # Caches are disposable and always go. Notes and settings only go when
    # the user asked for that explicitly.
    rm -rf "$HOME/.cache/LinNote" "$HOME/.cache/linnote" 2>/dev/null
    if [ "$PURGE" = "yes" ]; then
        rm -rf "$HOME/.config/LinNote" "$HOME/.config/linnote" 2>/dev/null
        rm -rf "$HOME/.local/share/LinNote" "$HOME/.local/share/linnote" 2>/dev/null
        print_success "Notes, settings and cache removed"
    else
        print_success "Cache removed, notes and settings kept"
    fi
    
    # Remove KDE global shortcuts
    if command -v kwriteconfig6 &>/dev/null; then
        kwriteconfig6 --file kglobalshortcutsrc --group linnote --key _k_friendly_name --delete 2>/dev/null || true
        kwriteconfig6 --file kglobalshortcutsrc --group LinNote --key _k_friendly_name --delete 2>/dev/null || true
    fi
    if command -v kwriteconfig5 &>/dev/null; then
        kwriteconfig5 --file kglobalshortcutsrc --group linnote --key _k_friendly_name --delete 2>/dev/null || true
        kwriteconfig5 --file kglobalshortcutsrc --group LinNote --key _k_friendly_name --delete 2>/dev/null || true
    fi
    # Delete LinNote's own group only. The old range ended ON the next
    # group header and deleted it too, which left the following
    # application's shortcuts headerless and broken.
    if [ -f "$HOME/.config/kglobalshortcutsrc" ]; then
        cp "$HOME/.config/kglobalshortcutsrc" \
           "$HOME/.config/kglobalshortcutsrc.linnote-backup" 2>/dev/null || true
        awk 'BEGIN{skip=0}
             /^\[/{ skip = ($0=="[linnote]" || $0=="[LinNote]") ? 1 : 0 }
             !skip{print}' \
            "$HOME/.config/kglobalshortcutsrc.linnote-backup" \
            > "$HOME/.config/kglobalshortcutsrc" 2>/dev/null || \
            cp "$HOME/.config/kglobalshortcutsrc.linnote-backup" \
               "$HOME/.config/kglobalshortcutsrc"
    fi
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

# Main
main() {
    case "${1:-}" in
        --uninstall|-u)
            PURGE_ARG="no"
            ASSUME_YES="no"
            for arg in "$@"; do
                [ "$arg" = "--purge" ] && PURGE_ARG="yes"
                [ "$arg" = "--yes" ] && ASSUME_YES="yes"
            done
            uninstall "$PURGE_ARG"
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
