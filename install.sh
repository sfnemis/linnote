#!/bin/bash
#
# LinNote Installer
# Usage: curl -fsSL https://raw.githubusercontent.com/sfnemis/linnote/main/install.sh | bash
#
# Options:
#   --uninstall    Remove LinNote
#   --update       Update to latest version
#

set -e

# Colors (using $'...' for proper escape sequences)
RED=$'\033[0;31m'
GREEN=$'\033[0;32m'
YELLOW=$'\033[1;33m'
BLUE=$'\033[0;34m'
CYAN=$'\033[0;36m'
NC=$'\033[0m'

# Config
REPO_URL="https://github.com/sfnemis/linnote.git"
INSTALL_DIR="$HOME/.local/bin"
DESKTOP_DIR="$HOME/.local/share/applications"
ICON_DIR="$HOME/.local/share/icons/hicolor"
BUILD_DIR="/tmp/linnote-build-$$"
APP_NAME="linnote"

# Print functions
print_header() {
    echo -e "\n${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
}

print_step() {
    echo -e "${BLUE}▶${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Detect Linux distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO=$ID
        DISTRO_LIKE=$ID_LIKE
    elif [ -f /etc/arch-release ]; then
        DISTRO="arch"
    elif [ -f /etc/debian_version ]; then
        DISTRO="debian"
    elif [ -f /etc/fedora-release ]; then
        DISTRO="fedora"
    else
        DISTRO="unknown"
    fi
    
    echo "$DISTRO"
}

# Install dependencies based on distro
install_dependencies() {
    local distro=$(detect_distro)
    print_step "Detected distribution: $distro"
    
    case "$distro" in
        arch|manjaro|endeavouros|garuda)
            print_step "Installing dependencies with pacman..."
            sudo pacman -S --needed --noconfirm \
                qt6-base qt6-tools \
                kwindowsystem kglobalaccel \
                tesseract tesseract-data-eng tesseract-data-tur \
                cmake make gcc git
            ;;
        debian|ubuntu|linuxmint|pop)
            print_step "Installing dependencies with apt..."
            sudo apt-get update
            sudo apt-get install -y \
                qt6-base-dev libqt6sql6-sqlite \
                libkf6windowsystem-dev libkf6globalaccel-dev \
                tesseract-ocr tesseract-ocr-eng tesseract-ocr-tur \
                cmake build-essential git
            ;;
        fedora)
            print_step "Installing dependencies with dnf..."
            sudo dnf install -y \
                qt6-qtbase-devel \
                kf6-kwindowsystem-devel kf6-kglobalaccel-devel \
                tesseract tesseract-langpack-eng tesseract-langpack-tur \
                cmake gcc-c++ git
            ;;
        opensuse*|suse)
            print_step "Installing dependencies with zypper..."
            sudo zypper install -y \
                qt6-base-devel \
                kf6-kwindowsystem-devel kf6-kglobalaccel-devel \
                tesseract-ocr tesseract-ocr-traineddata-english tesseract-ocr-traineddata-turkish \
                cmake gcc-c++ git
            ;;
        *)
            print_warning "Unknown distribution: $distro"
            print_warning "Please install manually: Qt6, KDE Frameworks 6, CMake, Git"
            read -p "Continue anyway? [y/N] " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                exit 1
            fi
            ;;
    esac
    
    print_success "Dependencies installed"
}

# Clone or update repository
clone_repo() {
    print_step "Cloning LinNote repository..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
    fi
    
    git clone --depth 1 "$REPO_URL" "$BUILD_DIR"
    print_success "Repository cloned"
}

# Build the application
build_app() {
    print_step "Building LinNote..."
    
    cd "$BUILD_DIR"
    mkdir -p build
    cd build
    
    print_step "Running cmake..."
    cmake .. -DCMAKE_BUILD_TYPE=Release
    
    print_step "Compiling... (this may take a few minutes)"
    make -j$(nproc)
    
    if [ ! -f "LinNote" ]; then
        print_error "Build failed: LinNote binary not found"
        exit 1
    fi
    
    print_success "Build completed"
}

