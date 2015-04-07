#pragma once

#include <QWidget>

namespace Ui {
    class HybridTestTab;
}

namespace GUI{

    class HybridTestTab : public QWidget
    {
        Q_OBJECT

    public:
        explicit HybridTestTab(QWidget *parent);

    ~HybridTestTab();
    signals:
        void globalEnable(bool enable);
        void startHybridTest();
        void sendVcthValue(int cVcth);
        void sendEventsNumber(int cEvents);
        void sendIsRegTestChecked(const bool);
        void sendIsScanChecked(const bool);
        void sendIsHoleModeChecked(const bool);

    public slots:
        void enable(bool enable);
        void getVcthDialValue();
        void getEventsDial();
        void getIsRegTestChecked();
        void getIsScanChecked();
        void getIsHoleModeChecked();

    private slots:
        void on_btnStart_clicked();

        void on_dialVcth_sliderMoved(int position);

        void on_txtVcth_editingFinished();

        void on_dialEvents_sliderMoved(int position);

        void on_txtEvents_editingFinished();

    private:

        Ui::HybridTestTab *ui;

        int m_Vcth;
        int m_Events;

    };

}
