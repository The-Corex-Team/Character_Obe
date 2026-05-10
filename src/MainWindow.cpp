#include "MainWindow.h"
#include "ExportDialog.h"
#include "FindDialog.h"
#include "ui_characterobe.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextStream>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupMenus();
    setupStatusBar();

    connect(ui->editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::onTextChanged);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QWidget::customContextMenuRequested,
            this, &MainWindow::showContextMenu);

    updateWindowTitle();
    updateStatusBar();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) event->accept();
    else             event->ignore();
}

// ── Menus ─────────────────────────────────────────────────────────────────────

void MainWindow::setupMenus()
{
    // File
    QMenu *fileMenu = menuBar()->addMenu("&File");

    fileMenu->addAction("&New",    this, &MainWindow::newFile,    QKeySequence::New);
    fileMenu->addAction("&Open...", this, &MainWindow::openFile,  QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save",        this, &MainWindow::saveFile,   QKeySequence::Save);
    fileMenu->addAction("Save &As...",  this, &MainWindow::saveFileAs, QKeySequence::SaveAs);
    fileMenu->addSeparator();

    QMenu *importMenu = fileMenu->addMenu("&Import");
    importMenu->addAction("Plain Text (.txt)", this,
        [this]{ importFile("Text Files (*.txt)"); });
    importMenu->addAction("Log File (.log)", this,
        [this]{ importFile("Log Files (*.log)"); });

    fileMenu->addAction("&Export...", this, &MainWindow::exportFile);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", qApp, &QApplication::quit, QKeySequence::Quit);

    // Edit
    QMenu *editMenu = menuBar()->addMenu("&Edit");

    editMenu->addAction("&Undo",  ui->editor, &QPlainTextEdit::undo,  QKeySequence::Undo);
    editMenu->addAction("&Redo",  ui->editor, &QPlainTextEdit::redo,  QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction("Cu&t",   ui->editor, &QPlainTextEdit::cut,   QKeySequence::Cut);
    editMenu->addAction("&Copy",  ui->editor, &QPlainTextEdit::copy,  QKeySequence::Copy);
    editMenu->addAction("&Paste", ui->editor, &QPlainTextEdit::paste, QKeySequence::Paste);
    editMenu->addAction("&Delete", this, [this]{
        QTextCursor c = ui->editor->textCursor();
        if (c.hasSelection()) c.removeSelectedText();
        else                  c.deleteChar();
        ui->editor->setTextCursor(c);
    }, QKeySequence::Delete);
    editMenu->addSeparator();
    editMenu->addAction("Select &All", ui->editor, &QPlainTextEdit::selectAll,
                        QKeySequence::SelectAll);
    editMenu->addSeparator();
    editMenu->addAction("&Find...",           this, &MainWindow::find,
                        QKeySequence::Find);
    editMenu->addAction("Find && &Replace...", this, &MainWindow::findAndReplace,
                        QKeySequence(Qt::CTRL | Qt::Key_H));

    // View
    QMenu *viewMenu = menuBar()->addMenu("&View");

    m_wordWrapAction = viewMenu->addAction("&Word Wrap");
    m_wordWrapAction->setCheckable(true);
    m_wordWrapAction->setChecked(true);
    connect(m_wordWrapAction, &QAction::toggled, this, &MainWindow::toggleWordWrap);

    viewMenu->addSeparator();
    viewMenu->addAction("Zoom &In",    this, &MainWindow::zoomIn,
                        QKeySequence::ZoomIn);
    viewMenu->addAction("Zoom &Out",   this, &MainWindow::zoomOut,
                        QKeySequence::ZoomOut);
    viewMenu->addAction("&Reset Zoom", this, &MainWindow::resetZoom,
                        QKeySequence(Qt::CTRL | Qt::Key_0));
    viewMenu->addSeparator();

    m_statusBarAction = viewMenu->addAction("&Status Bar");
    m_statusBarAction->setCheckable(true);
    m_statusBarAction->setChecked(true);
    connect(m_statusBarAction, &QAction::toggled, this, &MainWindow::toggleStatusBar);

    // Help
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About Character Obe...", this, &MainWindow::showAbout);
}

void MainWindow::setupStatusBar()
{
    m_fileLabel = new QLabel("Untitled");
    m_posLabel  = new QLabel("Line 1, Col 1");
    statusBar()->addWidget(m_fileLabel, 1);
    statusBar()->addPermanentWidget(m_posLabel);
}

// ── File operations ────────────────────────────────────────────────────────────

void MainWindow::newFile()
{
    if (!maybeSave()) return;
    ui->editor->clear();
    setCurrentFile({});
}

void MainWindow::openFile()
{
    if (!maybeSave()) return;
    const QString path = QFileDialog::getOpenFileName(this, "Open File", {},
        "Text Files (*.txt);;Log Files (*.log);;All Files (*)");
    if (!path.isEmpty()) loadFileContents(path);
}

bool MainWindow::saveFile()
{
    if (m_currentFile.isEmpty()) return saveFileAs();
    if (!writeFile(m_currentFile, ui->editor->toPlainText())) return false;
    setCurrentFile(m_currentFile);
    return true;
}

bool MainWindow::saveFileAs()
{
    const QString path = QFileDialog::getSaveFileName(this, "Save File As", {},
        "Text Files (*.txt);;Log Files (*.log);;All Files (*)");
    if (path.isEmpty()) return false;
    if (!writeFile(path, ui->editor->toPlainText())) return false;
    setCurrentFile(path);
    return true;
}

void MainWindow::importFile(const QString &filter)
{
    if (!maybeSave()) return;
    const QString path = QFileDialog::getOpenFileName(this, "Import File", {},
        filter + ";;All Files (*)");
    if (!path.isEmpty()) loadFileContents(path);
}

void MainWindow::exportFile()
{
    const QString suggested = m_currentFile.isEmpty()
        ? "Untitled"
        : QFileInfo(m_currentFile).completeBaseName();

    ExportDialog dlg(suggested, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString dir = QFileDialog::getExistingDirectory(this, "Choose Export Folder");
    if (dir.isEmpty()) return;

    QString name = dlg.fileName().isEmpty() ? "Untitled" : dlg.fileName();
    const QString fullPath = dir + "/" + name + dlg.fileExtension();

    QString content = ui->editor->toPlainText();

    // Optional portable text header (off by default — metadata goes to the OS)
    if (dlg.includeMetadata()) {
        QString header;
        header += "; ============================================\n";
        header += "; Character Obe Export\n";
        header += QString("; Name:     %1\n").arg(name);
        header += QString("; Creator:  %1\n").arg(dlg.creator());
        header += QString("; Created:  %1\n").arg(dlg.createdDate().toString("yyyy-MM-dd"));
        header += QString("; Modified: %1\n").arg(dlg.modifiedDate().toString("yyyy-MM-dd"));
        header += "; ============================================\n\n";
        content = header + content;
    }

    if (!writeFile(fullPath, content)) return;

    // Apply real OS-level file metadata (timestamps + author stream)
    applyFileMetadata(fullPath, dlg.createdDate(), dlg.modifiedDate(), dlg.creator());

    QMessageBox::information(this, "Export",
        QString("Exported to:\n%1").arg(fullPath));
}

// ── Find / Replace ─────────────────────────────────────────────────────────────

void MainWindow::find()
{
    if (!m_findDialog)
        m_findDialog = new FindDialog(ui->editor, this);
    m_findDialog->setReplaceMode(false);
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::findAndReplace()
{
    if (!m_findDialog)
        m_findDialog = new FindDialog(ui->editor, this);
    m_findDialog->setReplaceMode(true);
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

// ── View ───────────────────────────────────────────────────────────────────────

void MainWindow::toggleWordWrap(bool on)
{
    ui->editor->setLineWrapMode(on ? QPlainTextEdit::WidgetWidth
                                   : QPlainTextEdit::NoWrap);
}

void MainWindow::zoomIn()  { ui->editor->zoomIn(); }
void MainWindow::zoomOut() { ui->editor->zoomOut(); }

void MainWindow::resetZoom()
{
    QFont f = ui->editor->font();
    f.setPointSize(11);
    ui->editor->setFont(f);
}

void MainWindow::toggleStatusBar(bool on)
{
    statusBar()->setVisible(on);
}

// ── Help ───────────────────────────────────────────────────────────────────────

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About Character Obe",
        "<b>Character Obe</b> v1.0<br>"
        "Part of <b>Obe Office</b><br><br>"
        "A plain text editor — no formatting, just characters.");
}

// ── Status bar ─────────────────────────────────────────────────────────────────

void MainWindow::updateStatusBar()
{
    const QTextCursor c = ui->editor->textCursor();
    m_posLabel->setText(
        QString("Line %1, Col %2")
            .arg(c.blockNumber() + 1)
            .arg(c.columnNumber() + 1));
}

// ── Context menu ───────────────────────────────────────────────────────────────

void MainWindow::showContextMenu(const QPoint &pos)
{
    QMenu menu(this);
    menu.addAction("&Undo",  ui->editor, &QPlainTextEdit::undo,  QKeySequence::Undo);
    menu.addAction("&Redo",  ui->editor, &QPlainTextEdit::redo,  QKeySequence::Redo);
    menu.addSeparator();
    menu.addAction("Cu&t",   ui->editor, &QPlainTextEdit::cut,   QKeySequence::Cut);
    menu.addAction("&Copy",  ui->editor, &QPlainTextEdit::copy,  QKeySequence::Copy);
    menu.addAction("&Paste", ui->editor, &QPlainTextEdit::paste, QKeySequence::Paste);
    menu.addAction("&Delete", this, [this]{
        QTextCursor c = ui->editor->textCursor();
        if (c.hasSelection()) c.removeSelectedText();
        else                  c.deleteChar();
        ui->editor->setTextCursor(c);
    });
    menu.addSeparator();
    menu.addAction("Select &All", ui->editor, &QPlainTextEdit::selectAll,
                   QKeySequence::SelectAll);
    menu.exec(ui->editor->mapToGlobal(pos));
}

// ── Internal helpers ───────────────────────────────────────────────────────────

void MainWindow::onTextChanged()
{
    if (!m_modified) {
        m_modified = true;
        updateWindowTitle();
    }
}

bool MainWindow::maybeSave()
{
    if (!m_modified) return true;
    const auto btn = QMessageBox::warning(this, "Character Obe",
        "The document has been modified.\nDo you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (btn == QMessageBox::Save)   return saveFile();
    if (btn == QMessageBox::Cancel) return false;
    return true;
}

void MainWindow::loadFileContents(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot open file:\n" + path);
        return;
    }
    QTextStream in(&file);
    ui->editor->setPlainText(in.readAll());
    setCurrentFile(path);
}

bool MainWindow::writeFile(const QString &path, const QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Cannot write file:\n" + path);
        return false;
    }
    QTextStream out(&file);
    out << content;
    return true;
}

void MainWindow::applyFileMetadata(const QString &path, const QDate &created,
                                    const QDate &modified, const QString &creator)
{
#ifdef Q_OS_WIN
    // Convert a QDate to a Windows FILETIME (100-ns intervals since 1601-01-01 UTC)
    auto toFileTime = [](const QDate &d) -> FILETIME {
        const QDateTime dt(d, QTime(0, 0, 0), Qt::UTC);
        ULARGE_INTEGER uli;
        uli.QuadPart = static_cast<quint64>(
            dt.toMSecsSinceEpoch() + 11644473600000LL) * 10000ULL;
        return FILETIME{ uli.LowPart, uli.HighPart };
    };

    // Set creation + modification timestamps visible in Windows Properties
    HANDLE hFile = CreateFileW(
        reinterpret_cast<const wchar_t *>(path.utf16()),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr, OPEN_EXISTING, 0, nullptr);

    if (hFile != INVALID_HANDLE_VALUE) {
        FILETIME ftCreated  = toFileTime(created);
        FILETIME ftModified = toFileTime(modified);
        SetFileTime(hFile, &ftCreated, nullptr, &ftModified);
        CloseHandle(hFile);
    }

    // Store creator in an NTFS alternate data stream (:ObeAuthor)
    // Readable by any tool that supports ADS; does not appear inside the file
    if (!creator.isEmpty()) {
        QFile ads(path + ":ObeAuthor");
        if (ads.open(QIODevice::WriteOnly | QIODevice::Text))
            QTextStream(&ads) << creator;
    }
#else
    Q_UNUSED(path); Q_UNUSED(created); Q_UNUSED(modified); Q_UNUSED(creator);
#endif
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    m_modified    = false;
    updateWindowTitle();
    m_fileLabel->setText(path.isEmpty() ? "Untitled" : strippedName(path));
}

void MainWindow::updateWindowTitle()
{
    const QString name = m_currentFile.isEmpty() ? "Untitled" : strippedName(m_currentFile);
    setWindowTitle((m_modified ? "*" : "") + name + " — Character Obe");
}

QString MainWindow::strippedName(const QString &fullPath) const
{
    return QFileInfo(fullPath).fileName();
}
