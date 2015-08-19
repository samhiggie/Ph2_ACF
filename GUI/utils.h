#pragma once
#include <QWidget>
#include <memory>

namespace GUI{
    class Utils
    {
    public:
        Utils();

        static void DestructorMsg(QString value);
        static void DestructorMsg(QObject *object);

        template<typename T, typename... Args> //implementing a C++14 feature
        static std::unique_ptr<T> make_unique(Args&&... args)
        {
            return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    private:
        explicit Utils(const Utils& rhs) = delete;
        Utils& operator= (const Utils& rhs) = delete;
    };
}



