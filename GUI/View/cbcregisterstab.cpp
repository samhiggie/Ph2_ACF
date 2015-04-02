#include "cbcregisterstab.h"
#include "ui_cbcregisterstab.h"

#include <QFont>
#include <QVector>
#include <QMap>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QStandardItem>
#include <QTableView>
#include <QStandardItem>
#include <QList>
#include <QScrollArea>
#include <QGroupBox>
#include <QVBoxLayout>

namespace GUI {

    CbcRegistersTab::CbcRegistersTab(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CbcRegistersTab),
        m_tabSh(new QTabWidget)
    {
        ui->setupUi(this);
        ui->loCbcs->addWidget(m_tabSh);
    }

    CbcRegistersTab::~CbcRegistersTab()
    {
        qDebug() << "Destructing " << this;
        delete ui;
    }

    void CbcRegistersTab::reset()
    {
        clearTabs();
    }

    void CbcRegistersTab::setupShTab(const int idSh)
    {

        reset();

        QTabWidget *tabSh =  new QTabWidget(this);

        QString title = QString("Sh %1").arg(idSh);
        m_tabSh->addTab(tabSh, title);
        m_mapTabSh[idSh] = tabSh;
    }

    void CbcRegistersTab::setupBeTab(const int idSh, const int idBe)
    {
        if (!m_mapTabSh.contains(idSh))
        {
            qDebug() << this->objectName() << "WARNING: idSh " << idSh << "is not valid";
        }

        QTabWidget *tabBe =  new QTabWidget(this);

        QString title = QString("Be %1").arg(idBe);
        m_mapTabSh[idSh]->addTab(tabBe, title);

        m_mapTabBe[idSh][idBe] = tabBe;
    }

    void CbcRegistersTab::setupFeTab(const int idSh, const int idBe, const int idFe)
    {
        if (!m_mapTabBe.contains(idSh))
        {
            qDebug() << this->objectName() << "WARNING: idSh " << idSh << "is not valid";
        }

        if (!m_mapTabBe[idSh].contains(idBe))
        {
            qDebug() << this->objectName() << "WARNING: in idSh " << idSh << ", idBe " << idBe << "is not valid";
        }

        QTabWidget *tabFe =  new QTabWidget(this);

        QString title = QString("Fe %1").arg(idFe);
        m_mapTabBe[idSh][idBe]->addTab(tabFe, title);

        m_mapTabFe[idSh][idBe][idFe] = tabFe;

    }

    void CbcRegistersTab::setupCbcRegGrid(const int idSh, const int idBe, const int idFe, const int idCbc)
    {
        if (!m_mapTabFe.contains(idSh))
        {
            qDebug() << this->objectName() << "WARNING: idSh " << idSh << "is not valid";
        }

        if (!m_mapTabFe[idSh].contains(idBe))
        {
            qDebug() << this->objectName() << "WARNING: in idSh " << idSh << ", idBe " << idBe << "is not valid";
        }

        if (!m_mapTabFe[idSh][idBe].contains(idFe))
        {
            qDebug() << this->objectName() << "WARNING: in idSh " << idSh << " & idBe " << idBe << ", idFe " <<idFe << " is not valid";
        }


        QTabWidget *tabCbc =  new QTabWidget(this);

        for(int i=0; i<2; i++) //number of register PAGES
        {
            QWidget *client = new QWidget; //client widget for scroll area
            QScrollArea *scrollArea = new QScrollArea;
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(client); //add scroll area to client
            QGridLayout *loGrid = new QGridLayout;
            client->setLayout(loGrid);

            m_mapTabPage[idSh][idBe][idFe][idCbc].push_back(loGrid);

            QWidget *pageWidget = new QWidget;
            pageWidget->setLayout(new QVBoxLayout);
            pageWidget->layout()->addWidget(scrollArea);

            QString title = QString("Page %1").arg(i);
            tabCbc->addTab(pageWidget, title);
        }

        QString title = QString("CBC %1").arg(idCbc);
        m_mapTabFe[idSh][idBe][idFe]->addTab(tabCbc, title);
        m_mapTabCbc[idSh][idBe][idFe][idCbc] = tabCbc;

    }


