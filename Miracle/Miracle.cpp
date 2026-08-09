#include <dpp/dpp.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "AcceptCommand.h"
#include "PurgeCommand.h"

using json = nlohmann::json;

const dpp::snowflake LOG_CHANNEL_ID = 1501989417307345052ULL;
const dpp::snowflake ARCHIVE_CHANNEL_ID = 1502372785635197090ULL;

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

    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            RegisterAcceptCommand(bot);
            RegisterPurgeCommand(bot);
        }

        dpp::activity act;
        act.type = dpp::activity_type::at_streaming;
        act.name = "НЕ ОФНУ ПОКА НЕ СДЕЛАЮ 1000 ГПМ, 10К ММР, АМБАССАДОР ЗЕЛЕНЫХ ТРЕУГОЛЬНИКОВ";
        act.url = "https://www.twitch.tv/lizok_ssk";

        bot.set_presence(dpp::presence(dpp::ps_dnd, act));
        });

    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {
        std::vector<dpp::snowflake> blacklist = {
            1503055805967106181
        };

        if (event.command.get_command_name() == "accept") {
            HandleAcceptCommand(bot, event, ARCHIVE_CHANNEL_ID, blacklist);
        }
        else if (event.command.get_command_name() == "purge") {
            HandlePurgeCommand(bot, event, LOG_CHANNEL_ID, blacklist);
        }

        });

    bot.start(dpp::st_wait);
    return 0;
}