# Install the application
install_app() {
    print_step "Installing LinNote..."
    
    # Create directories
    mkdir -p "$INSTALL_DIR"
    mkdir -p "$DESKTOP_DIR"
    mkdir -p "$ICON_DIR/16x16/apps"
    mkdir -p "$ICON_DIR/32x32/apps"
    mkdir -p "$ICON_DIR/48x48/apps"
    mkdir -p "$ICON_DIR/64x64/apps"
    mkdir -p "$ICON_DIR/128x128/apps"
    mkdir -p "$ICON_DIR/256x256/apps"
    mkdir -p "$ICON_DIR/512x512/apps"
    
    # Copy binary
    cp "$BUILD_DIR/build/LinNote" "$INSTALL_DIR/$APP_NAME"
    chmod +x "$INSTALL_DIR/$APP_NAME"
    print_success "Binary installed to $INSTALL_DIR/$APP_NAME"
    
    # Copy desktop entry (original working format)
    cat > "$DESKTOP_DIR/linnote.desktop" << EOF
[Desktop Entry]
Type=Application
Name=LinNote
GenericName=Quick Notes
Comment=A quick scratchpad for Linux with global hotkey support
Exec=linnote %U
Icon=linnote
Terminal=false
Categories=Utility;TextEditor;
Keywords=notes;scratchpad;clipboard;paste;
StartupNotify=true
StartupWMClass=LinNote
MimeType=x-scheme-handler/linnote;
EOF
    print_success "Desktop entry installed"
    
    # CRITICAL: Also install to system-wide location for KDE Plasma
    # Some systems have XDG_DATA_DIRS that doesn't include ~/.local/share
    if [ -w /usr/share/applications ] || command -v sudo &> /dev/null; then
        sudo cp "$DESKTOP_DIR/linnote.desktop" /usr/share/applications/ 2>/dev/null || true
        print_success "Desktop entry installed system-wide"
    fi
    
    # Copy icons
    if [ -d "$BUILD_DIR/resources/icons/app" ]; then
        cp "$BUILD_DIR/resources/icons/app/linnote-16.png" "$ICON_DIR/16x16/apps/linnote.png" 2>/dev/null || true
        cp "$BUILD_DIR/resources/icons/app/linnote-32.png" "$ICON_DIR/32x32/apps/linnote.png" 2>/dev/null || true
        cp "$BUILD_DIR/resources/icons/app/linnote-48.png" "$ICON_DIR/48x48/apps/linnote.png" 2>/dev/null || true
        cp "$BUILD_DIR/resources/icons/app/linnote-64.png" "$ICON_DIR/64x64/apps/linnote.png" 2>/dev/null || true
        cp "$BUILD_DIR/resources/icons/app/linnote-128.png" "$ICON_DIR/128x128/apps/linnote.png" 2>/dev/null || true
        cp "$BUILD_DIR/resources/icons/app/linnote-256.png" "$ICON_DIR/256x256/apps/linnote.png" 2>/dev/null || true
        cp "$BUILD_DIR/resources/icons/app/linnote-512.png" "$ICON_DIR/512x512/apps/linnote.png" 2>/dev/null || true
        print_success "Icons installed"
    fi
    
    # Update icon cache
    if command -v gtk-update-icon-cache &> /dev/null; then
        gtk-update-icon-cache -f -t "$ICON_DIR" 2>/dev/null || true
    fi
    
    # Update desktop database (both user and system)
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
        sudo update-desktop-database /usr/share/applications 2>/dev/null || true
    fi
    
    # Update KDE Plasma menu cache
    if command -v kbuildsycoca6 &> /dev/null; then
        kbuildsycoca6 --noincremental 2>/dev/null || true
    elif command -v kbuildsycoca5 &> /dev/null; then
        kbuildsycoca5 2>/dev/null || true
    fi
}

