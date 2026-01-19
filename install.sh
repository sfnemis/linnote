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

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

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
                qt6-base qt6-declarative qt6-tools \
                kf6 layer-shell-qt \
                cmake make gcc git
            ;;
        debian|ubuntu|linuxmint|pop)
            print_step "Installing dependencies with apt..."
            sudo apt-get update
            sudo apt-get install -y \
                qt6-base-dev qt6-declarative-dev qt6-tools-dev \
                libkf6windowsystem-dev libkf6globalaccel-dev \
                cmake build-essential git
            ;;
        fedora)
            print_step "Installing dependencies with dnf..."
            sudo dnf install -y \
                qt6-qtbase-devel qt6-qtdeclarative-devel \
                kf6-kwindowsystem-devel kf6-kglobalaccel-devel \
                cmake gcc-c++ git
            ;;
        opensuse*|suse)
            print_step "Installing dependencies with zypper..."
            sudo zypper install -y \
                qt6-base-devel qt6-declarative-devel \
                kf6-kwindowsystem-devel kf6-kglobalaccel-devel \
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
    
    cmake .. -DCMAKE_BUILD_TYPE=Release
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
    
    # Copy desktop entry
    cat > "$DESKTOP_DIR/linnote.desktop" << EOF
[Desktop Entry]
Type=Application
Name=LinNote
GenericName=Quick Notes
Comment=A quick scratchpad for Linux with global hotkey support
Exec=$INSTALL_DIR/$APP_NAME %U
Icon=linnote
Terminal=false
Categories=Utility;TextEditor;
Keywords=notes;scratchpad;clipboard;paste;
StartupNotify=true
StartupWMClass=LinNote
MimeType=x-scheme-handler/linnote;
EOF
    print_success "Desktop entry installed"
    
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
    
    # Update desktop database
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
    fi
}

# Check PATH
check_path() {
    if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
        print_warning "$INSTALL_DIR is not in your PATH"
        echo ""
        echo "Add this line to your ~/.bashrc or ~/.zshrc:"
        echo -e "${CYAN}export PATH=\"\$HOME/.local/bin:\$PATH\"${NC}"
        echo ""
    fi
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
    
    rm -f "$INSTALL_DIR/$APP_NAME"
    rm -f "$DESKTOP_DIR/linnote.desktop"
    rm -f "$ICON_DIR/16x16/apps/linnote.png"
    rm -f "$ICON_DIR/32x32/apps/linnote.png"
    rm -f "$ICON_DIR/48x48/apps/linnote.png"
    rm -f "$ICON_DIR/64x64/apps/linnote.png"
    rm -f "$ICON_DIR/128x128/apps/linnote.png"
    rm -f "$ICON_DIR/256x256/apps/linnote.png"
    rm -f "$ICON_DIR/512x512/apps/linnote.png"
    
    print_success "LinNote has been uninstalled"
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
    echo "  ╦   ╦ ╔╗╔ ╔╗╔ ╔═╗ ╔╦╗ ╔═╗"
    echo "  ║   ║ ║║║ ║║║ ║ ║  ║  ║╣ "
    echo "  ╩═╝ ╩ ╝╚╝ ╝╚╝ ╚═╝  ╩  ╚═╝"
    echo -e "${NC}"
    echo "  Quick Notes for Linux"
    echo ""
    
    install_dependencies
    clone_repo
    build_app
    install_app
    cleanup
    check_path
    
    echo ""
    print_header "Installation Complete! 🎉"
    echo "  Run LinNote with: ${CYAN}linnote${NC}"
    echo "  Or find it in your applications menu"
    echo ""
}

# Run main with all arguments
main "$@"
