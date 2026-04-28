#pragma once

#include <rpl/producer.h>

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Arcanegram::ChatWallpaper {

[[nodiscard]] bool Disabled();
[[nodiscard]] rpl::producer<bool> DisabledValue();

void Setup(::Settings::Builder::SectionBuilder &builder);
void Init();

} // namespace Arcanegram::ChatWallpaper
