#pragma once
#include <QWidget>

namespace Ui {
    class CmTestTab;
}

namespace GUI {


    class CmTestTab : public QWidget
    {
        Q_OBJECT

    public:
        explicit CmTestTab(QWidget *parent);
        ~CmTestTab();
    signals:
        void globalEnable(bool enable);
        void startCmTest();
        void sendIsScan(const bool);

    public slots:
        void enable(bool enable);
        void getIsNoiseScan();

    private slots:
        void on_btnLaunch_clicked();

    private:
        Ui::CmTestTab *ui;
    };
}

