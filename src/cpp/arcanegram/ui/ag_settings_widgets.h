#pragma once

#include <QtCore/QString>
#include <rpl/producer.h>

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Arcanegram {
namespace Config {
class BoolItem;
} // namespace Config

namespace Settings {

void AddBoolRow(
    ::Settings::Builder::SectionBuilder &builder,
    const QString &id,
    rpl::producer<QString> title,
    rpl::producer<QString> info,
    Config::BoolItem &item,
    bool needsRestart = false);

} // namespace Settings
} // namespace Arcanegram
