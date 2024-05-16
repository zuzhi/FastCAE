#include "FITKCopyrightDialog.h"
#include "ui_FITKCopyrightDialog.h"
#include <QTextBrowser>
#include <QFile>
#include <QDesktopServices>
#include <QGridLayout>
#include <QHeaderView>
#include <QTableWidgetItem>

FITKCopyrigntInfoDlg::FITKCopyrigntInfoDlg(QWidget* parent /*= nullptr*/)
{
    auto gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    _text = new QTextBrowser(this);
    gridLayout->addWidget(_text, 0, 0, 1, 1);

}

void FITKCopyrigntInfoDlg::setText(const QString & text)
{
    if (_text == nullptr) return;
    _text->clear();
    _text->setText(text);
}

FITKCopyRightDialog::FITKCopyRightDialog(QWidget* parent /*= nullptr*/)
    :QDialog(parent), _ui(new Ui::DlgCopyright)
{
    _ui->setupUi(this);
    _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    const int nR = _ui->tableWidget->rowCount();
    const int nC = _ui->tableWidget->columnCount();

    for (int i = 0; i < nR; i++)
    {
        for (int j = 0; j < nC; j++)
        {
            _ui->tableWidget->item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
    connect(_ui->tableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onTableItemClickedSlot(int, int)));
}

FITKCopyRightDialog::~FITKCopyRightDialog()
{
    if (_ui) delete _ui;
}



void FITKCopyRightDialog::onTableItemClickedSlot(int row, int col)
{
    if (col == 1)
    {
        this->browseLicense(row);
    }
    else if (col == 3)
    {
        this->openUrl(row, col);
    }
    return;
}

void FITKCopyRightDialog::browseLicense(int row)
{
    QTableWidgetItem* item = _ui->tableWidget->item(row, 0);
    if (item == nullptr) return;
    QString text = item->text();
    QString file = QString(":/License/License/%1.txt").arg(text);
    QFile f(file);
    if (!f.exists()) return;
    if (!f.open(QIODevice::ReadOnly)) return;
    QString alltext = f.readAll();
    f.close();
    FITKCopyrigntInfoDlg dlg(this);
    dlg.setText(alltext);
    dlg.setWindowTitle(text);
    dlg.exec();
}

void FITKCopyRightDialog::openUrl(int row, int col)
{

    QTableWidgetItem* item = _ui->tableWidget->item(row, col);
    if (item == nullptr) return;
    QString text = item->text();
    if (!text.startsWith("http")) return;
    QDesktopServices::openUrl(QUrl(text));
}

