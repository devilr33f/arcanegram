#include "arcanegram/features/streamer_mode/platform/ag_streamer_mode_impl.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <windows.h>

namespace Arcanegram::StreamerMode::Impl {
namespace {

void ApplyAllTopLevels(DWORD affinity) {
	for (auto *widget : QApplication::topLevelWidgets()) {
		if (!widget
			|| !widget->isWindow()
			|| !widget->isVisible()
			|| widget->testAttribute(Qt::WA_DontShowOnScreen)) {
			continue;
		}
		const auto handle = reinterpret_cast<HWND>(widget->internalWinId());
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

void ApplyToWidget(QWidget *widget, DWORD affinity) {
	if (!widget) {
		return;
	}
	const auto top = widget->window();
	if (!top || !top->testAttribute(Qt::WA_WState_Created)) {
		return;
	}
	const auto handle = reinterpret_cast<HWND>(top->internalWinId());
	if (handle) {
		SetWindowDisplayAffinity(handle, affinity);
	}
}

void HideWidget(QWidget *widget) {
	ApplyToWidget(widget, WDA_EXCLUDEFROMCAPTURE);
}

void ShowWidget(QWidget *widget) {
	ApplyToWidget(widget, WDA_NONE);
}

} // namespace Arcanegram::StreamerMode::Impl
