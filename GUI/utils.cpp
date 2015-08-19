#include "utils.h"
#include <QString>
#include <QDebug>

namespace GUI
{
    static QString DESTRUCTOR_MSG = QString("Running the %1 destructor.").toLatin1();

    void Utils::DestructorMsg(QString value)
    {

        qDebug()<<DESTRUCTOR_MSG.arg(value);

    }

    void Utils::DestructorMsg(QObject* object)
    {

        DestructorMsg(object->metaObject()->className());

    }


}
