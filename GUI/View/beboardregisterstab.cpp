#pragma once
#include "beboardregisterstab.h"
#include "ui_beboardregisterstab.h"

#include <QDebug>
#include <map>
#include <QString>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

namespace GUI {
    BeBoardRegistersTab::BeBoardRegistersTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::BeBoardRegistersTab),
        m_tabSh(new QTabWidget)
    {
        ui->setupUi(this);
        ui->loBeBoard->addWidget(m_tabSh); //lo name wrong
    }

    BeBoardRegistersTab::~BeBoardRegistersTab()
    {
        qDebug() << "Destructing " << this;
        delete ui;
    }

    void BeBoardRegistersTab::reset()
    {
        tabsClear();
    }

    void BeBoardRegistersTab::enable(bool enable)
    {
        ui->btnRefresh->setEnabled(enable);
        ui->btnWrite->setEnabled(enable);
    }

    void BeBoardRegistersTab::setupSh(const int idSh)
    {
        reset();

        QTabWidget *tabSh =  new QTabWidget(this);

        QString title = QString("Sh %1").arg(idSh);
        m_tabSh->addTab(tabSh, title);
        m_mapTabSh[idSh] = tabSh;
    }

    void BeBoardRegistersTab::setupBe(const int idSh, const int idBe)
    {
        if (!m_mapTabSh.contains(idSh))
        {
            qDebug() << this->objectName() << "WARNING: idSh " << idSh << "is not valid";
            return;
        }

        QTabWidget *tabBe =  new QTabWidget(this);

        QWidget *client = new QWidget; //client widget for scroll area
        QScrollArea *scrollArea = new QScrollArea;
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(client); //add scroll area to client

        QGridLayout *loGrid = new QGridLayout;
        client->setLayout(loGrid);

        QString title = QString("Be %1").arg(idBe);

        tabBe->setLayout(new QVBoxLayout);
        tabBe->layout()->addWidget(scrollArea);

        m_mapTabSh[idSh]->addTab(tabBe, title);

        m_mapBeGrid[idSh][idBe] = loGrid;

        m_mapTabBe[idSh][idBe] = tabBe;
    }

    void BeBoardRegistersTab::createBeBoardRegisterValues(const int idSh, const int idBe, const std::map<std::string, uint32_t> mapReg)
    {
        int row = 0;
        int column = 0;
        int cSizeTitle = 0;

        QMap<QString, QLineEdit*> cWidgetMap;

        for (auto& kv : mapReg)
        {
            QHBoxLayout *loHorz = new QHBoxLayout;
            QLabel *lblRegTitle = new QLabel(this);
            lblRegTitle->setText(QString::fromStdString(kv.first));
            QLineEdit *lineEdit = new QLineEdit(this);
            lineEdit->setText(QString::number(kv.second));

            loHorz->addWidget(lblRegTitle);
            loHorz->addWidget(lineEdit);

            m_mapBeGrid[idSh][idBe]->addLayout(loHorz, row, column);
            row++;

            cWidgetMap.insert(QString::fromStdString(kv.first), lineEdit);

        }

        m_mapWidgets[idSh][idBe] = cWidgetMap;
    }

    void BeBoardRegistersTab::refreshBeBoardRegisterValues(const int idSh, const int idBe, const std::map<std::string, uint32_t> mapReg)
    {
        for (auto& regName : mapReg)
        {
            m_mapWidgets[idSh][idBe][QString::fromStdString(regName.first)]->setText(QString::number(regName.second));
        }

    }

    void BeBoardRegistersTab::createBeRegItems()
    {
        for(auto& cSh : m_mapWidgets.keys())
        {
            for(auto& cBe : m_mapWidgets[cSh].keys())
            {
                QMap<QString, int> cMap;
                for (auto& regName: m_mapWidgets[cSh][cBe].keys())
                {
                    cMap[regName] = (m_mapWidgets[cSh][cBe][regName]->text()).toInt();
                }
                emit writeBeRegisters(cSh,cBe, cMap);
            }
        }
    }

    void BeBoardRegistersTab::tabsClear()
    {
        m_mapBeGrid.clear();
        m_mapTabBe.clear();
        m_mapTabSh.clear();

    }

    void BeBoardRegistersTab::on_btnRefresh_clicked()
    {
        emit globalEnable(false);
        emit refreshBeValues();
    }

    void BeBoardRegistersTab::on_btnWrite_clicked()
    {
        emit globalEnable(false);
        createBeRegItems();
    }

}
