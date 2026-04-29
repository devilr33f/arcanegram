#include "arcanegram/sync/ag_sync_engine.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/sync/ag_sync_client.h"
#include "arcanegram/sync/ag_sync_initdata.h"
#include "arcanegram/sync/ag_sync_keys.h"
#include "arcanegram/sync/ag_sync_state.h"
#include "core/application.h"
#include "main/main_domain.h"
#include "main/main_session.h"

#include <QtCore/QDateTime>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <rpl/event_stream.h>
#include <rpl/lifetime.h>

namespace Arcanegram::Sync {
namespace {

std::unique_ptr<Engine> g_engine;

rpl::lifetime &SessionLifetime() {
    static rpl::lifetime value;
    return value;
}

constexpr auto kRetryBackoffMaxMs = 5 * 60 * 1000;
constexpr auto kRetryBackoffStartMs = 1000;

QString StateFilePath() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + u"/tdata/arcanegram_sync.bin"_q;
}

} // namespace

struct Engine::Private {
    InitDataFetcher initData;
    HttpClient http;
    StateStore store{StateFilePath()};
    State state;
    std::deque<Patch> inflight;
    StatusInfo status;
    rpl::event_stream<StatusInfo> statusStream;
    QTimer flushTimer;
    int retryDelayMs = kRetryBackoffStartMs;
    bool flushing = false;
    bool needsPull = false;
    bool sessionAttached = false;

    Private() {
        state = store.load();
        flushTimer.setSingleShot(true);
        QObject::connect(&flushTimer, &QTimer::timeout, [this] { flush(); });
        status.pendingCount = int(state.outbox.size());
    }

    void persist() { store.save(state); }
    void emitStatus() { statusStream.fire_copy(status); }

    void scheduleFlush(int delayMs) {
        if (flushTimer.isActive()) return;
        flushTimer.start(delayMs);
    }

    void flush() {
        if (flushing) return;
        if (!sessionAttached) return;
        if (!Config::Sync::Enabled.value()) return;

        flushing = true;
        initData.request(
            [this](InitData id) { onInitDataReady(id.raw); },
            [this](QString err) { onTransientFail(err); });
    }

    void onInitDataReady(const QString &raw) {
        if (needsPull || state.outbox.empty()) {
            needsPull = false;
            doPull(raw);
        } else {
            doPut(raw);
        }
    }

    void doPull(const QString &raw) {
        status.status = Status::Syncing;
        emitStatus();
        const auto url = Config::Sync::Endpoint.value() + u"/sync"_q;
        http.get(url, raw, [this](HttpClient::Response r) { onPullResult(r); });
    }

    void onPullResult(const HttpClient::Response &r) {
        flushing = false;
        if (r.status == 200) {
            const auto cloudVersion = r.body.value(u"version"_q).toInt();
            const auto cloudData = r.body.value(u"data"_q).toObject();
            applyCloud(cloudVersion, cloudData);
            retryDelayMs = kRetryBackoffStartMs;
            if (!state.outbox.empty()) scheduleFlush(50);
        } else if (r.status == 401) {
            initData.invalidate();
            scheduleRetry(/*reset=*/true, 100);
        } else {
            offlineWith(r);
        }
    }

    void doPut(const QString &raw) {
        // snapshot all current outbox entries into inflight; reset outbox so
        // new enqueues during the network call don't collapse onto in-flight
        // values (and so we can put them back if the request fails).
        inflight = std::move(state.outbox);
        state.outbox.clear();
        persist();

        status.status = Status::Syncing;
        emitStatus();

        QJsonObject patch;
        for (const auto &p : inflight) patch.insert(p.key, p.value);
        QJsonObject body{
            {u"baseVersion"_q, state.version},
            {u"patch"_q, patch},
        };
        const auto url = Config::Sync::Endpoint.value() + u"/sync"_q;
        http.put(url, raw, body, [this](HttpClient::Response r) { onPutResult(r); });
    }

    void onPutResult(const HttpClient::Response &r) {
        flushing = false;
        if (r.status == 200) {
            const auto cloudVersion = r.body.value(u"version"_q).toInt();
            const auto cloudData = r.body.value(u"data"_q).toObject();
            inflight.clear();
            applyCloud(cloudVersion, cloudData);
            retryDelayMs = kRetryBackoffStartMs;
            if (!state.outbox.empty()) scheduleFlush(50);
        } else if (r.status == 409) {
            // server has newer version. put inflight back and force a pull.
            requeueInflight();
            needsPull = true;
            status.status = Status::Conflict;
            emitStatus();
            scheduleFlush(50);
        } else if (r.status == 401) {
            requeueInflight();
            initData.invalidate();
            scheduleRetry(/*reset=*/true, 100);
        } else if (r.status == 413) {
            // payload too large; drop the offending batch (head of inflight).
            // pragmatic v1 behaviour — surface in status message.
            inflight.clear();
            persist();
            status.status = Status::Offline;
            status.message = u"payload too large"_q;
            status.pendingCount = int(state.outbox.size());
            emitStatus();
        } else {
            requeueInflight();
            offlineWith(r);
        }
    }

