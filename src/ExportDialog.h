#pragma once

#include <QDialog>
#include <QDate>

class QLineEdit;
class QDateEdit;
class QComboBox;
class QCheckBox;

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ExportDialog(const QString &suggestedName, QWidget *parent = nullptr);

    QString fileName()       const;
    QString creator()        const;
    QDate   createdDate()    const;
    QDate   modifiedDate()   const;
    QString fileExtension()  const;
    bool    includeMetadata() const;

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_creatorEdit;
    QDateEdit *m_createdEdit;
    QDateEdit *m_modifiedEdit;
    QComboBox *m_formatCombo;
    QCheckBox *m_metaCheck;
};
