#include "setuptab.h"
#include "ui_setuptab.h"

#include <QDebug>
#include <QVariantMap>

#include <QListView>

#include <QStandardItemModel>
#include <QFileDialog>
#include <QProcess>

namespace GUI{

    SetupTab::SetupTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::SetupTab)
    {
        ui->setupUi(this);
    }

    SetupTab::~SetupTab()
    {
        qDebug() << "Destructing " << this;
        delete ui;
    }

    void SetupTab::enable(bool enable)
    {
        ui->btnConfig->setEnabled(enable);
        ui->btnInit->setEnabled(enable);
        ui->btnLoad->setEnabled(enable);
        ui->btnLoadCustom->setEnabled(enable);
    }

    void SetupTab::setHwTreeView(QStandardItemModel *model)
    {
        ui->treeTest->setModel(model);

        ui->btnInit->setEnabled(true);
        ui->btnConfig->setEnabled(false);
    }

    void SetupTab::onStatusUpdate(const QString &statusMsg)
    {
        ui->editMsgBox->append(statusMsg);
    }

    bool SetupTab::isCbc2Checked ()
    {
        return ui->radio2CBC->isChecked();
    }

    void SetupTab::on_radio2CBC_clicked()
    {
        ui->radio2CBC->toggle();
    }

    void SetupTab::on_radio8CBC_clicked()
    {
        ui->radio8CBC->toggle();
    }

    void SetupTab::on_btnConfig_clicked()
    {
        ui->btnConfig->setEnabled(false);
        ui->btnInit->setEnabled(false);
        ui->btnLoad->setEnabled(false);
        ui->btnLoadCustom->setEnabled(false);
        emit onBtnCfgClicked();
    }

    void SetupTab::on_btnLoad_clicked()
    {
        emit enableAllTabs(false);
        emit onBtnLoad();
        emit onBtnLoadSettingsClicked(isCbc2Checked());
        ui->btnInit->setEnabled(true);
        ui->btnConfig->setEnabled(false);
    }

    void SetupTab::on_btnInit_clicked()
    {
        emit enableAllTabs(false);
        ui->btnInit->setEnabled(false);
        ui->btnLoad->setEnabled(false);
        ui->btnLoad->setEnabled(false);
        ui->btnConfig->setEnabled(false);
        emit onBtnInitClicked();
    }

    void SetupTab::onInitFinished()
    {
        emit enableAllTabs(false);
        ui->btnConfig->setEnabled(true);
        ui->btnInit->setEnabled(true);
        ui->btnLoad->setEnabled(true);
        ui->btnLoadCustom->setEnabled(true);
    }

    void SetupTab::onConfigFinished()
    {
        emit enableAllTabs(true);
        ui->btnConfig->setEnabled(true);
        ui->btnInit->setEnabled(true);
        ui->btnLoad->setEnabled(true);
        ui->btnLoadCustom->setEnabled(true);
    }


    void SetupTab::on_radio2CBC_toggled(bool checked)
    {
        emit on2CbcToggle(checked);
    }

    void SetupTab::on_btnOpenDir_clicked()
    {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Open Settings"), "./settings", tr("Settings File (*.json)"));

        ui->lineSettings->setText(fileName);
    }

    void SetupTab::on_btnLoadCustom_clicked()
    {
        emit onBtnLoad();
        emit onBtnCustomLoadSettingsClicked(ui->lineSettings->text());
    }

    void SetupTab::on_btnTBrows_clicked()
    {
        QProcess *process = new QProcess(this);
        process->start("./GUI/Macros/TBrowser");
    }
}
