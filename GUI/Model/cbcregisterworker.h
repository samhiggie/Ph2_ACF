#pragma once
#include <QObject>

#include "Model/systemcontroller.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

namespace GUI{
    class CbcRegisterWorker : public QObject
    {
        Q_OBJECT
    public:
        explicit CbcRegisterWorker(QObject *parent,
                                   SystemController &sysCtrl);
        void requestWork();
        void abort();

        ~CbcRegisterWorker();

    signals:
        void globalEnable(bool enable);
        void sendInitialCbcRegisterValues(const int idBe,
                                          const int idFe,
                                          const int idFmc,
                                          const int idCbc,
                                          const std::map<std::string, CbcRegItem> mapReg);
        void sendCbcRegisterValues(const int idBe,
                                  const int idFe,
                                  const int idFmc,
                                  const int idCbc,
                                  const std::map<std::string, CbcRegItem> mapReg);
        void workRequested();
        void finishedInitCbcReg();
        void finished();

    public slots:
        void writeCbcRegisters(const int idSh, const int idBe, const int idFe, const int idCbc,
                               std::vector<std::pair<std::string, std::uint8_t>> mapReg);
        void getInitialCbcRegistersMap();
        void getCbcRegistersMap();

    private:

        SystemController& m_systemController;

        BeBoardInterface*       fBeBoardInterface;
        CbcInterface*           fCbcInterface;
        ShelveVec fShelveVector;
        BeBoardFWMap fBeBoardFWMap;

        void getObjects();

        explicit CbcRegisterWorker(const CbcRegisterWorker& rhs) = delete;
        CbcRegisterWorker& operator= (const CbcRegisterWorker& rhs) = delete;
    };
}



