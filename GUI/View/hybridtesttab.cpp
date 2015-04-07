#include "hybridtesttab.h"

#include <sstream>

#include "ui_hybridtesttab.h"
#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTabWidget>

#include "utils.h"

namespace GUI {

    HybridTestTab::HybridTestTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::HybridTestTab)
    {
        ui->setupUi(this);

        m_Vcth = 120;
        m_Events= 100;
    }

    HybridTestTab::~HybridTestTab()
    {
        qDebug() << "Destructing " << this;
        delete ui;
    }

    void HybridTestTab::enable(bool enable)
    {
        ui->btnStart->setEnabled(enable);
    }

    void HybridTestTab::getVcthDialValue()
    {
        emit sendVcthValue(m_Vcth);
    }

    void HybridTestTab::getEventsDial()
    {
        emit sendEventsNumber(m_Events);
    }

    void HybridTestTab::getIsRegTestChecked()
    {
        emit sendIsRegTestChecked(ui->checkTestReg->isChecked());
    }

    void HybridTestTab::getIsScanChecked()
    {
        emit sendIsScanChecked(ui->checkScan->isChecked());
    }

    void HybridTestTab::getIsHoleModeChecked()
    {
        emit sendIsHoleModeChecked(ui->checkHoleMode->isChecked());
    }

    void HybridTestTab::on_btnStart_clicked()
    {
        emit globalEnable(false);
        emit startHybridTest();
    }

    void HybridTestTab::on_dialVcth_sliderMoved(int position)
    {
        ui->txtVcth->setText(QString::number(position));
        m_Vcth = position;
    }

    void HybridTestTab::on_txtVcth_editingFinished()
    {
        int value = (ui->txtVcth->text()).toInt();
        if(value<256)
        {
            ui->dialVcth->setValue(value);
            m_Vcth = value;
        }
        else
        {
            ui->txtVcth->setText(QString("0"));
            ui->dialVcth->setValue(0);
            m_Vcth = value;
        }
    }

    void HybridTestTab::on_dialEvents_sliderMoved(int position)
    {
        ui->txtEvents->setText(QString::number(position));
        m_Events = position;
    }

    void HybridTestTab::on_txtEvents_editingFinished()
    {
        int value = (ui->txtVcth->text()).toInt();
        if(value<501)
        {
            ui->dialVcth->setValue(value);
            m_Vcth = value;
        }
        else
        {
            ui->txtVcth->setText(QString("0"));
            ui->dialVcth->setValue(0);
            m_Vcth = value;
        }
    }
}