# Setup PATH automatically
setup_path() {
    # Check if already in current PATH
    if [[ ":$PATH:" == *":$INSTALL_DIR:"* ]]; then
        print_success "$INSTALL_DIR is already in PATH"
        return
    fi
    
    print_step "Configuring PATH..."
    
    local path_line='export PATH="$HOME/.local/bin:$PATH"'
    local added=false
    
    # Add to .zshrc if it exists (check for UNCOMMENTED path line)
    if [ -f "$HOME/.zshrc" ]; then
        # Check if there's an uncommented .local/bin export
        if ! grep -v '^[[:space:]]*#' "$HOME/.zshrc" | grep -qF '.local/bin'; then
            echo "" >> "$HOME/.zshrc"
            echo "# LinNote PATH" >> "$HOME/.zshrc"
            echo "$path_line" >> "$HOME/.zshrc"
            print_success "Added to ~/.zshrc"
            added=true
        else
            print_success "PATH already in ~/.zshrc"
        fi
    fi
    
    # Add to .bashrc if it exists (check for UNCOMMENTED path line)
    if [ -f "$HOME/.bashrc" ]; then
        if ! grep -v '^[[:space:]]*#' "$HOME/.bashrc" | grep -qF '.local/bin'; then
            echo "" >> "$HOME/.bashrc"
            echo "# LinNote PATH" >> "$HOME/.bashrc"
            echo "$path_line" >> "$HOME/.bashrc"
            if [ "$added" = false ]; then
                print_success "Added to ~/.bashrc"
            fi
            added=true
        fi
    fi
    
    # Create .bashrc if neither exists
    if [ ! -f "$HOME/.bashrc" ] && [ ! -f "$HOME/.zshrc" ]; then
        echo "$path_line" >> "$HOME/.bashrc"
        print_success "Created ~/.bashrc with PATH"
        added=true
    fi
    
    # Export for current session
    export PATH="$INSTALL_DIR:$PATH"
}

# Cleanup
cleanup() {
    print_step "Cleaning up..."
    rm -rf "$BUILD_DIR"
    print_success "Cleanup completed"
}

