#include "FindDialog.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>
#include <QVBoxLayout>

FindDialog::FindDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent)
    , m_editor(editor)
{
    setWindowTitle("Find");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(340);

    m_findEdit     = new QLineEdit;
    m_replaceEdit  = new QLineEdit;
    m_replaceLabel = new QLabel("Replace:");

    m_caseCheck      = new QCheckBox("Match &case");
    m_wholeWordCheck = new QCheckBox("&Whole word");

    m_findNextBtn   = new QPushButton("Find &Next");
    m_findPrevBtn   = new QPushButton("Find &Previous");
    m_replaceBtn    = new QPushButton("&Replace");
    m_replaceAllBtn = new QPushButton("Replace &All");
    auto *closeBtn  = new QPushButton("Close");

    m_findNextBtn->setDefault(true);

    auto *form = new QFormLayout;
    form->addRow("Find:", m_findEdit);
    form->addRow(m_replaceLabel, m_replaceEdit);

    auto *optRow = new QHBoxLayout;
    optRow->addWidget(m_caseCheck);
    optRow->addWidget(m_wholeWordCheck);
    optRow->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->addWidget(m_findPrevBtn);
    btnRow->addWidget(m_findNextBtn);
    btnRow->addWidget(m_replaceBtn);
    btnRow->addWidget(m_replaceAllBtn);
    btnRow->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addSpacing(4);
    layout->addLayout(optRow);
    layout->addSpacing(8);
    layout->addLayout(btnRow);
    layout->addWidget(closeBtn, 0, Qt::AlignRight);

    connect(m_findNextBtn,   &QPushButton::clicked, this, &FindDialog::findNext);
    connect(m_findPrevBtn,   &QPushButton::clicked, this, &FindDialog::findPrev);
    connect(m_replaceBtn,    &QPushButton::clicked, this, &FindDialog::replace);
    connect(m_replaceAllBtn, &QPushButton::clicked, this, &FindDialog::replaceAll);
    connect(closeBtn,        &QPushButton::clicked, this, &QDialog::close);

    setReplaceMode(false);
}

void FindDialog::setReplaceMode(bool enabled)
{
    m_replaceLabel->setVisible(enabled);
    m_replaceEdit->setVisible(enabled);
    m_replaceBtn->setVisible(enabled);
    m_replaceAllBtn->setVisible(enabled);
    setWindowTitle(enabled ? "Find & Replace" : "Find");
    adjustSize();
}

void FindDialog::findNext()
{
    const QString text = m_findEdit->text();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())      flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked()) flags |= QTextDocument::FindWholeWords;

    if (!m_editor->find(text, flags)) {
        QTextCursor c = m_editor->textCursor();
        c.movePosition(QTextCursor::Start);
        m_editor->setTextCursor(c);
        if (!m_editor->find(text, flags))
            QMessageBox::information(this, "Find", "Text not found.");
    }
}

void FindDialog::findPrev()
{
    const QString text = m_findEdit->text();
    if (text.isEmpty()) return;

    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (m_caseCheck->isChecked())      flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked()) flags |= QTextDocument::FindWholeWords;

    if (!m_editor->find(text, flags)) {
        QTextCursor c = m_editor->textCursor();
        c.movePosition(QTextCursor::End);
        m_editor->setTextCursor(c);
        if (!m_editor->find(text, flags))
            QMessageBox::information(this, "Find", "Text not found.");
    }
}

void FindDialog::replace()
{
    QTextCursor c = m_editor->textCursor();
    if (c.hasSelection()) {
        const Qt::CaseSensitivity cs = m_caseCheck->isChecked()
            ? Qt::CaseSensitive : Qt::CaseInsensitive;
        if (c.selectedText().compare(m_findEdit->text(), cs) == 0)
            c.insertText(m_replaceEdit->text());
    }
    findNext();
}

void FindDialog::replaceAll()
{
    const QString findText    = m_findEdit->text();
    const QString replaceText = m_replaceEdit->text();
    if (findText.isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())      flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordCheck->isChecked()) flags |= QTextDocument::FindWholeWords;

    QTextDocument *doc = m_editor->document();
    QTextCursor c(doc);
    c.beginEditBlock();
    int count = 0;
    while (!(c = doc->find(findText, c, flags)).isNull()) {
        c.insertText(replaceText);
        ++count;
    }
    c.endEditBlock();

    QMessageBox::information(this, "Replace All",
        count > 0
            ? QString("Replaced %1 occurrence(s).").arg(count)
            : "No occurrences found.");
}
