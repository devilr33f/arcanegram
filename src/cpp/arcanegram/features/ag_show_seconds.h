#pragma once

#include <QtCore/QString>
#include <QtCore/QTime>

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Arcanegram::Time {

[[nodiscard]] QString FormatShort(QTime time);
void Setup(::Settings::Builder::SectionBuilder &builder);
void Init();

} // namespace Arcanegram::Time
