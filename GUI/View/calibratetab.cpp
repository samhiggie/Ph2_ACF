#pragma once

#include "calibratetab.h"
#include "ui_calibratetab.h"

namespace GUI{

    CalibrateTab::CalibrateTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CalibrateTab)
    {
        ui->setupUi(this);
    }

    CalibrateTab::~CalibrateTab()
    {
        delete ui;
    }

    void CalibrateTab::getIsFastCalib()
    {
        emit sendIsFastCalib(ui->radioFast->isChecked());
    }

    void CalibrateTab::getIsScan()
    {
        emit sendIsScan(ui->chbxScanV->isChecked());
    }

    void CalibrateTab::getIsBitwise()
    {
        emit sendIsBitwise(ui->chbxBit->isChecked());
    }

    void CalibrateTab::getIsAllCh()
    {
        emit sendIsAllCh(ui->chbxAllChan->isChecked());
    }

    void CalibrateTab::enable(bool enable)
    {
        ui->btnCalibrate->setEnabled(enable);
    }

    void CalibrateTab::on_btnCalibrate_clicked()
    {
        emit globalEnable(false);
        emit startCalibration();
    }


}