    void CbcRegistersTab::createCbcRegisterValue(const int idSh, const int idBe, const int idFe, const int idCbc,
                                                 const std::map<std::string, CbcRegItem> mapReg) //for initial creation - later should find inside map
    {
        qDebug() << "populating values";
        int row = 0;
        int column = 0;
        int cSizeTitle = 0;
        int cSizeAddr = 0; //TODO size by page number
        QMap<QString, QLineEdit*> cWidgetMap;

        for (auto& kv : mapReg)
        {
            QFont font("Sans Serif", 8);
            QHBoxLayout *loHorz = new QHBoxLayout; //Lo to add to grid
            QSpacerItem *spacer = new QSpacerItem(50,50); //TODO play with this

            QLabel *lblRegTitle = new QLabel(this);
            QLabel *lblRegAddress = new QLabel(this);

            lblRegTitle->setText(QString::fromStdString(kv.first));
            lblRegTitle->setFont(font);

            auto cAddress = kv.second.fAddress;
            if (cAddress < 16) lblRegAddress->setText(QString("[0x0%1]").arg(QString::number(cAddress,16))); //appends padding 0
            else lblRegAddress->setText(QString("[0x%1]").arg(QString::number(cAddress,16)));
            lblRegAddress->setFont(font);

            QLineEdit *lineRegValue = new QLineEdit(this);
            lineRegValue->setFixedWidth(30);
            lineRegValue->setText(QString::number(kv.second.fValue));
            lineRegValue->setFont(font);

            loHorz->addWidget(lblRegTitle);
            loHorz->setAlignment(lblRegTitle, Qt::AlignLeft);
            loHorz->addWidget(lblRegAddress);
            loHorz->setAlignment(lblRegAddress, Qt::AlignLeft);
            loHorz->addWidget(lineRegValue);
            loHorz->setAlignment(lineRegValue, Qt::AlignLeft);
            loHorz->addSpacerItem(spacer);
            loHorz->addStretch(5);

            m_mapTabPage[idSh][idBe][idFe][idCbc][kv.second.fPage]->addLayout(loHorz, row, column);

            cWidgetMap.insert(QString::fromStdString(kv.first),lineRegValue);

            if (cSizeTitle < lblRegTitle->width()) cSizeTitle = lblRegTitle->width(); //find min width needed
            if (cSizeAddr < lblRegAddress->width()) cSizeAddr = lblRegAddress->width();

            ++row;

            if (row == 20)
            {
                ++column;
                row = 0;
            }

        }
        m_mapWidgets[idSh][idBe][idFe][idCbc] = cWidgetMap;
    }

    void CbcRegistersTab::updateCbcRegisterValues(const int idSh, const int idBe, const int idFe, const int idCbc,
                                                  const std::map<std::string, CbcRegItem> mapReg)
    {
        for(auto &kv : mapReg)
        {
            auto cValue = kv.second.fValue;
            m_mapWidgets[idSh][idBe][idFe][idCbc].value(QString::fromStdString(kv.first))->setText(QString::number(cValue));
        }
    }

    void CbcRegistersTab::createCbcRegItems()
    {
        int nCbc = 0;

        /*for(auto& cCbc : m_widgetMap)
        {
            std::vector<std::pair<std::string, std::uint8_t>> vecRegValues;
            for (auto& regName : cCbc.keys())
            {
                std::string regTitle = regName.toStdString();

                std::string regValueTemp = (cCbc.value(regName)->text()).toStdString();
                std::vector<uint8_t> stupidConversion(regValueTemp.begin(), regValueTemp.end());
                std::uint8_t regValue = *&stupidConversion[0];

                vecRegValues.push_back(std::make_pair(regTitle, regValue));
            }
            //emit writeCbcRegisters(nCbc, vecRegValues);
            ++nCbc;
        }*/
    }

    void CbcRegistersTab::clearTabs()
    {
       m_mapWidgets.clear();
       m_mapTabSh.clear();
       m_mapTabBe.clear();
       m_mapTabFe.clear();
       m_mapTabCbc.clear();
       m_mapTabPage.clear();
    }

    void CbcRegistersTab::on_btnRefresh_clicked()
    {
        emit refreshCbcRegisters();
    }

    void CbcRegistersTab::on_btnWrite_clicked()
    {
        createCbcRegItems();
    }

    void CbcRegistersTab::on_btnResetSoft_clicked()
    {
        return; //TODO
    }

    void CbcRegistersTab::on_btnHardReset_clicked()
    {
        return; //TODO
    }


}
