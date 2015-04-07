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
        void writeBeRegisters(const int idSh, const int idBe, QMap<QString, int>);
        void globalEnable(bool enable);

    public slots:
        void reset();
        void enable(bool enable);
        void setupSh(const int idSh);
        void setupBe(const int idSh, const int idBe);

        void createBeBoardRegisterValues(const int idSh, const int idBe, const std::map< std::string, uint32_t > mapReg);
        void refreshBeBoardRegisterValues(const int idSh, const int idBe, const std::map< std::string, uint32_t > mapReg);

    private slots:
        void on_btnRefresh_clicked();

        void on_btnWrite_clicked();

    private:

        QTabWidget *m_tabSh;

        QMap<int, QTabWidget*> m_mapTabSh;
        QMap<int, QMap<int, QTabWidget*>> m_mapTabBe;

        QMap<int, QMap<int, QGridLayout*>> m_mapBeGrid;
        QMap<int, QMap<int, QMap<QString, QLineEdit*>>> m_mapWidgets;
        void createBeRegItems();
        void tabsClear();
        Ui::BeBoardRegistersTab *ui;
    };
}
