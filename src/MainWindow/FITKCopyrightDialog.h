#ifndef __FITKCOPYRIGHT_H___
#define __FITKCOPYRIGHT_H___

#include <QDialog>
#include <QString>
#include "MainWindowAPI.h"

namespace Ui
{
    class DlgCopyright;
}

class QTextBrowser;

class MAINWINDOWAPI FITKCopyrigntInfoDlg : public QDialog
{
public:
    FITKCopyrigntInfoDlg(QWidget* parent = nullptr);
    virtual ~FITKCopyrigntInfoDlg() = default;

    void setText(const QString & text);

private:
    QTextBrowser* _text{};
};


class MAINWINDOWAPI FITKCopyRightDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FITKCopyRightDialog(QWidget* parent = nullptr);
    virtual ~FITKCopyRightDialog();

private slots:
    void onTableItemClickedSlot(int r, int w);

private:
    void browseLicense(int row);

    void openUrl(int row, int col);

private:
    Ui::DlgCopyright* _ui{};



};


#endif
