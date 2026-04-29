// ag_sync_state.cpp
#include "arcanegram/sync/ag_sync_state.h"

#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QSaveFile>

namespace Arcanegram::Sync {

StateStore::StateStore(QString filePath) : _path(std::move(filePath)) {}

State StateStore::load() {
    QFile f(_path);
    if (!f.open(QIODevice::ReadOnly)) return {};
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isObject()) return {};
    const auto root = doc.object();
    State s;
    s.version = root.value(u"version"_q).toInt(0);
    s.data = root.value(u"data"_q).toObject();
    for (const auto &v : root.value(u"outbox"_q).toArray()) {
        const auto entry = v.toObject();
        s.outbox.push_back({
            entry.value(u"key"_q).toString(),
            entry.value(u"value"_q),
        });
    }
    return s;
}

void StateStore::save(const State &state) {
    QJsonArray outbox;
    for (const auto &p : state.outbox) {
        outbox.append(QJsonObject{{u"key"_q, p.key}, {u"value"_q, p.value}});
    }
    const auto root = QJsonObject{
        {u"version"_q, state.version},
        {u"data"_q, state.data},
        {u"outbox"_q, outbox},
    };
    QSaveFile f(_path);
    if (!f.open(QIODevice::WriteOnly)) return;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    f.commit();
}

} // namespace Arcanegram::Sync
