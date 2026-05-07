#pragma once

class QWidget;

namespace Arcanegram::StreamerMode::Impl {

void EnableHook();
void DisableHook();
void HideWidget(QWidget *widget);
void ShowWidget(QWidget *widget);

} // namespace Arcanegram::StreamerMode::Impl
