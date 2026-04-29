add_library(arcanegram STATIC)
init_target(arcanegram)

# match stock pch so includes like core/application.h transitively resolve.
target_precompile_headers(arcanegram PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../stdafx.h)

include(${submodules_loc}/lib_ui/cmake/generate_styles.cmake)

set(arcanegram_style_files
    arcanegram.style
)
set(arcanegram_dependent_style_files
    ${submodules_loc}/lib_ui/ui/colors.palette
    ${submodules_loc}/lib_ui/ui/basic.style
    ${submodules_loc}/lib_ui/ui/widgets/widgets.style
)
generate_styles(arcanegram ${CMAKE_CURRENT_SOURCE_DIR} "${arcanegram_style_files}" "${arcanegram_dependent_style_files}")

nice_target_sources(arcanegram ${CMAKE_CURRENT_SOURCE_DIR}
PRIVATE
    ${arcanegram_style_files}

    ag_hooks.h
    ag_hooks.cpp
    ag_config.h
    ag_config.cpp
    ag_refresh.h
    ag_refresh.cpp
    features/ag_forwarded_header.h
    features/ag_forwarded_header.cpp
    features/ag_show_seconds.h
    features/ag_show_seconds.cpp
    features/ag_hidden_users.h
    features/ag_hidden_users.cpp
    features/ag_chat_wallpaper.h
    features/ag_chat_wallpaper.cpp
    ui/ag_settings_widgets.h
    ui/ag_settings_widgets.cpp
    ui/ag_settings_main.h
    ui/ag_settings_main.cpp
    ui/ag_hidden_users_settings.h
    ui/ag_hidden_users_settings.cpp
    features/ag_sync_feature.h
    features/ag_sync_feature.cpp
    sync/ag_sync_initdata.h
    sync/ag_sync_initdata.cpp
    sync/ag_sync_client.h
    sync/ag_sync_client.cpp
    sync/ag_sync_state.h
    sync/ag_sync_state.cpp
    sync/ag_sync_keys.h
    sync/ag_sync_keys.cpp
    sync/ag_sync_engine.h
    sync/ag_sync_engine.cpp
)

target_include_directories(arcanegram PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(arcanegram
PRIVATE
    tdesktop::td_lang
    tdesktop::td_mtproto
    tdesktop::td_scheme
    tdesktop::td_ui
    desktop-app::lib_ui
    desktop-app::lib_base
    desktop-app::lib_rpl
    desktop-app::lib_storage
    Qt::Network
)

# config.h #errors without these; they're only set on the Telegram target by stock.
target_compile_definitions(arcanegram PRIVATE
    TDESKTOP_API_ID=${TDESKTOP_API_ID}
    TDESKTOP_API_HASH=${TDESKTOP_API_HASH}
)
