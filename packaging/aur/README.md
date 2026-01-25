# LinNote AUR Package

Bu klasör Arch User Repository (AUR) için paket dosyalarını içerir.

## AUR'a Gönderme Adımları

### 1. AUR Hesabı Oluştur
- https://aur.archlinux.org/register adresine git
- Hesap oluştur ve SSH key ekle

### 2. AUR Reposunu Klonla
```bash
git clone ssh://aur@aur.archlinux.org/linnote.git
cd linnote
```

### 3. Dosyaları Kopyala
```bash
cp /path/to/LinNote/packaging/aur/PKGBUILD .
cp /path/to/LinNote/packaging/aur/.SRCINFO .
```

### 4. SHA256 Hash Hesapla (Eğer release tag varsa)
```bash
makepkg -g >> PKGBUILD  # veya updpkgsums kullan
```

### 5. Test Et
```bash
makepkg -si  # Lokal olarak build ve install
```

### 6. AUR'a Gönder
```bash
git add PKGBUILD .SRCINFO
git commit -m "Initial upload: linnote 1.0.0"
git push
```

## Notlar

- İlk gönderimden önce projenin bir GitHub release tag'i (`v1.0.0`) olması gerekiyor
- `sha256sums` değerini gerçek hash ile değiştirmeyi unutma
- AUR paketi kaynak koddan derler, bu yüzden tüm build dependencies gerekli
