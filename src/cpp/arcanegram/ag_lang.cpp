#include "arcanegram/ag_lang.h"

#include "lang/lang_instance.h"

namespace Arcanegram::Lang {
namespace {

rpl::lifetime &Lifetime() {
    static rpl::lifetime value;
    return value;
}

constexpr auto kRuStrings = R"strings(
"ag_settings_title" = "Arcanegram";
"ag_forwarded_show_date_setting" = "Показывать дату в пересланных сообщениях";
"ag_forwarded_show_date_info" = "Добавлять исходную дату отправки к строке «Переслано от».";
"ag_forwarded_date_label" = "Дата";
"ag_show_seconds_setting" = "Показывать секунды в метках времени";
"ag_show_seconds_info" = "Отображать ЧЧ:ММ:СС вместо ЧЧ:ММ в метках времени сообщений и подсказках.";
"ag_chat_wallpaper_disabled_setting" = "Игнорировать фон чата";
"ag_chat_wallpaper_disabled_info" = "Всегда использовать фон по умолчанию; игнорировать фон, установленный чатами и каналами.";
"ag_hidden_content_title" = "Скрытое";
"ag_hidden_content_info" = "Скрывайте сообщения по отправителю, инлайн-боту или регулярному выражению. Настройки применяются ко всем чатам.";
"ag_hidden_users_section" = "Пользователи";
"ag_hidden_users_info" = "Нажмите правой кнопкой на аватар, чтобы скрыть сообщения пользователя.";
"ag_hidden_users_empty" = "Нет скрытых пользователей.";
"ag_hidden_users_menu_hide" = "Скрыть сообщения от этого пользователя";
"ag_hidden_users_menu_unhide" = "Показать сообщения";
"ag_hidden_user_message" = "Скрытое сообщение";
"ag_hidden_user_name" = "Скрыто";
"ag_hidden_users_remove" = "Удалить";
"ag_hidden_bots_section" = "Инлайн-боты";
"ag_hidden_bots_info" = "Нажмите правой кнопкой на сообщение, отправленное через инлайн-бота, чтобы скрыть его.";
"ag_hidden_bots_empty" = "Нет скрытых инлайн-ботов.";
"ag_hidden_bots_menu_hide" = "Скрыть сообщения через этого бота";
"ag_hidden_bots_menu_unhide" = "Показать бота";
"ag_hidden_regexes_section" = "Шаблоны";
"ag_hidden_regexes_info" = "Скрывать любые сообщения, текст которых соответствует одному из этих регулярных выражений. Без учёта регистра. Некорректные шаблоны игнорируются.";
"ag_hidden_regexes_placeholder" = "Шаблон";
"ag_hidden_regexes_add" = "Добавить шаблон";
"ag_peer_id_copy_botapi" = "Копировать ID (Bot API)";
"ag_peer_id_copy_mtproto" = "Копировать ID (MTProto)";
"ag_fast_messages_title" = "Быстрые сообщения";
"ag_fast_messages_info" = "Введите сообщение для каждого слота. Привяжите клавиши в Настройках → Настройки чатов → Сочетания клавиш.";
"ag_fast_messages_slot" = "Слот {index}";
"ag_fast_messages_placeholder" = "Текст сообщения";
"ag_fast_messages_add" = "Добавить слот";
"ag_fast_messages_remove" = "Удалить слот";
"lng_shortcuts_fast_message" = "Быстрое сообщение {index}";
"ag_settings_appearance" = "Внешний вид чата";
"ag_settings_sync_title" = "Облачная синхронизация";
"ag_sync_enabled_setting" = "Включить облачную синхронизацию";
"ag_sync_enabled_info" = "Синхронизировать настройки Arcanegram между устройствами через arcanesync.";
"ag_sync_endpoint_placeholder" = "URL сервера";
"ag_sync_status_disabled" = "Отключено";
"ag_sync_status_ready" = "Готово";
"ag_sync_status_synced" = "Синхронизировано";
"ag_sync_status_syncing" = "Синхронизация…";
"ag_sync_status_offline" = "Не в сети";
"ag_sync_status_offline_pending" = "Не в сети · {n} ожидают";
"ag_sync_status_conflict" = "Разрешение конфликта…";
"ag_quick_access_title" = "Быстрый доступ";
"ag_quick_access_sidebar" = "Боковая панель";
"ag_quick_access_screenshot_mode" = "Режим скриншота";
"ag_quick_access_sidebar_screenshot_info" = "Добавить переключатель режима скриншота в меню боковой панели.";
"ag_streamer_title" = "Режим скриншота";
"ag_streamer_subtitle" = "Скрывать имена, аватары и идентификаторы при создании скриншотов.";
"ag_streamer_enabled" = "Анонимизировать других пользователей";
"ag_streamer_enabled_about" = "Заменяет имя каждого пользователя псевдонимом, убирает аватар, цвет и значки, скрывает телефон, имя пользователя, ID, описание, день рождения и время последнего визита.";
"ag_streamer_anonymize_bots" = "Анонимизировать ботов";
"ag_streamer_anonymize_bots_about" = "Боты открыты по умолчанию, так как обычно это намеренно. Включите, чтобы также давать им псевдонимы.";
"ag_streamer_mode_title" = "Режим стримера";
"ag_streamer_mode_enabled" = "Блокировать захват экрана";
"ag_streamer_mode_about" = "Запрещает другим приложениям захватывать окно Telegram через скриншоты, запись экрана или стриминг.";
"ag_quick_access_streamer_mode" = "Режим стримера";
"ag_quick_access_sidebar_streamer_mode_info" = "Добавить переключатель режима стримера в меню боковой панели.";
"ag_streamer_preview" = "Пример: {name}";
)strings";

void apply(const QString &langId) {
    if (langId.startsWith(u"ru"_q)) {
        ::Lang::GetInstance().ag_loadContent(QByteArray(kRuStrings));
    }
}

} // namespace

void Init() {
    auto &instance = ::Lang::GetInstance();
    apply(instance.id());
    std::move(instance.idChanges()) | rpl::on_next([](const QString &id) {
        apply(id);
    }, Lifetime());
}

} // namespace Arcanegram::Lang
