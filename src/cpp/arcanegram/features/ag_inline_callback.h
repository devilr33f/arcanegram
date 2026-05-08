#pragma once

#include <QtCore/QString>
#include <optional>

struct HistoryMessageMarkupButton;

namespace Settings::Builder {
class SectionBuilder;
} // namespace Settings::Builder

namespace Arcanegram::InlineCallback {

[[nodiscard]] std::optional<QString> Tooltip(
    const HistoryMessageMarkupButton *button,
    const QString &fallbackText);
[[nodiscard]] QString CopyText(const HistoryMessageMarkupButton *button);
[[nodiscard]] QString CopyContextItemText(
    const HistoryMessageMarkupButton *button);
void Setup(::Settings::Builder::SectionBuilder &builder);

} // namespace Arcanegram::InlineCallback