    void applyCloud(int version, const QJsonObject &cloudData) {
        for (auto it = cloudData.constBegin(); it != cloudData.constEnd(); ++it) {
            ApplyToLocal(it.key(), it.value());
        }
        state.version = version;
        state.data = cloudData;
        persist();
        status.status = Status::Idle;
        status.pendingCount = int(state.outbox.size());
        status.lastSyncedAt = QDateTime::currentSecsSinceEpoch();
        status.message.clear();
        emitStatus();
    }

    void requeueInflight() {
        for (auto it = inflight.rbegin(); it != inflight.rend(); ++it) {
            state.outbox.push_front(*it);
        }
        inflight.clear();
        status.pendingCount = int(state.outbox.size());
        persist();
    }

    void offlineWith(const HttpClient::Response &r) {
        status.status = Status::Offline;
        status.message = r.error.isEmpty()
            ? u"http %1"_q.arg(r.status)
            : r.error;
        status.pendingCount = int(state.outbox.size());
        emitStatus();
        scheduleRetry(/*reset=*/false, retryDelayMs);
    }

    void onTransientFail(const QString &err) {
        flushing = false;
        status.status = Status::Offline;
        status.message = err;
        emitStatus();
        scheduleRetry(/*reset=*/false, retryDelayMs);
    }

    void scheduleRetry(bool reset, int initial) {
        if (reset) retryDelayMs = kRetryBackoffStartMs;
        scheduleFlush(initial);
        retryDelayMs = std::min(retryDelayMs * 2, kRetryBackoffMaxMs);
    }
};

Engine::Engine() : _p(std::make_unique<Private>()) {}
Engine::~Engine() = default;

Engine *Engine::Instance() { return g_engine.get(); }

void Engine::setSession(Main::Session *session) {
    _p->initData.setSession(session);
    _p->sessionAttached = (session != nullptr);
    if (session && Config::Sync::Enabled.value()) {
        _p->status.status = Status::Idle;
        _p->emitStatus();
        _p->scheduleFlush(0);
    } else if (!session) {
        _p->status.status = Status::Disabled;
        _p->emitStatus();
    }
}

void Engine::enqueue(const QString &key, const QJsonValue &value) {
    if (!Config::Sync::Enabled.value()) return;
    if (!_p->state.outbox.empty() && _p->state.outbox.back().key == key) {
        _p->state.outbox.back().value = value;
    } else {
        _p->state.outbox.push_back({key, value});
    }
    _p->status.pendingCount = int(_p->state.outbox.size());
    _p->persist();
    _p->emitStatus();
    _p->scheduleFlush(50);
}

rpl::producer<StatusInfo> Engine::status() const {
    return rpl::single(_p->status) | rpl::then(_p->statusStream.events());
}

void Engine::forcePull() {
    _p->needsPull = true;
    _p->scheduleFlush(0);
}

void Engine::uploadLocalToCloud() {
    const auto snapshot = SnapshotLocal();
    for (auto it = snapshot.constBegin(); it != snapshot.constEnd(); ++it) {
        if (!_p->state.outbox.empty() && _p->state.outbox.back().key == it.key()) {
            _p->state.outbox.back().value = it.value();
        } else {
            _p->state.outbox.push_back({it.key(), it.value()});
        }
    }
    _p->status.pendingCount = int(_p->state.outbox.size());
    _p->persist();
    _p->emitStatus();
    _p->scheduleFlush(0);
}

void Engine::resetCloud() {
    QJsonObject deletes;
    for (const auto &key : _p->state.data.keys()) {
        deletes.insert(key, QJsonValue::Null);
    }
    for (auto it = deletes.constBegin(); it != deletes.constEnd(); ++it) {
        _p->state.outbox.push_back({it.key(), it.value()});
    }
    _p->status.pendingCount = int(_p->state.outbox.size());
    _p->persist();
    _p->emitStatus();
    _p->scheduleFlush(0);
}

void Init() {
    g_engine = std::make_unique<Engine>();
    Core::App().domain().activeSessionValue(
    ) | rpl::start_with_next([](Main::Session *session) {
        if (g_engine) g_engine->setSession(session);
    }, SessionLifetime());
}

} // namespace Arcanegram::Sync
