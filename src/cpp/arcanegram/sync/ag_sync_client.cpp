#include "arcanegram/sync/ag_sync_client.h"

#include <QtCore/QJsonDocument>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Arcanegram::Sync {
namespace {

HttpClient::Response BuildResponse(QNetworkReply *reply) {
    HttpClient::Response r;
    r.status = reply->attribute(
        QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError && r.status == 0) {
        r.error = reply->errorString();
        return r;
    }
    const auto bytes = reply->readAll();
    QJsonParseError pe{};
    const auto doc = QJsonDocument::fromJson(bytes, &pe);
    if (pe.error == QJsonParseError::NoError && doc.isObject()) {
        r.body = doc.object();
    } else if (r.status >= 200 && r.status < 300) {
        r.error = u"bad-json"_q;
    }
    return r;
}

QNetworkRequest BuildRequest(const QString &url, const QString &initData) {
    QNetworkRequest req{QUrl(url)};
    req.setRawHeader("Authorization",
        ("tma " + initData).toUtf8());
    req.setHeader(QNetworkRequest::ContentTypeHeader,
        QStringLiteral("application/json"));
    return req;
}

} // namespace

HttpClient::HttpClient()
: _nam(std::make_unique<QNetworkAccessManager>()) {}

HttpClient::~HttpClient() = default;

void HttpClient::get(const QString &url, const QString &initData, Callback cb) {
    auto *reply = _nam->get(BuildRequest(url, initData));
    QObject::connect(reply, &QNetworkReply::finished, [reply, cb = std::move(cb)] {
        cb(BuildResponse(reply));
        reply->deleteLater();
    });
}

void HttpClient::put(const QString &url, const QString &initData,
                     const QJsonObject &body, Callback cb) {
    const auto bytes = QJsonDocument(body).toJson(QJsonDocument::Compact);
    auto *reply = _nam->put(BuildRequest(url, initData), bytes);
    QObject::connect(reply, &QNetworkReply::finished, [reply, cb = std::move(cb)] {
        cb(BuildResponse(reply));
        reply->deleteLater();
    });
}

} // namespace Arcanegram::Sync
