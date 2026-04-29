// ag_sync_initdata.cpp
#include "arcanegram/sync/ag_sync_initdata.h"

#include "arcanegram/ag_config.h"
#include "apiwrap.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "main/main_session.h"
#include "mtproto/sender.h"

#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

namespace Arcanegram::Sync {
namespace {

constexpr auto kStaleAfterSeconds = 60 * 60; // 1h

QString ParseFragment(const QString &url) {
    const auto hash = url.indexOf('#');
    if (hash < 0) return {};
    const auto fragment = url.mid(hash + 1);
    QUrlQuery query(fragment);
    return QUrl::fromPercentEncoding(
        query.queryItemValue(u"tgWebAppData"_q, QUrl::FullyEncoded).toUtf8());
}

} // namespace

struct InitDataFetcher::Private {
    Main::Session *session = nullptr;
    std::unique_ptr<MTP::Sender> api;
    InitData cached;
    bool inflight = false;
    std::vector<std::pair<Callback, ErrorCallback>> waiters;

    void resetSender() {
        if (session) {
            api = std::make_unique<MTP::Sender>(&session->api().instance());
        } else {
            api.reset();
        }
    }

    void resolveAndOpen(const QString &username) {
        api->request(MTPcontacts_ResolveUsername(
            MTP_flags(0),
            MTP_string(username),
            MTP_string()
        )).done([this](const MTPcontacts_ResolvedPeer &result) {
            result.match([&](const MTPDcontacts_resolvedPeer &data) {
                session->data().processUsers(data.vusers());
                session->data().processChats(data.vchats());
                const auto peerId = peerFromMTP(data.vpeer());
                if (!peerId) {
                    failAll(u"bot-not-found"_q);
                    return;
                }
                const auto peer = session->data().peer(peerId);
                const auto user = peer ? peer->asUser() : nullptr;
                if (!user) {
                    failAll(u"bot-not-user"_q);
                    return;
                }
                openWebView(user->inputUser());
            });
        }).fail([this](const MTP::Error &err) {
            failAll(u"resolve-failed: %1"_q.arg(err.type()));
        }).send();
    }

    void openWebView(const MTPInputUser &botInput) {
        using Flag = MTPmessages_RequestSimpleWebView::Flag;
        api->request(MTPmessages_RequestSimpleWebView(
            MTP_flags(Flag::f_url),
            botInput,
            MTP_bytes((Config::Sync::Endpoint.value() + u"/miniapp"_q).toUtf8()),
            MTPstring(),
            MTPDataJSON(),
            MTP_string("tdesktop")
        )).done([this](const MTPWebViewResult &result) {
            const auto &data = result.data();
            const auto raw = ParseFragment(qs(data.vurl()));
            if (raw.isEmpty()) {
                failAll(u"empty-init-data"_q);
                return;
            }
            cached.raw = raw;
            cached.authDate = QDateTime::currentSecsSinceEpoch();
            inflight = false;
            auto pending = std::exchange(waiters, {});
            for (auto &[ok, _] : pending) ok(cached);
        }).fail([this](const MTP::Error &err) {
            failAll(u"webview-failed: %1"_q.arg(err.type()));
        }).send();
    }

    void failAll(const QString &msg) {
        inflight = false;
        auto pending = std::exchange(waiters, {});
        for (auto &[_, fail] : pending) if (fail) fail(msg);
    }
};

InitDataFetcher::InitDataFetcher() : _p(std::make_unique<Private>()) {}
InitDataFetcher::~InitDataFetcher() = default;

void InitDataFetcher::setSession(Main::Session *session) {
    _p->session = session;
    _p->resetSender();
    _p->cached = {};
}

void InitDataFetcher::invalidate() { _p->cached = {}; }

void InitDataFetcher::request(Callback ok, ErrorCallback fail) {
    if (!_p->api) {
        if (fail) fail(u"no-session"_q);
        return;
    }
    const auto now = QDateTime::currentSecsSinceEpoch();
    if (!_p->cached.raw.isEmpty()
        && (now - _p->cached.authDate) < kStaleAfterSeconds) {
        ok(_p->cached);
        return;
    }
    _p->waiters.emplace_back(std::move(ok), std::move(fail));
    if (_p->inflight) return;
    _p->inflight = true;
    _p->resolveAndOpen(Config::Sync::Bot.value());
}

} // namespace Arcanegram::Sync
