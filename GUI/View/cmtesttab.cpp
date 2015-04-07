#pragma once

#include "cmtesttab.h"
#include "ui_cmtesttab.h"

namespace GUI {

    CmTestTab::CmTestTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CmTestTab)
    {
        ui->setupUi(this);
    }

    CmTestTab::~CmTestTab()
    {
        delete ui;
    }

    void CmTestTab::enable(bool enable)
    {
        ui->btnLaunch->setEnabled(enable);
    }

    void CmTestTab::getIsNoiseScan()
    {
        emit sendIsScan(ui->chbxScanNoise->isChecked());
    }

    void CmTestTab::on_btnLaunch_clicked()
    {
        emit globalEnable(false);
        emit startCmTest();
    }


}
