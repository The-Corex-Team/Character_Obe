#include "ExportDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

ExportDialog::ExportDialog(const QString &suggestedName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Export File");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumWidth(380);

    m_nameEdit    = new QLineEdit(suggestedName);
    m_creatorEdit = new QLineEdit;

    m_createdEdit  = new QDateEdit(QDate::currentDate());
    m_modifiedEdit = new QDateEdit(QDate::currentDate());
    m_createdEdit->setCalendarPopup(true);
    m_modifiedEdit->setCalendarPopup(true);
    m_createdEdit->setDisplayFormat("yyyy-MM-dd");
    m_modifiedEdit->setDisplayFormat("yyyy-MM-dd");

    m_formatCombo = new QComboBox;
    m_formatCombo->addItem("Plain Text (.txt)", ".txt");
    m_formatCombo->addItem("Log File (.log)",   ".log");

    m_metaCheck = new QCheckBox("Also embed metadata as text header (portable)");
    m_metaCheck->setChecked(false);

    auto *form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    form->addRow("Name:",     m_nameEdit);
    form->addRow("Creator:",  m_creatorEdit);
    form->addRow("Created:",  m_createdEdit);
    form->addRow("Modified:", m_modifiedEdit);
    form->addRow("Format:",   m_formatCombo);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->button(QDialogButtonBox::Ok)->setText("Export");
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addSpacing(4);
    layout->addWidget(m_metaCheck);
    layout->addSpacing(8);
    layout->addWidget(buttons);
}

QString ExportDialog::fileName()      const { return m_nameEdit->text().trimmed(); }
QString ExportDialog::creator()       const { return m_creatorEdit->text().trimmed(); }
QDate   ExportDialog::createdDate()   const { return m_createdEdit->date(); }
QDate   ExportDialog::modifiedDate()  const { return m_modifiedEdit->date(); }
QString ExportDialog::fileExtension() const { return m_formatCombo->currentData().toString(); }
bool    ExportDialog::includeMetadata() const { return m_metaCheck->isChecked(); }
