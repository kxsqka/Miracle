#include "PurgeCommand.h"
#include <algorithm>

bool is_offtopic(const std::string& content) {
    std::vector<std::string> prefixes = { "//", "((", "))", "Вне:", "\\" };
    for (const auto& p : prefixes) {
        if (content.find(p) == 0) {
            return true;
        }
    }
    return false;
}

void RegisterPurgeCommand(dpp::cluster& bot) {
    dpp::slashcommand purge_cmd("purge", "Удалить оффтоп сообщения", bot.me.id);
    purge_cmd.add_option(
        dpp::command_option(dpp::co_integer, "amount", "Количество сообщений для проверки (до 100)", true)
    );
    bot.global_command_create(purge_cmd);
}

void HandlePurgeCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake log_channel_id) {
    int64_t amount = std::get<int64_t>(event.get_parameter("amount"));
    if (amount > 100) amount = 100;
    if (amount < 1) amount = 1;

    event.thinking(true);

    bot.messages_get(event.command.channel_id, 0, 0, 0, amount, [&bot, event, log_channel_id](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            event.edit_original_response(dpp::message("Ошибка при получении истории сообщений."));
            return;
        }

        dpp::message_map messages = std::get<dpp::message_map>(callback.value);
        std::vector<dpp::snowflake> to_delete;
        std::vector<std::string> deleted_logs;

        for (const auto& [id, msg] : messages) {
            if (is_offtopic(msg.content)) {
                to_delete.push_back(id);
                deleted_logs.push_back("**" + msg.author.username + "**: " + msg.content);
            }
        }

        if (to_delete.empty()) {
            event.edit_original_response(dpp::message("Оффтоп сообщений среди последних не найдено."));
            return;
        }

        bot.message_delete_bulk(to_delete, event.command.channel_id, [&bot, event, log_channel_id, deleted_logs, to_delete_size = to_delete.size()](const dpp::confirmation_callback_t& del_cb) {
            if (del_cb.is_error()) {
                event.edit_original_response(dpp::message("Ошибка удаления. Возможно, сообщения старше 14 дней."));
                return;
            }

            auto logs_to_process = deleted_logs;
            std::reverse(logs_to_process.begin(), logs_to_process.end());

            std::vector<std::string> chunks;
            std::string current_chunk = "";

            for (const auto& log_text : logs_to_process) {
                if (current_chunk.length() + log_text.length() > 950) {
                    chunks.push_back(current_chunk);
                    current_chunk = "";
                }
                current_chunk += log_text + "\n";
            }
            if (!current_chunk.empty()) {
                chunks.push_back(current_chunk);
            }

            std::vector<dpp::embed> all_embeds;
            for (size_t i = 0; i < chunks.size(); ++i) {
                dpp::embed emb = dpp::embed()
                    .set_color(dpp::colors::orange)
                    .set_timestamp(time(0));

                if (i == 0) {
                    emb.set_title("🗑️ Очистка оффтопа")
                        .add_field("Модератор", "<@" + std::to_string(event.command.usr.id) + ">", true)
                        .add_field("Канал", "<#" + std::to_string(event.command.channel_id) + ">", true)
                        .add_field("Удалено сообщений", std::to_string(to_delete_size), true)
                        .add_field("Удаленный текст (ч. 1)", chunks[i]);
                }
                else {
                    emb.set_title("🗑️ Очистка оффтопа (продолжение)")
                        .add_field("Удаленный текст (ч. " + std::to_string(i + 1) + ")", chunks[i]);
                }
                all_embeds.push_back(emb);
            }

            for (const auto& emb : all_embeds) {
                dpp::message log_msg(log_channel_id, emb);
                bot.message_create(log_msg);
            }

            event.edit_original_response(dpp::message("Успешно удалено " + std::to_string(to_delete_size) + " оффтоп-сообщений. Лог отправлен."));
            });
        });
}