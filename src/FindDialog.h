#pragma once

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;
class QPlainTextEdit;

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    void setReplaceMode(bool enabled);

private slots:
    void findNext();
    void findPrev();
    void replace();
    void replaceAll();

private:
    QPlainTextEdit *m_editor;

    QLineEdit   *m_findEdit;
    QLineEdit   *m_replaceEdit;
    QLabel      *m_replaceLabel;
    QCheckBox   *m_caseCheck;
    QCheckBox   *m_wholeWordCheck;
    QPushButton *m_findNextBtn;
    QPushButton *m_findPrevBtn;
    QPushButton *m_replaceBtn;
    QPushButton *m_replaceAllBtn;
};
