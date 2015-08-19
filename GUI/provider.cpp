#include "provider.h"
#include <QString>
#include "Model/settings.h"
#include "utils.h"

#include <QDebug>

namespace GUI
{
    //static QString SETTINGS_FILE=QString("settings/HwDescription.json").toLatin1();
    std::unique_ptr<Settings> Provider::m_instanceSettings = nullptr;

    Settings &Provider::getSettingsAsSingleton()
    {
        if (m_instanceSettings == nullptr)
        {
            m_instanceSettings = Utils::make_unique<Settings>(nullptr);
        }
        return *m_instanceSettings;
    }
}
