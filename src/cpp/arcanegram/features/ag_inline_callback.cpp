#include "arcanegram/features/ag_inline_callback.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "history/history_item_reply_markup.h"
#include "lang/lang_keys.h"

namespace Arcanegram::InlineCallback {
namespace {

[[nodiscard]] bool IsCallback(const HistoryMessageMarkupButton *button) {
    using Type = HistoryMessageMarkupButton::Type;
    return button
        && (button->type == Type::Callback
            || button->type == Type::CallbackWithPassword
            || button->type == Type::Game);
}

[[nodiscard]] QString FormatPayload(const QByteArray &data) {
    if (data.isEmpty()) {
        return QString();
    }
    const auto asText = QString::fromUtf8(data);
    if (asText.toUtf8() == data) {
        return asText;
    }
    return QString::fromLatin1(data.toHex());
}

} // namespace

std::optional<QString> Tooltip(
        const HistoryMessageMarkupButton *button,
        const QString &fallbackText) {
    if (!Config::InlineCallback::Show.value() || !IsCallback(button)) {
        return std::nullopt;
    }
    const auto payload = FormatPayload(button->data);
    if (payload.isEmpty()) {
        return std::nullopt;
    }
    const auto label = tr::ag_inline_callback_label(tr::now);
    const auto line = label + ": " + payload;
    if (fallbackText.isEmpty()) {
        return line;
    }
    return QString("%1\n\n%2").arg(fallbackText, line);
}

QString CopyText(const HistoryMessageMarkupButton *button) {
    if (!Config::InlineCallback::Show.value() || !IsCallback(button)) {
        return QString();
    }
    return FormatPayload(button->data);
}

QString CopyContextItemText(const HistoryMessageMarkupButton *button) {
    if (!Config::InlineCallback::Show.value() || !IsCallback(button)) {
        return QString();
    }
    if (button->data.isEmpty()) {
        return QString();
    }
    return tr::ag_inline_callback_copy(tr::now);
}

void Setup(::Settings::Builder::SectionBuilder &builder) {
    Arcanegram::Settings::AddBoolRow(
        builder,
        u"arcanegram/inline_callback"_q,
        tr::ag_inline_callback_setting(),
        tr::ag_inline_callback_info(),
        Config::InlineCallback::Show);
}

} // namespace Arcanegram::InlineCallback
