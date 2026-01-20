#ifndef LINNOTE_EXPORT_H
#define LINNOTE_EXPORT_H

#include <QString>

class QWidget;

/**
 * @brief File export utilities
 */
namespace Export {
/**
 * @brief Export content to a text file
 *
 * Opens a file dialog to let the user choose the save location.
 *
 * @param content Text content to export
 * @param parent Parent widget for the file dialog
 * @return true if export was successful
 */
bool exportToTxt(const QString &content, QWidget *parent = nullptr);

/**
 * @brief Export content to Markdown file
 */
bool exportToMarkdown(const QString &content, QWidget *parent = nullptr);

/**
 * @brief Export content to PDF file
 * Uses font settings from Settings
 */
bool exportToPdf(const QString &content, QWidget *parent = nullptr);

/**
 * @brief Export content to CSV file
 * Best for checklist mode (each line = row)
 */
bool exportToCsv(const QString &content, QWidget *parent = nullptr);

/**
 * @brief Export content to a specific file path
 *
 * @param content Text content to export
 * @param filePath Destination file path
 * @return true if export was successful
 */
bool exportToFile(const QString &content, const QString &filePath);

/**
 * @brief Export all notes to a single ZIP file
 * Each note becomes a separate .txt file inside the archive
 *
 * @param noteTitles List of note titles
 * @param noteContents List of note contents (same order as titles)
 * @param parent Parent widget for file dialog
 * @return true if export was successful
 */
bool exportAllToZip(const QStringList &noteTitles,
                    const QStringList &noteContents, QWidget *parent = nullptr);
} // namespace Export

#endif // LINNOTE_EXPORT_H
