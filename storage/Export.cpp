#include "Export.h"
#include "core/Settings.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFontMetrics>
#include <QMessageBox>
#include <QPainter>
#include <QPdfWriter>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

namespace Export {

// Helper: generate default path with timestamp
static QString defaultExportPath(const QString &extension) {
  QString defaultName =
      QStringLiteral("linnote_%1.%2")
          .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
          .arg(extension);
  QString documentsPath =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  return documentsPath + "/" + defaultName;
}

bool exportToTxt(const QString &content, QWidget *parent) {
  QString filePath = QFileDialog::getSaveFileName(
      parent, QObject::tr("Export as Text"), defaultExportPath("txt"),
      QObject::tr("Text Files (*.txt);;All Files (*)"));

  if (filePath.isEmpty())
    return false;

  bool success = exportToFile(content, filePath);

  if (!success && parent) {
    QMessageBox::warning(parent, QObject::tr("Export Failed"),
                         QObject::tr("Could not save the file."));
  }

  return success;
}

bool exportToMarkdown(const QString &content, QWidget *parent) {
  QString filePath = QFileDialog::getSaveFileName(
      parent, QObject::tr("Export as Markdown"), defaultExportPath("md"),
      QObject::tr("Markdown Files (*.md);;All Files (*)"));

  if (filePath.isEmpty())
    return false;

  // Add markdown header
  QString mdContent = QStringLiteral("# LinNote Export\n\n") + content;

  bool success = exportToFile(mdContent, filePath);

  if (!success && parent) {
    QMessageBox::warning(parent, QObject::tr("Export Failed"),
                         QObject::tr("Could not save the file."));
  }

  return success;
}

bool exportToPdf(const QString &content, QWidget *parent) {
  QString filePath = QFileDialog::getSaveFileName(
      parent, QObject::tr("Export as PDF"), defaultExportPath("pdf"),
      QObject::tr("PDF Files (*.pdf);;All Files (*)"));

  if (filePath.isEmpty())
    return false;

  // Get font settings
  Settings *s = Settings::instance();
  QString fontFamily = s->fontFamily();
  int fontSize = s->fontSize();

  // Create PDF writer
  QPdfWriter writer(filePath);
  writer.setPageSize(QPageSize::A4);
  writer.setResolution(300);

  QPainter painter(&writer);
  if (!painter.isActive()) {
    if (parent) {
      QMessageBox::warning(parent, QObject::tr("Export Failed"),
                           QObject::tr("Could not create PDF file."));
    }
    return false;
  }

  // Set font
  QFont font(fontFamily, fontSize);
  painter.setFont(font);

  QFontMetrics fm(font);
  int lineHeight = fm.height() + 4;
  int margin = 100;
  int textWidth = writer.width() - 2 * margin;
  int y = margin;
  int pageHeight = writer.height() - 2 * margin;

  // Split content and draw
  QStringList lines = content.split('\n');
  for (const QString &line : lines) {
    // Word wrap
    QStringList words = line.split(' ');
    QString currentLine;

    for (const QString &word : words) {
      QString testLine =
          currentLine.isEmpty() ? word : currentLine + " " + word;
      if (fm.horizontalAdvance(testLine) > textWidth &&
          !currentLine.isEmpty()) {
        painter.drawText(margin, y, currentLine);
        y += lineHeight;
        currentLine = word;

        // New page if needed
        if (y > pageHeight) {
          writer.newPage();
          y = margin;
        }
      } else {
        currentLine = testLine;
      }
    }

    // Draw remaining
    if (!currentLine.isEmpty()) {
      painter.drawText(margin, y, currentLine);
      y += lineHeight;
    } else {
      // Empty line
      y += lineHeight;
    }

    // New page if needed
    if (y > pageHeight) {
      writer.newPage();
      y = margin;
    }
  }

  painter.end();
  return true;
}

bool exportToCsv(const QString &content, QWidget *parent) {
  QString filePath = QFileDialog::getSaveFileName(
      parent, QObject::tr("Export as CSV"), defaultExportPath("csv"),
      QObject::tr("CSV Files (*.csv);;All Files (*)"));

  if (filePath.isEmpty())
    return false;

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    if (parent) {
      QMessageBox::warning(parent, QObject::tr("Export Failed"),
                           QObject::tr("Could not save the file."));
    }
    return false;
  }

