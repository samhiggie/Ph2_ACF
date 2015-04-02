#include "settings.h"
#include <QString>
#include "../../Utils/picojson.h"
#include <QStringListModel>
#include <QDebug>
#include <QDir>
#include <QVariantMap>
#include <QMessageBox>
#include <map>
#include <QList>
#include <fstream>

namespace GUI
{
    Settings::Settings(QObject *parent) :
        QObject(parent)
    {
    }

    Settings::~Settings()
    {
        qDebug() << "Destructing " << this;
    }

    void Settings::ParseJsondata()
    {
        QString raw_json = ReadJsonFile();
        if (raw_json.size()==0)
        {
            SendStatusMessage(tr("Json file empty or not formatted correctly"));
            return;
        }

        m_jsonValue = GetJsonObject(raw_json);
    }

    void Settings::ParseCustomJsonData()
    {
        QString raw_json = ReadJsonFile();

        if (raw_json.size()==0)
        {
            SendStatusMessage(tr("Custom Json file not formatted correctly"));
            return;
        }
    }

    void Settings::onLoadButtonClicked(bool cbc2)
    {
        if (cbc2)
        {
            m_filename = QString("settings/HWDescription_2CBC.json").toLatin1();
            ParseJsondata();
            emit setHwTree(getHwStandardItems()); //TODO - add debug
            SendStatusMessage(tr("Settings for 2CBC2 loaded"));
        }
        else
        {
            m_filename= QString("settings/HWDescription_8CBC.json").toLatin1();
            ParseJsondata();
            emit setHwTree(getHwStandardItems());
            SendStatusMessage(tr("Settings for 8CBC2 loaded"));
        }
    }

    void Settings::onCustomLoadButtonClicked(QString cfileName)
    {
        m_filename = cfileName.toLatin1();
        ParseJsondata();
        emit setHwTree(getHwStandardItems()); //TODO - add debug
        SendStatusMessage(tr("Custom settings loaded from :"));
        SendStatusMessage(m_filename);
    }

    QString Settings::ReadJsonFile()
    {
        auto default_settings = ReadJsonFromInternalResource();
        return default_settings;
    }

    picojson::value Settings::GetJsonObject(const QString& rawJson)
    {
        picojson::value  cJsonValue;

        std::string cRawValue = rawJson.toUtf8().constData();

        std::string cErr = picojson::parse( cJsonValue, cRawValue );
        if ( !cErr.empty() ) SendStatusMessage(tr("Could not parse JSON file ") + (QString::fromStdString(cErr)) );

        return cJsonValue;
    }

