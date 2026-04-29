#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QString>

namespace Arcanegram::Sync {

bool ApplyToLocal(const QString &key, const QJsonValue &value);

QJsonObject SnapshotLocal();

bool IsApplyingFromCloud();

} // namespace Arcanegram::Sync
