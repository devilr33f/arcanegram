#include "arcanegram/features/streamer_mode/platform/ag_streamer_mode_impl.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <Cocoa/Cocoa.h>

namespace Arcanegram::StreamerMode::Impl {
namespace {

NSWindow *NativeWindow(QWidget *widget) {
	if (!widget || !widget->testAttribute(Qt::WA_WState_Created)) {
		return nil;
	}
	const auto view = reinterpret_cast<NSView*>(widget->window()->winId());
	return view ? [view window] : nil;
}

void ApplyAllTopLevels(NSWindowSharingType sharing) {
	for (auto *widget : QApplication::topLevelWidgets()) {
		if (!widget || !widget->isWindow()) {
			continue;
		}
		if (auto window = NativeWindow(widget)) {
			window.sharingType = sharing;
		}
	}
}

} // namespace

void EnableHook() {
	ApplyAllTopLevels(NSWindowSharingNone);
}

void DisableHook() {
	ApplyAllTopLevels(NSWindowSharingReadOnly);
}

void HideWidget(QWidget *widget) {
	if (auto window = NativeWindow(widget)) {
		window.sharingType = NSWindowSharingNone;
	}
}

void ShowWidget(QWidget *widget) {
	if (auto window = NativeWindow(widget)) {
		window.sharingType = NSWindowSharingReadOnly;
	}
}

} // namespace Arcanegram::StreamerMode::Impl
