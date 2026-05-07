#include "arcanegram/features/streamer_mode/platform/ag_streamer_mode_impl.h"

// x11 / wayland have no portable api to exclude a window from screen capture.

namespace Arcanegram::StreamerMode::Impl {

void EnableHook() {
}

void DisableHook() {
}

void HideWidget(QWidget *) {
}

void ShowWidget(QWidget *) {
}

} // namespace Arcanegram::StreamerMode::Impl
