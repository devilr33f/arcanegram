#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <functional>
#include <memory>

class QNetworkAccessManager;

namespace Arcanegram::Sync {

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    struct Response {
        int status = 0;
        QJsonObject body;     // parsed JSON, empty on parse failure
        QString error;        // non-empty on transport/json error
    };
    using Callback = std::function<void(Response)>;

    void get(const QString &url, const QString &initData, Callback cb);
    void put(const QString &url, const QString &initData,
             const QJsonObject &body, Callback cb);

private:
    std::unique_ptr<QNetworkAccessManager> _nam;
};

} // namespace Arcanegram::Sync
