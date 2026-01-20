# OCR (Screen Text Capture)

LinNote can extract text from any area of your screen using Tesseract OCR.

**Activate:**
```
ocr
```

Or use keyboard shortcut: `Ctrl+Shift+O`

---

## How It Works

1. Type `ocr` and press Enter (or press `Ctrl+Shift+O`)
2. Screen capture mode activates
3. Select the area containing text
4. Text is extracted and inserted into your note

---

## Requirements

Tesseract must be installed:

**Arch Linux:**
```bash
sudo pacman -S tesseract tesseract-data-eng
```

**Ubuntu/Debian:**
```bash
sudo apt install tesseract-ocr tesseract-ocr-eng
```

**Fedora:**
```bash
sudo dnf install tesseract tesseract-langpack-eng
```

**openSUSE:**
```bash
sudo zypper install tesseract-ocr tesseract-ocr-traineddata-english
```

---

## Language Support

LinNote supports multiple OCR languages. Configure in Settings → OCR.

### Available Languages

English (eng), Turkish (tur), German (deu), French (fra), Spanish (spa), Italian (ita), Portuguese (por), Russian (rus), Chinese Simplified (chi_sim), Japanese (jpn), Korean (kor), Arabic (ara)

> **Note:** Additional language packs must be installed separately via your package manager.

---

## OCR Settings

Access via Settings → OCR:

| Setting | Description |
|---------|-------------|
| **OCR Language** | Select recognition language |
| **Create new note** | Put OCR text in new note (vs. current) |

---

## Tips

1. **High contrast text** works best
2. **Larger font sizes** improve accuracy
3. **Clean backgrounds** give better results
4. For multiple languages, install additional Tesseract data packs
