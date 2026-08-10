#include "HelpCommand.h"

void RegisterHelpCommand(dpp::cluster& bot) {
    dpp::slashcommand help_cmd("help", "Помощь", bot.me.id);
    bot.global_command_create(help_cmd);
}

void HandleHelpCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, const PermissionManager& perm_manager) {
    dpp::embed emb = dpp::embed()
        .set_color(0x5865F2)
        .set_title("🤖 Справочная система бота")
        .set_description("Добро пожаловать в меню помощи!\n\nИспользуйте кнопки ниже, чтобы переключаться между категориями команд.");

    // Создаем ряд кнопок вместо выпадающего списка
    dpp::component row;
    row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_primary).set_label("Главная").set_emoji("🏠").set_id("help_main"));
    row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label("Развлечения").set_emoji("🎮").set_id("help_fun"));

    // Показываем кнопку модерации только при наличии прав
    if (perm_manager.HasModerationAccess(event)) {
        row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_danger).set_label("Модерация").set_emoji("🛡️").set_id("help_moderation"));
    }

    dpp::message msg(event.command.channel_id, emb);
    msg.add_component(row);
    msg.set_flags(dpp::m_ephemeral);

    event.reply(msg);
}

void RegisterHelpHandlers(dpp::cluster& bot, const PermissionManager& perm_manager) {
    bot.on_button_click([&bot, &perm_manager](const dpp::button_click_t& event) {
        if (event.custom_id == "help_main" || event.custom_id == "help_fun" || event.custom_id == "help_moderation") {
            dpp::embed emb;
            emb.set_color(0x5865F2);

            if (event.custom_id == "help_main") {
                emb.set_title("🤖 Справочная система бота")
                    .set_description("Добро пожаловать в меню помощи!\n\nИспользуйте кнопки ниже, чтобы переключаться между категориями команд.");
            }
            else if (event.custom_id == "help_fun") {
                emb.set_title("🎮 Развлечения")
                    .set_description("Список доступных развлекательных команд:\n\n• **`/ship`** — Проверить совместимость двух пользователей с генерацией уникальной визуальной карточки.");
            }
            else if (event.custom_id == "help_moderation") {
                if (!perm_manager.HasModerationAccess(event)) {
                    event.reply(dpp::message("⚠️ У вас нет доступа к этой категории.").set_flags(dpp::m_ephemeral));
                    return;
                }
                emb.set_title("🛡️ Модерация и администрирование")
                    .set_description("Список доступных команд модерации и проверки анкет:\n\n• **`/accept`** — Проверить и принять анкету игрока, отправить ее в архив и выдать роль.\n• **`/purge`** — Удалить оффтоп сообщения в канале с логированием.");
            }

            // Пересоздаем ряд кнопок с актуальными правами для текущего пользователя
            dpp::component row;
            row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_primary).set_label("Главная").set_emoji("🏠").set_id("help_main"));
            row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_secondary).set_label("Развлечения").set_emoji("🎮").set_id("help_fun"));

            if (perm_manager.HasModerationAccess(event)) {
                row.add_component(dpp::component().set_type(dpp::cot_button).set_style(dpp::cos_danger).set_label("Модерация").set_emoji("🛡️").set_id("help_moderation"));
            }

            dpp::message msg;
            msg.add_embed(emb);
            msg.add_component(row);

            // Обновляем сообщение при нажатии
            event.reply(dpp::ir_update_message, msg);
        }
        });
}