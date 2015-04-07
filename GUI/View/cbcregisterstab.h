#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QTableView>
#include <map>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMap>

#include "Model/systemcontroller.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace Ui {
    class CbcRegistersTab;
}

namespace GUI {

    class CbcRegistersTab : public QWidget
    {
        Q_OBJECT

    public:
        explicit CbcRegistersTab(QWidget *parent);
        ~CbcRegistersTab();

    signals:
        void refreshCbcRegisters();
        void writeCbcRegisters(const int idSh, const int idBe, const int idFe, const int cbc,
                               std::vector<std::pair<std::string, std::uint8_t>>);

    public slots:
        void reset();
        void setupShTab(const int idSh);
        void setupBeTab(const int idSh, const int idBe);
        void setupFeTab(const int idSh, const int idBe, const int idFe);
        void setupCbcRegGrid(const int idSh, const int idBe, const int idFe, const int idCbc);
        void createCbcRegisterValue(const int idSh, const int idBe, const int idFe, const int idCbc, const std::map<std::string, CbcRegItem> mapReg);
        void updateCbcRegisterValues(const int idSh, const int idBe, const int idFe, const int idCbc, const std::map<std::string, CbcRegItem> mapReg);

    private slots:

        void onValueChanged(QString cMsg);

        void on_btnRefresh_clicked();

        void on_btnWrite_clicked();

    private:

        Ui::CbcRegistersTab *ui;

        QTabWidget *m_tabSh;

        QMap<int, QTabWidget*> m_mapTabSh;
        QMap<int, QMap<int, QTabWidget*>> m_mapTabBe;
        QMap<int, QMap<int, QMap<int, QTabWidget*>>> m_mapTabFe;
        QMap<int, QMap<int, QMap<int, QMap<int, QTabWidget*>>>> m_mapTabCbc;
        QMap<int, QMap<int, QMap<int, QMap<int, QVector<QGridLayout*>>>>> m_mapTabPage;
        QMap<int, QMap<int, QMap<int, QMap<int, QMap<QString, QLineEdit*>>>>> m_mapWidgets;

        QTabWidget *createCbcTab();
        void createCbcRegItems();
        void clearTabs();
    };
}

