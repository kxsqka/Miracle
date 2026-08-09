#include "AcceptCommand.h"
#include <algorithm>
#include <functional>
#include <iomanip>
#include <sstream>

const dpp::snowflake ACCEPT_ROLE_ID = 1501989416455897295ULL;
const dpp::snowflake ACCEPTED_ROLE_ID = 1501989416321810575ULL;

const std::vector<std::string> TEMPLATE_KEYWORDS = {
    "Основная информация",
    "Внешность",
    "Характер",
    "Характеристики",
    "Способности",
    "Слабости",
    "Биография",
    "Инвентарь"
};

void RegisterAcceptCommand(dpp::cluster& bot) {
    dpp::slashcommand accept_cmd("accept", "Принять анкету (Только Рустодия)", bot.me.id);

    accept_cmd.add_option(
        dpp::command_option(dpp::co_user, "user", "Чья анкета", true)
    );

    bot.global_command_create(accept_cmd);
}

void HandleAcceptCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake archive_channel_id, const std::vector<dpp::snowflake>& blacklist) {
    dpp::snowflake admin_id = event.command.usr.id;

    if (std::find(blacklist.begin(), blacklist.end(), admin_id) != blacklist.end()) {
        event.reply(dpp::message("У вас недостаточно бурмалды для пользования Всевышним, сосите!").set_flags(dpp::m_ephemeral));
        return;
    }

    bool has_permission = false;
    if (event.command.app_permissions.has(dpp::p_administrator)) {
        has_permission = true;
    }
    else {
        const auto& roles = event.command.member.get_roles();
        if (std::find(roles.begin(), roles.end(), ACCEPT_ROLE_ID) != roles.end()) {
            has_permission = true;
        }
    }

    if (!has_permission) {
        event.reply(dpp::message("У вас недостаточно бурмалды для пользования Всевышним, сосите!").set_flags(dpp::m_ephemeral));
        return;
    }

    dpp::snowflake target_user_id = std::get<dpp::snowflake>(event.get_parameter("user"));

    event.thinking(true);

    bot.messages_get(event.command.channel_id, 0, 0, 0, 100, [&bot, event, archive_channel_id, target_user_id](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error()) {
            event.edit_original_response(dpp::message("Ошибка при получении истории сообщений."));
            return;
        }

        dpp::message_map messages = std::get<dpp::message_map>(callback.value);
        std::vector<dpp::message> user_messages;

        for (const auto& [id, msg] : messages) {
            if (msg.author.id == target_user_id) {
                user_messages.push_back(msg);
            }
        }

        std::sort(user_messages.begin(), user_messages.end(), [](const dpp::message& a, const dpp::message& b) {
            return a.id.get_creation_time() < b.id.get_creation_time();
            });

        if (user_messages.size() > 5) {
            user_messages.erase(user_messages.begin(), user_messages.end() - 5);
        }

        if (user_messages.empty()) {
            event.edit_original_response(dpp::message("Не удалось найти сообщения этого пользователя в данном канале."));
            return;
        }

        std::string full_application_text = "";
        std::string found_image_url = "";

        for (const auto& msg : user_messages) {
            full_application_text += msg.content + "\n\n";

            if (found_image_url.empty() && !msg.attachments.empty()) {
                for (const auto& att : msg.attachments) {
                    if (att.url.find(".png") != std::string::npos ||
                        att.url.find(".jpg") != std::string::npos ||
                        att.url.find(".jpeg") != std::string::npos ||
                        att.url.find(".webp") != std::string::npos ||
                        att.content_type.find("image") != std::string::npos) {
                        found_image_url = att.url;
                        break;
                    }
                }
            }
        }

        int found_keywords = 0;
        for (const auto& kw : TEMPLATE_KEYWORDS) {
            if (full_application_text.find(kw) != std::string::npos) {
                found_keywords++;
            }
        }

        if (found_keywords < 3) {
            event.edit_original_response(dpp::message("⚠️ Сообщения игрока не содержат ключевых разделов шаблона анкеты."));
            return;
        }

        bot.guild_member_add_role(event.command.guild_id, target_user_id, ACCEPTED_ROLE_ID, [](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error()) {
            }
            });

        std::vector<std::string> chunks;
        std::string current_chunk = "";

        size_t pos = 0;
        while (pos < full_application_text.length()) {
            size_t next_pos = full_application_text.find('\n', pos);
            if (next_pos == std::string::npos) next_pos = full_application_text.length();
            else next_pos++;

            std::string line = full_application_text.substr(pos, next_pos - pos);
            pos = next_pos;

            if (current_chunk.length() + line.length() > 4000) {
                chunks.push_back(current_chunk);
                current_chunk = "";
            }
            current_chunk += line;
        }
        if (!current_chunk.empty()) {
            chunks.push_back(current_chunk);
        }

        if (chunks.size() > 10) {
            chunks.resize(10);
        }

        std::time_t now = std::time(nullptr);
        std::tm local_tm;
#if defined(_MSC_VER)
        localtime_s(&local_tm, &now);
#else
        local_tm = *std::localtime(&now);
#endif
        std::ostringstream time_oss;
        time_oss << std::put_time(&local_tm, "%d.%m.%Y, %H:%M");
        std::string formatted_time = time_oss.str();

        std::vector<dpp::message> archive_messages;
        for (size_t i = 0; i < chunks.size(); ++i) {
            dpp::embed emb = dpp::embed()
                .set_color(0x0085ff)
                .set_footer(dpp::embed_footer().set_text(formatted_time))
                .set_description(chunks[i]);

            if (i == 0) {
                emb.set_title("📄 Принятая анкета персонажа");
            }
            else {
                emb.set_title("📄 Принятая анкета персонажа (продолжение)");
            }

            if (i == chunks.size() - 1) {
                emb.add_field("Пользователь", "<@" + std::to_string(target_user_id) + ">", true)
                    .add_field("Анкетолог", "<@" + std::to_string(event.command.usr.id) + ">", true);

                if (!found_image_url.empty()) {
                    emb.set_image(found_image_url);
                }
            }

            dpp::message archive_msg(archive_channel_id, emb);
            archive_messages.push_back(archive_msg);
        }

        std::shared_ptr<std::function<void(size_t)>> send_next_ptr = std::make_shared<std::function<void(size_t)>>();
        *send_next_ptr = [&bot, archive_messages, send_next_ptr, event](size_t index) {
            if (index < archive_messages.size()) {
                bot.message_create(archive_messages[index], [index, send_next_ptr](const dpp::confirmation_callback_t& callback) {
                    (*send_next_ptr)(index + 1);
                    });
            }
            else {
                event.edit_original_response(dpp::message("✅ Готово!"));
            }
            };

        (*send_next_ptr)(0);
        });
}