  QTextStream out(&file);

  // CSV header
  out << "\"Status\",\"Content\"\n";

  // Process lines (detect checklist format)
  QStringList lines = content.split('\n');
  for (const QString &line : lines) {
    QString trimmed = line.trimmed();
    if (trimmed.isEmpty())
      continue;

    QString status = "item";
    QString text = trimmed;

    // Check for checkbox markers
    if (trimmed.startsWith("☑") || trimmed.startsWith("[x]") ||
        trimmed.startsWith("✓")) {
      status = "done";
      text = trimmed.mid(trimmed.indexOf(' ') + 1);
    } else if (trimmed.startsWith("☐") || trimmed.startsWith("[ ]") ||
               trimmed.startsWith("-")) {
      status = "todo";
      text = trimmed.mid(trimmed.indexOf(' ') + 1);
    }

    // Escape quotes
    text.replace("\"", "\"\"");

    out << "\"" << status << "\",\"" << text << "\"\n";
  }

  file.close();
  return true;
}

bool exportToFile(const QString &content, const QString &filePath) {
  QFile file(filePath);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    return false;
  }

  QTextStream out(&file);
  out << content;

  file.close();

  return true;
}

bool exportAllToZip(const QStringList &noteTitles,
                    const QStringList &noteContents, QWidget *parent) {
  if (noteTitles.isEmpty() || noteTitles.size() != noteContents.size()) {
    return false;
  }

  // Ask for zip file location
  QString zipPath = QFileDialog::getSaveFileName(
      parent, QObject::tr("Export All Notes as ZIP"), defaultExportPath("zip"),
      QObject::tr("ZIP Archives (*.zip);;All Files (*)"));

  if (zipPath.isEmpty())
    return false;

  // Create temp directory for note files
  QString tempDir =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
      "/linnote_export_" +
      QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
  QDir().mkpath(tempDir);

  // Write each note to a file
  QStringList fileNames;
  for (int i = 0; i < noteTitles.size(); ++i) {
    // Sanitize title for filename
    QString title = noteTitles[i];
    title.replace(QRegularExpression("[^\\w\\s-]"), "_");
    title = title.left(50).trimmed();
    if (title.isEmpty()) {
      title = QString("note_%1").arg(i + 1);
    }

    QString fileName = QString("%1.txt").arg(title);
    QString filePath = tempDir + "/" + fileName;

    // Handle duplicates
    int counter = 1;
    while (QFile::exists(filePath)) {
      fileName = QString("%1_%2.txt").arg(title).arg(counter++);
      filePath = tempDir + "/" + fileName;
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << noteContents[i];
      file.close();
      fileNames << fileName;
    }
  }

  // Use zip command (available on most Linux systems)
  QProcess zipProcess;
  zipProcess.setWorkingDirectory(tempDir);

  QStringList args;
  args << "-r" << zipPath << ".";

  zipProcess.start("zip", args);
  bool success = zipProcess.waitForFinished(30000); // 30 second timeout

  if (!success || zipProcess.exitCode() != 0) {
    // Cleanup temp files
    QDir(tempDir).removeRecursively();

    if (parent) {
      QMessageBox::warning(parent, QObject::tr("Export Failed"),
                           QObject::tr("Could not create ZIP archive. "
                                       "Make sure 'zip' is installed."));
    }
    return false;
  }

  // Cleanup temp files
  QDir(tempDir).removeRecursively();

  if (parent) {
    QMessageBox::information(
        parent, QObject::tr("Export Complete"),
        QObject::tr("Exported %1 notes to ZIP archive.").arg(fileNames.size()));
  }

  return true;
}

} // namespace Export
