#pragma once

#include <QtCore/QString>
#include <rpl/producer.h>

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Arcanegram {
namespace Config {
class BoolItem;
template <typename T> class Item;
} // namespace Config

namespace Settings {

void AddBoolRow(
    ::Settings::Builder::SectionBuilder &builder,
    const QString &id,
    rpl::producer<QString> title,
    rpl::producer<QString> info,
    Config::BoolItem &item,
    bool needsRestart = false);

void AddTextRow(
    ::Settings::Builder::SectionBuilder &builder,
    rpl::producer<QString> placeholder,
    Config::Item<QString> &item);

void AddStatusRow(
    ::Settings::Builder::SectionBuilder &builder,
    rpl::producer<QString> text);

} // namespace Settings
} // namespace Arcanegram
