#pragma once

#include "base/basic_types.h"

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Ui {
class VerticalLayout;
} // namespace Ui

namespace Arcanegram::QuickAccess {

void Setup(::Settings::Builder::SectionBuilder &builder);
void SetupMainMenu(not_null<Ui::VerticalLayout*> menu);

} // namespace Arcanegram::QuickAccess
