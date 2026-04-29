// ag_sync_state.h
#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QString>
#include <deque>

namespace Arcanegram::Sync {

struct Patch {
    QString key;
    QJsonValue value; // QJsonValue::Null means delete
};

struct State {
    int version = 0;
    QJsonObject data;
    std::deque<Patch> outbox;
};

class StateStore {
public:
    explicit StateStore(QString filePath);

    [[nodiscard]] State load();
    void save(const State &state);

private:
    QString _path;
};

} // namespace Arcanegram::Sync
