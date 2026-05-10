#pragma once

#include <QMainWindow>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FindDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void importFile(const QString &filter);
    void exportFile();

    void find();
    void findAndReplace();

    void toggleWordWrap(bool on);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void toggleStatusBar(bool on);

    void showAbout();
    void updateStatusBar();
    void onTextChanged();
    void showContextMenu(const QPoint &pos);

private:
    void setupMenus();
    void setupStatusBar();
    bool maybeSave();
    void loadFileContents(const QString &path);
    bool writeFile(const QString &path, const QString &content);
    void applyFileMetadata(const QString &path, const QDate &created,
                           const QDate &modified, const QString &creator);
    void setCurrentFile(const QString &path);
    void updateWindowTitle();
    QString strippedName(const QString &fullPath) const;

    Ui::MainWindow   *ui;
    FindDialog       *m_findDialog    = nullptr;
    QString           m_currentFile;
    bool              m_modified      = false;

    QAction          *m_wordWrapAction  = nullptr;
    QAction          *m_statusBarAction = nullptr;
    QLabel           *m_posLabel        = nullptr;
    QLabel           *m_fileLabel       = nullptr;
};