# Uninstall
uninstall() {
    print_header "Uninstalling LinNote"
    
    echo -e "${CYAN}"
    echo "         .--.        "
    echo "        |o_o |       "
    echo "        |:_/ |       "
    echo "       //   \\ \\      "
    echo "      (|     | )     "
    echo "     /'\_   _/'\\     "
    echo "     \\___)=(___/    "
    echo -e "${NC}"
    echo "  Goodbye friend... 👋"
    echo ""
    sleep 1
    
    # Animated removal process
    local items=("Binary" "Desktop entry" "Autostart" "Icons" "User data" "Cache")
    local paths=(
        "$INSTALL_DIR/$APP_NAME"
        "$DESKTOP_DIR/linnote.desktop"
        "$HOME/.config/autostart/linnote.desktop"
        "icons"
        "data"
        "cache"
    )
    
    local frames=("(o< " "(o> " "(o^ " "(;_; " "(T_T " "(o< ")
    local i=0
    
    for item in "${items[@]}"; do
        local frame="${frames[$((i % ${#frames[@]}))]}"
        printf "\r  ${CYAN}%s${NC} Removing %s...     " "$frame" "$item"
        sleep 0.3
        
        case "$item" in
            "Binary")
                rm -f "$INSTALL_DIR/$APP_NAME"
                ;;
            "Desktop entry")
                rm -f "$DESKTOP_DIR/linnote.desktop"
                sudo rm -f /usr/share/applications/linnote.desktop 2>/dev/null || true
                ;;
            "Autostart")
                rm -f "$HOME/.config/autostart/linnote.desktop"
                rm -f "$HOME/.config/menus/applications-merged/linnote.menu"
                ;;
            "Icons")
                rm -f "$ICON_DIR/16x16/apps/linnote.png"
                rm -f "$ICON_DIR/32x32/apps/linnote.png"
                rm -f "$ICON_DIR/48x48/apps/linnote.png"
                rm -f "$ICON_DIR/64x64/apps/linnote.png"
                rm -f "$ICON_DIR/128x128/apps/linnote.png"
                rm -f "$ICON_DIR/256x256/apps/linnote.png"
                rm -f "$ICON_DIR/512x512/apps/linnote.png"
                ;;
            "User data")
                rm -rf "$HOME/.local/share/linnote"
                rm -rf "$HOME/.local/share/LinNote"
                rm -rf "$HOME/.local/share/sfnemis/LinNote"
                rm -f "$HOME/.local/state/LinNotestaterc"
                ;;
            "Cache")
                find "$HOME/.cache/drkonqi/crashes" -name "LinNote.*" -delete 2>/dev/null || true
                ;;
        esac
        
        printf "\r  ${GREEN}✓${NC} %s removed              \n" "$item"
        ((i++))
    done
    
    # Update caches
    echo ""
    echo -e "  ${BLUE}(o<${NC} Updating system caches..."
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
        sudo update-desktop-database /usr/share/applications 2>/dev/null || true
    fi
    if command -v kbuildsycoca6 &> /dev/null; then
        kbuildsycoca6 --noincremental 2>/dev/null || true
    elif command -v kbuildsycoca5 &> /dev/null; then
        kbuildsycoca5 --noincremental 2>/dev/null || true
    fi
    
    echo ""
    echo -e "  ${CYAN}(^_^)/${NC} LinNote has been completely uninstalled!"
    echo ""
    echo "  Removed files:"
    echo "    • Binary:         ~/.local/bin/linnote"
    echo "    • Desktop (user): ~/.local/share/applications/linnote.desktop"
    echo "    • Desktop (sys):  /usr/share/applications/linnote.desktop"
    echo "    • Menu config:    ~/.config/menus/applications-merged/linnote.menu"
    echo "    • Autostart:      ~/.config/autostart/linnote.desktop"
    echo "    • Icons:          ~/.local/share/icons/hicolor/*/apps/linnote.png"
    echo "    • User data:      ~/.local/share/linnote/"
    echo "    • State:          ~/.local/state/LinNotestaterc"
    echo "    • Crash logs:     ~/.cache/drkonqi/crashes/LinNote.*"
    echo ""
    echo "  We hope to see you again! 🐧"
    echo ""
}

# Main
main() {
    case "${1:-}" in
        --uninstall)
            uninstall
            exit 0
            ;;
        --update)
            print_header "Updating LinNote"
            ;;
        *)
            print_header "Installing LinNote"
            ;;
    esac
    
    echo -e "${CYAN}"
    echo "         .--.        "
    echo "        |o_o |       "
    echo "        |:_/ |       "
    echo "       //   \\ \\      "
    echo "      (|     | )     "
    echo "     /'\_   _/'\\    "
    echo "     \\___)=(___/    "
    echo -e "${NC}"
    echo -e "  ${CYAN}╦   ╦ ╔╗╔ ╔╗╔ ╔═╗ ╔╦╗ ╔═╗${NC}"
    echo -e "  ${CYAN}║   ║ ║║║ ║║║ ║ ║  ║  ║╣ ${NC}"
    echo -e "  ${CYAN}╩═╝ ╩ ╝╚╝ ╝╚╝ ╚═╝  ╩  ╚═╝${NC}"
    echo ""
    echo "  Quick Notes for Linux"
    echo ""
    
    install_dependencies
    clone_repo
    build_app
    install_app
    cleanup
    setup_path
    
    echo ""
    print_header "Installation Complete! 🎉"
    echo -e "  Run LinNote with: \033[0;36mlinnote\033[0m"
    echo "  Or find it in your applications menu"
    echo ""
}

# Run main with all arguments
main "$@"
