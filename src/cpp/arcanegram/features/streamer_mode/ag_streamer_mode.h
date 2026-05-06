#pragma once

namespace Settings::Builder { class SectionBuilder; }

namespace Arcanegram::StreamerMode {

void Init();
void Setup(::Settings::Builder::SectionBuilder &builder);

} // namespace Arcanegram::StreamerMode
