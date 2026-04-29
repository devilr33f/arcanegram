#pragma once

#include <QtCore/QJsonValue>
#include <QtCore/QString>
#include <memory>
#include <rpl/producer.h>

namespace Main { class Session; }

namespace Arcanegram::Sync {

enum class Status { Disabled, Idle, Syncing, Offline, Conflict };

struct StatusInfo {
    Status status = Status::Disabled;
    int pendingCount = 0;
    qint64 lastSyncedAt = 0;
    QString message;
};

class Engine {
public:
    Engine();
    ~Engine();

    static Engine *Instance();

    void setSession(Main::Session *session);

    void enqueue(const QString &key, const QJsonValue &value);

    void forcePull();
    void uploadLocalToCloud();
    void resetCloud();

    [[nodiscard]] rpl::producer<StatusInfo> status() const;

private:
    struct Private;
    std::unique_ptr<Private> _p;
};

void Init();

} // namespace Arcanegram::Sync
