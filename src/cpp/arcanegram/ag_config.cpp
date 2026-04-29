#include "ag_config.h"

#include "arcanegram/sync/ag_sync_engine.h"
#include "arcanegram/sync/ag_sync_keys.h"
#include "core/application.h"
#include "core/core_settings.h"

#include <QtCore/QJsonValue>
#include <QtCore/QVariant>

namespace Arcanegram::Config {

template <typename T>
Item<T>::Item(std::string_view key, T defaultValue)
: _key(key)
, _default(std::move(defaultValue)) {
}

template <typename T>
T Item<T>::value() const {
    return Core::App().settings().readPref<T>(_key, _default);
}

template <typename T>
void Item<T>::setValue(T v) {
    Core::App().settings().writePref<T>(_key, v);
    Core::App().saveSettingsDelayed();
    if (!::Arcanegram::Sync::IsApplyingFromCloud()) {
        if (auto *engine = ::Arcanegram::Sync::Engine::Instance()) {
            engine->enqueue(
                QString::fromUtf8(_key.data(), int(_key.size())),
                QJsonValue::fromVariant(QVariant::fromValue(v)));
        }
    }
    _changes.fire(std::move(v));
}

template <typename T>
rpl::producer<T> Item<T>::changes() const {
    return _changes.events();
}

template class Item<bool>;
template class Item<QString>;

} // namespace Arcanegram::Config
