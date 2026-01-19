# LinNote — KDE Plasma Smoke Test

> **Gereksinimler:** KDE Plasma 6+ | Wayland | xdg-desktop-portal-kde

---

## 1. Kurulum Testi

### Flatpak Kurulumu
```bash
cd /path/to/LinNote
flatpak-builder --user --install --force-clean build-dir flatpak/org.example.LinNote.yaml
```

**Beklenen:** Build başarılı, uygulama kuruldu.

### Manuel Build (Geliştirme)
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/qt6
make -j$(nproc)
./LinNote
```

**Beklenen:** Uygulama başlatıldı, tray icon görünüyor.

---

## 2. Tray Icon Testi

| Test | Adım | Beklenen |
|------|------|----------|
| Görünürlük | Uygulamayı başlat | Tray icon görünüyor |
| Tek tık | Tray icon'a tıkla | Pencere açılıyor |
| Tekrar tık | Tray icon'a tekrar tıkla | Pencere kapanıyor |
| Context menu | Sağ tık | Show/Hide, Export, Quit menüsü |
| Quit | Context menu > Quit | Uygulama kapanıyor |

---

## 3. Global Hotkey Testi

### İlk Çalıştırma
1. Uygulamayı başlat
2. KDE portal dialog veya System Settings'de kısayol yapılandırma
3. İzin ver

**Beklenen:** Kısayol kaydedildi.

### Hotkey Toggle
| Test | Adım | Beklenen |
|------|------|----------|
| Toggle açık | `Super + N` tuşla | Pencere açılıyor |
| Toggle kapalı | Tekrar `Super + N` | Pencere kapanıyor |
| Escape | Pencere açıkken Escape | Pencere kapanıyor |

> **Not:** System Settings > Shortcuts'da "LinNote" altında kısayol görünebilir.

---

## 4. AutoPaste Testi

| Test | Adım | Beklenen |
|------|------|----------|
| Clipboard kopyala | Başka uygulamada metin kopyala | — |
| Pencere aç | Global hotkey ile aç | Kopyalanan metin otomatik yapıştırılıyor |
| Tekrar aç | Pencereyi kapat, tekrar aç | Yapıştırma tekrarlanmıyor (aynı session) |
| Boş değilse | Editörde metin varken aç | AutoPaste çalışmıyor (beklenen) |

---

## 5. Export Testi

| Test | Adım | Beklenen |
|------|------|----------|
| Metin yaz | Editöre metin yaz | — |
| Export | File > Export veya tray menu | Dosya dialog açılıyor |
| Kaydet | Konum seç, kaydet | .txt dosyası oluşuyor |
| İçerik kontrol | Dosyayı aç | İçerik doğru |

---

## 6. Pencere Davranışı

| Test | Adım | Beklenen |
|------|------|----------|
| Stays on top | Başka pencere seç | LinNote hala üstte |
| Close button | X'e tıkla | Pencere gizleniyor (kapanmıyor) |
| Focus | Pencere açılınca | Editör odakta, yazı yazılabilir |

---

## 7. KDE-Specific Testler (Opsiyonel)

| Test | Adım | Beklenen |
|------|------|----------|
| KWin rules | Pencereyi sağ tık > More Actions | KWin window rules uygulanabilir |
| Activities | Farklı activity'de | Davranış tutarlı |

---

## Bilinen Avantajlar (KDE)

> [!TIP]
> **KDE Avantajları**
> - Portal desteği stabil
> - WindowStaysOnTopHint güvenilir çalışıyor
> - System Settings entegrasyonu iyi

---

## Test Sonucu

- [ ] Tüm testler geçti
- [ ] Tarih: ____
- [ ] Plasma Sürümü: ____
- [ ] Notlar: ____
