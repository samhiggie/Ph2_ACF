#pragma once

#include <QWidget>
#include <map>
#include <QTabWidget>
#include <QGridLayout>
#include <QLineEdit>
#include <QMap>

namespace Ui {
    class BeBoardRegistersTab;
}

namespace GUI {

    class BeBoardRegistersTab : public QWidget
    {
        Q_OBJECT

    public:
        explicit BeBoardRegistersTab(QWidget *parent);
        ~BeBoardRegistersTab();

    signals:
        void refreshBeValues();

    private slots:
        void reset();
        void setupSh(const int idSh);
        void setupBe(const int idSh, const int idBe);
        void createBeBoardRegisterValues(const int idSh, const int idBe, const std::map< std::string, uint32_t > mapReg);
        void on_btnRefresh_clicked();
        void onInitialiseBeReg(const std::map< std::string, uint32_t >  cMap);

    private:

        QTabWidget *m_tabSh;

        QMap<int, QTabWidget*> m_mapTabSh;
        QMap<int, QMap<int, QTabWidget*>> m_mapTabBe;

        QMap<int, QMap<int, QGridLayout*>> m_mapBeGrid;
        //QMap<int, QMap<int,

        void tabsClear();
        Ui::BeBoardRegistersTab *ui;
    };
}
