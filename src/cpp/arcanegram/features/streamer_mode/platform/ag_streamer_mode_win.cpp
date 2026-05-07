#include "arcanegram/features/streamer_mode/platform/ag_streamer_mode_impl.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <windows.h>

namespace Arcanegram::StreamerMode::Impl {
namespace {

void ApplyAllTopLevels(DWORD affinity) {
	for (auto *widget : QApplication::topLevelWidgets()) {
		if (!widget || !widget->isWindow() || !widget->testAttribute(Qt::WA_WState_Created)) {
			continue;
		}
		const auto handle = reinterpret_cast<HWND>(widget->winId());
		if (handle) {
			SetWindowDisplayAffinity(handle, affinity);
		}
	}
}

} // namespace

void EnableHook() {
	ApplyAllTopLevels(WDA_EXCLUDEFROMCAPTURE);
}

void DisableHook() {
	ApplyAllTopLevels(WDA_NONE);
}

void HideWidget(QWidget *widget) {
	if (!widget) {
		return;
	}
	const auto handle = reinterpret_cast<HWND>(widget->window()->winId());
	SetWindowDisplayAffinity(handle, WDA_EXCLUDEFROMCAPTURE);
}

void ShowWidget(QWidget *widget) {
	if (!widget) {
		return;
	}
	const auto handle = reinterpret_cast<HWND>(widget->window()->winId());
	SetWindowDisplayAffinity(handle, WDA_NONE);
}

} // namespace Arcanegram::StreamerMode::Impl
