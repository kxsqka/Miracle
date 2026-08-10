#include <dpp/dpp.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "Permissions.h"
#include "AcceptCommand.h"
#include "PurgeCommand.h"
#include "ShipCommand.h"
#include "HelpCommand.h"

using json = nlohmann::json;

const dpp::snowflake LOG_CHANNEL_ID = 1501989417307345052ULL;
const dpp::snowflake ARCHIVE_CHANNEL_ID = 1501989420121849993ULL;

int main() {
    std::ifstream config_file("config.json");
    if (!config_file.is_open()) {
        std::cerr << "Не удалось открыть файл config.json! Убедитесь, что он создан." << std::endl;
        return 1;
    }

    json config;
    config_file >> config;

    std::string token = config["token"];

    dpp::cluster bot(token);
    PermissionManager perm_manager;

    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot, &perm_manager](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            RegisterAcceptCommand(bot);
            RegisterPurgeCommand(bot);
            RegisterShipCommand(bot);
            RegisterHelpCommand(bot);
        }

        // Регистрация обработчиков нажатий кнопок для меню помощи
        RegisterHelpHandlers(bot, perm_manager);

        dpp::activity act;
        act.type = dpp::activity_type::at_streaming;
        act.name = "НЕ ОФНУ ПОКА НЕ СДЕЛАЮ 1000 ГПМ, 10К ММР, АМБАССАДОР ЗЕЛЕНЫХ ТРЕУГОЛЬНИКОВ";
        act.url = "https://www.twitch.tv/lizok_ssk";

        bot.set_presence(dpp::presence(dpp::ps_dnd, act));
        });

    bot.on_slashcommand([&bot, &perm_manager](const dpp::slashcommand_t& event) {
        std::string cmd_name = event.command.get_command_name();

        // Команду help разрешаем вызывать всем, категории внутри проверяются динамически
        if (cmd_name != "help" && !perm_manager.HasAccess(event)) {
            event.reply(dpp::message("У вас недостаточно бурмалды для пользования Всевышним, сосите!").set_flags(dpp::m_ephemeral));
            return;
        }

        if (cmd_name == "accept") {
            HandleAcceptCommand(bot, event, ARCHIVE_CHANNEL_ID);
        }
        else if (cmd_name == "purge") {
            HandlePurgeCommand(bot, event, LOG_CHANNEL_ID);
        }
        else if (cmd_name == "ship") {
            HandleShipCommand(bot, event);
        }
        else if (cmd_name == "help") {
            HandleHelpCommand(bot, event, perm_manager);
        }
        });

    bot.start(dpp::st_wait);
    return 0;
}