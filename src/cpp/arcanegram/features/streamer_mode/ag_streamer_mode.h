#pragma once

class QWidget;

namespace Settings::Builder { class SectionBuilder; }

namespace Arcanegram::StreamerMode {

void Init();
void Setup(::Settings::Builder::SectionBuilder &builder);

[[nodiscard]] bool IsActive();
void ApplyTo(QWidget *widget);

} // namespace Arcanegram::StreamerMode