    QStandardItemModel* Settings::CreateItemModel()
    {

        QStandardItemModel* standardModel = new QStandardItemModel;

        standardModel->clear();

        double cBeId, cModuleId, cCbcId;
        double cNShelve = 0;
        int i, j;

        picojson::array cShelves = m_jsonValue.get( "HwDescription" ).get( "Shelves" ).get<picojson::array>();

        for ( const auto& cShelve : cShelves )
        {
            double cShelveId = cShelve.get( "Id" ).get<double>() ;

            emit sendSh(cShelveId);

            QList<QStandardItem *> preparedRow = prepareRow("Shelve Id " + QString::number(cShelveId));
            QStandardItem *item = standardModel->invisibleRootItem();
            item->appendRow(preparedRow);

            picojson::array cBeBoards = cShelve.get( "BeBoards" ).get<picojson::array>();

            for ( const auto& cBoard : cBeBoards )
            {
                cBeId =  cBoard.get( "Id" ).get<double>() ;
                QList<QStandardItem *> secondRow = prepareRow("Be Board Id " + QString::number(cBeId));
                preparedRow.first()->appendRow(secondRow);

                QList<QStandardItem *> thirdRow_2 = prepareRow("Board Type: " + QString::fromStdString(cBoard.get( "boardType" ).get<std::string>()));
                secondRow.first()->appendRow(thirdRow_2);

                emit sendBe(cShelveId,cBeId);

                QList<QStandardItem *> thirdRow = prepareRow("Registers");
                secondRow.first()->appendRow(thirdRow);

                for ( const auto& cRegister :  cBoard.get( "RegisterName" ).get<picojson::object>( ) )
                {
                    QList<QStandardItem *> fourthRow = prepareRow(QString::fromStdString(cRegister.first) + " : " + QString::number(cRegister.second.get<double>()));
                    thirdRow.first()->appendRow(fourthRow);
                }

                picojson::array cModules = cBoard.get( "Modules" ).get<picojson::array>();

                for ( const auto& cModuleNode : cModules )
                {
                    cModuleId = ( cModuleNode.get( "ModuleId" ).get<double>() );
                    uint32_t cFMCId = ( cModuleNode.get( "FMCId" ).get<double>() );
                    uint32_t cFeId = ( cModuleNode.get( "FeId" ).get<double>() );

                    emit sendFe(cShelveId, cBeId, cFeId);

                    QList<QStandardItem *> thirdRow = prepareRow("Module Id: " + QString::number(cModuleId));
                    QList<QStandardItem *> thirdRow_2 = prepareRow("Fe Id: " + QString::number(cFeId));
                    QList<QStandardItem *> thirdRow_3 = prepareRow("FMC Id: " + QString::number(cFMCId));

                    secondRow.first()->appendRow(thirdRow);
                    secondRow.first()->appendRow(thirdRow_2);
                    secondRow.first()->appendRow(thirdRow_3);

                    bool cStatus = cModuleNode.get( "Status" ).evaluate_as_boolean();

                    if ( cStatus )
                    {

                        std::string cFilePrefix;
                        if ( !cModuleNode.get( "CbcFilePath" ).is<picojson::null>() ) cFilePrefix = cModuleNode.get( "CbcFilePath" ).get<std::string>();
                        picojson::array cCbcs = cModuleNode.get( "CBCs" ).get<picojson::array>();

                        for ( const auto& cCbcNode : cCbcs )
                        {
                            cCbcId = ( cCbcNode.get( "Id" ).get<double>() );
                            std::string cFileName;
                            if ( !cFilePrefix.empty() )
                                cFileName = cFilePrefix + cCbcNode.get( "configfile" ).get<std::string>();
                            else cFileName = cCbcNode.get( "configfile" ).get<std::string>();

                            QList<QStandardItem *> cbcRow = prepareRow("Cbc " + QString::number(cCbcId) + " : Files Path: " + QString::fromStdString(cFileName));
                            secondRow.first()->appendRow(cbcRow);

                            emit sendCbc(cShelveId, cBeId, cFeId, cCbcId);

                            if ( !cCbcNode.get( "Register" ).is<picojson::null>() )
                            {
                                for ( const auto& cRegister :  cCbcNode.get( "Register" ).get<picojson::object>( ) )
                                {
                                    QList<QStandardItem *> cbcRegRow = prepareRow(QString::fromStdString(cRegister.first) + " : " + QString::fromStdString(cRegister.second.get<std::string>().c_str()));
                                    cbcRow.first()->appendRow(cbcRegRow);
                                }
                            }

                            if ( !cModuleNode.get( "Global_CBC_Registers" ).is<picojson::null>() )
                            {
                                // iterate the registers
                                for ( const auto& cRegister :  cModuleNode.get( "Global_CBC_Registers" ).get<picojson::object>( ) )
                                {
                                    QList<QStandardItem *> cbcGlobalRegRow = prepareRow(QString::fromStdString(cRegister.first) + " : " + QString::fromStdString(cRegister.second.get<std::string>().c_str()));
                                    cbcRow.first()->appendRow(cbcGlobalRegRow);
                                }
                            }
                        }
                    }
                }
            }
            cNShelve++;
        }
        return standardModel;
    }

    QList<QStandardItem *> Settings::prepareRow(const QString &first)
    {
        QList<QStandardItem *> rowItems;
        rowItems << new QStandardItem(first);
        return rowItems;
    }

    QString Settings::ReadJsonFromInternalResource()
    {
        QDir res_dir;

        if (!res_dir.exists())
        {
            SendStatusMessage(tr("Internal resource path missing ") + res_dir.canonicalPath());
            return "";
        }

        auto path = res_dir.filePath(m_filename);
        QFile res_file(path);
        if(!res_file.open(QFile::ReadOnly|QFile::Text))
        {
            SendStatusMessage(tr("Could not open internal resource ") );//+ path);
            return ""; //flush
        }

        QString settings = res_file.readAll();
        return settings;
    }

    void Settings::SendStatusMessage(QString msg)
    {
        emit notifyStatusMessage(msg);
    }
}
