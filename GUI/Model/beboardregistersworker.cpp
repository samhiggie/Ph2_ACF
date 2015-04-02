#pragma once
#include "beboardregistersworker.h"
#include <QDebug>
#include <QThread>

namespace GUI{
    BeBoardRegistersWorker::BeBoardRegistersWorker(QObject *parent,
                                                   SystemController &sysCtrl) :
        QObject(parent),
        m_systemController(sysCtrl)
    {

    }

    BeBoardRegistersWorker::~BeBoardRegistersWorker()
    {
        qDebug() << "Destructing BeRegister Worker" << this;
    }

    void BeBoardRegistersWorker::requestWork()
    {
        qDebug()<<"Request worker start in Thread "<<thread()->currentThreadId();
        emit workRequested();
    }

    void BeBoardRegistersWorker::getObjects()
    {
        fBeBoardInterface = m_systemController.getBeBoardInterface();
        fCbcInterface = m_systemController.getCbcInterface();
        fShelveVector = m_systemController.getfShelveVector();
        fBeBoardFWMap = m_systemController.getBeBoardFWMap();
    }

    void BeBoardRegistersWorker::getInitialBeBoardRegMap()
    {
        getObjects();
        for ( auto cShelve : fShelveVector )
        {
            for ( auto cBoard : ( cShelve )->fBoardVector )
            {
                emit sendInitialBeBoardRegValues(cShelve->getShelveId(),
                                                 cBoard->getBeId(),
                                                 cBoard->getBeBoardRegMap());
            }
        }

        emit finishedInitialiseBeBoardRegValues();


    }

}

