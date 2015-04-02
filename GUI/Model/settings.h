#pragma once
#include <QObject>
#include <QVariantMap>
#include "../../Utils/picojson.h"
#include <QStandardItem>

class QString;
class QStringListModel;
class QFile;

namespace GUI{

    class Settings : public QObject
    {
        Q_OBJECT
    public:
        explicit Settings(QObject *parent);
        void ParseJsondata();
        void ParseCustomJsonData();

        QString getHwFileName() {return m_filename;}
        ~Settings();

    signals:
        void sendSh(const int idSh);
        void sendBe(const int idSh, const int idBe);
        void sendFe(const int idSh, const int idBe, const int idFe);
        void sendCbc(const int idSh, const int idBe, const int idFe, const int idCbc);
        void notifyStatusMessage(QString msg);
        void setHwTree(QStandardItemModel * hwItems);

    public slots:
        void onLoadButtonClicked(bool cbc2);
        void onCustomLoadButtonClicked(QString cfileName);

    private:
        QString m_filename;

        QStandardItemModel *CreateItemModel();
        QStandardItemModel *getHwStandardItems() {return CreateItemModel();}
        picojson::value m_jsonValue;

        QList <QString> list_ShelveId;
        QList <QString> list_BeId;

        QStandardItemModel* m_HwStandardItems;
        QList<QStandardItem *> prepareRow(const QString &first);

        QString ReadJsonFromInternalResource();
        QString ReadJsonFile();

        void SendStatusMessage(QString msg);

        picojson::value GetJsonObject(const QString &rawJson);

        explicit Settings(const Settings& rhs) = delete;
        Settings& operator= (const Settings& rhs) = delete;
    };
}



