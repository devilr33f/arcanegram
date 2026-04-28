#include "ag_config.h"

#include "core/application.h"
#include "core/core_settings.h"

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
    _changes.fire(std::move(v));
}

template <typename T>
rpl::producer<T> Item<T>::changes() const {
    return _changes.events();
}

template class Item<bool>;
template class Item<QString>;

} // namespace Arcanegram::Config
