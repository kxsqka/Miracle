#define _USE_MATH_DEFINES
#include "ShipCommand.h"
#include <cairo/cairo.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <random>

struct ImageBuffer {
    std::vector<unsigned char> data;
};

struct PngReadContext {
    const char* data;
    size_t size;
    size_t offset;
};

static cairo_status_t png_read_func(void* closure, unsigned char* data, unsigned int length) {
    PngReadContext* ctx = static_cast<PngReadContext*>(closure);
    if (ctx->offset + length > ctx->size) {
        length = ctx->size - ctx->offset;
    }
    if (length > 0) {
        std::memcpy(data, ctx->data + ctx->offset, length);
        ctx->offset += length;
    }
    return CAIRO_STATUS_SUCCESS;
}

cairo_status_t write_to_buffer(void* closure, const unsigned char* data, unsigned int length) {
    ImageBuffer* buffer = static_cast<ImageBuffer*>(closure);
    buffer->data.insert(buffer->data.end(), data, data + length);
    return CAIRO_STATUS_SUCCESS;
}

// Локальная база пожеланий и предсказаний (описания не изменены)
std::string GetShipWish(int percent, size_t seed) {
    static const std::vector<std::string> tier_0_20 = {
        "Лучше держитесь друг от друга на расстоянии броска гранаты.",
        "Совместимость стремится к нулю быстрее, чем FPS на старом ноуте.",
        "Звезды шепчут: «Даже не думайте».",
        "Ваш союз продержится меньше, чем перезарядка ульта.",
        "Максимум — поздороваться в общем чате и разойтись.",
        "Бегите друг от друга, пока не начался ебаный цирк.",
        "Совместимость как у C++ с новичком: один сплошной Segmentation fault.",
        "Шансы ниже, чем выбить аркану с бесплатного сундука.",
        "Даже не думайте. Вы загрызете друг друга через 5 минут в голосовом.",
        "Ваш союз замусорен сильнее, чем папка LOLI у Шведа.",
        "Максимум, что вас ждет — взаимный блок во всех соцсетях."
    };

    static const std::vector<std::string> tier_21_50 = {
        "Шансы есть, но кому-то придется сильно уступать.",
        "Чисто дружеский поход в таверну, не более.",
        "Сложный союз, но в дуэтах и не такое тащили.",
        "Терпимо, но кто-то один точно будет мыть посуду.",
        "Вы нейтральны друг к другу, как швейцарский банк.",
        "Жить будете, но регулярно хуесосить друг друга в чате.",
        "Типичный дуэт в рейтинге: один тащит, второй скулит.",
        "Чисто попиздеть ночью по пьяни и разбежаться.",
        "Сойдет, но кто-то один в этой паре явно терпила.",
        "Выглядит сомнительно, но забавы ради попробовать можно.",
        "Нейтрально. Как еда без соли или пиво без алкоголя."
    };

    static const std::vector<std::string> tier_51_80 = {
        "Отличная пара! Главное — не спорить из-за бытовухи.",
        "Идеальный дуэт для парных игр и поздних разговоров.",
        "Кажется, между вами пробежала искра.",
        "Очень достойный результат, пора звать на свидание!",
        "Вы отлично дополняете недостающие недостатки друг друга.",
        "Нормально! Можно и в ДС посидеть, и пиваса вместе бахнуть.",
        "Звезды говорят: пердеть в присутствии друг друга уже можно.",
        "Вы отлично дополняете ебанину друг друга.",
        "Не идеал, но трахаться... ой, то есть общаться можно.",
        "Главное — не запускайте вместе кастомки, иначе разведетесь.",
        "Ваш флирт уже заебал половину сервера, идите в ЛС."
    };

    static const std::vector<std::string> tier_81_100 = {
        "Совет да любовь! Готовьте свадебные наряды.",
        "Вы созданы друг для друга, как C++ и нулевые указатели!",
        "Абсолютный мэтч! Срочно ставьте парные аватарки.",
        "Ваша любовь способна растопить даже сердце ГМа.",
        "100% гармония. Ждем приглашение на свадьбу на сервере!",
        "Ебать вы парочка, срочно ставьте парные аватарки и не бесите.",
        "Тут уже запахло совместной ипотекой и парными татухами.",
        "Идеальный мэтч. Вы созданы, чтобы вместе бесить весь чат.",
        "Кажется, кое-кто сегодня ночью не будет спать...",
        "Ваша менталочка официально синхронизирована.",
        "Совет да любовь. И пусть весь сервер подохнет."
    };

    const std::vector<std::string>* current_tier;

    if (percent <= 20)      current_tier = &tier_0_20;
    else if (percent <= 50) current_tier = &tier_21_50;
    else if (percent <= 80) current_tier = &tier_51_80;
    else                    current_tier = &tier_81_100;

    std::mt19937 rng(static_cast<unsigned int>(seed));
    std::uniform_int_distribution<size_t> dist(0, current_tier->size() - 1);

    return (*current_tier)[dist(rng)];
}

std::string GetMatchTitle(int percent) {
    if (percent == 100) return "ABSOLUTE SOULMATES!";
    if (percent >= 80)  return "PERFECT MATCH!";
    if (percent >= 50)  return "GREAT COMPATIBILITY!";
    if (percent >= 20)  return "COMPLEX DYNAMICS!";
    return "TOTAL DISASTER!";
}

// Вспомогательная функция для переноса текста по словам
static std::vector<std::string> wrap_text(cairo_t* cr, const std::string& text, double max_width) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word;
    std::string current_line;
    cairo_text_extents_t ext;

    while (stream >> word) {
        std::string test_line = current_line.empty() ? word : current_line + " " + word;
        cairo_text_extents(cr, test_line.c_str(), &ext);
        if (ext.width > max_width && !current_line.empty()) {
            lines.push_back(current_line);
            current_line = word;
        }
        else {
            current_line = test_line;
        }
    }
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    return lines;
}

void draw_heart(cairo_t* cr, double x, double y, double width, double height, double fill_percent) {
    cairo_save(cr);
    cairo_translate(cr, x, y);

    auto make_heart_path = [width, height](cairo_t* c) {
        cairo_move_to(c, width / 2, height / 4);
        cairo_curve_to(c, width / 2, 0, 0, 0, 0, height / 2.5);
        cairo_curve_to(c, 0, height / 1.3, width / 2, height, width / 2, height);
        cairo_curve_to(c, width / 2, height, width, height / 1.3, width, height / 2.5);
        cairo_curve_to(c, width, 0, width / 2, 0, width / 2, height / 4);
        cairo_close_path(c);
        };

    // Подложка
    make_heart_path(cr);
    cairo_set_source_rgba(cr, 0.22, 0.20, 0.25, 0.8);
    cairo_fill_preserve(cr);

    cairo_set_source_rgba(cr, 1.0, 0.4, 0.6, 0.4);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);

    // Заполнение снизу вверх
    if (fill_percent > 0) {
        make_heart_path(cr);
        cairo_clip(cr);

        double fill_h = height * (fill_percent / 100.0);

        cairo_pattern_t* pattern = cairo_pattern_create_linear(0, height - fill_h, 0, height);
        cairo_pattern_add_color_stop_rgba(pattern, 0.0, 1.0, 0.3, 0.6, 1.0);
        cairo_pattern_add_color_stop_rgba(pattern, 1.0, 1.0, 0.1, 0.3, 1.0);
        cairo_set_source(cr, pattern);

        cairo_rectangle(cr, 0, height - fill_h, width, fill_h);
        cairo_fill(cr);

        cairo_pattern_destroy(pattern);
    }

    cairo_restore(cr);
}

void RegisterShipCommand(dpp::cluster& bot) {
    dpp::slashcommand ship_cmd("ship", "Проверить совместимость двух пользователей", bot.me.id);
    ship_cmd.add_option(
        dpp::command_option(dpp::co_user, "user1", "Первый пользователь", true)
    ).add_option(
        dpp::command_option(dpp::co_user, "user2", "Второй пользователь", false)
    );
    bot.global_command_create(ship_cmd);
}

void HandleShipCommand(dpp::cluster& bot, const dpp::slashcommand_t& event) {
    dpp::snowflake user1_id = std::get<dpp::snowflake>(event.get_parameter("user1"));
    dpp::snowflake user2_id;

    if (auto p2 = event.get_parameter("user2"); std::holds_alternative<dpp::snowflake>(p2)) {
        user2_id = std::get<dpp::snowflake>(p2);
    }
    else {
        user2_id = event.command.usr.id;
    }

    if (user1_id == user2_id) {
        event.reply(dpp::message("⚠️ Нельзя шипперить самого себя. Совместимость 100%, лол.").set_flags(dpp::m_ephemeral));
        return;
    }

    event.thinking();

    std::string seed_str = std::to_string(user1_id) + std::to_string(user2_id);
    size_t hash = std::hash<std::string>{}(seed_str);
    int percent = static_cast<int>(hash % 101);

    auto generate_and_reply = [event, percent, hash](const std::string& name1, const std::string& avatar1_data, const std::string& name2, const std::string& avatar2_data) {
        const int img_w = 1024, img_h = 512;
        cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img_w, img_h);
        cairo_t* cr = cairo_create(surface);

        // Основной фон карточки
        cairo_set_source_rgb(cr, 0.09, 0.09, 0.10);
        cairo_rectangle(cr, 0, 0, img_w, img_h);
        cairo_fill(cr);

        // Верхний заголовок: MIRACLE BOT | /SHIP
        cairo_set_source_rgba(cr, 0.6, 0.6, 0.65, 1.0);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 15.0);
        std::string header_text = "MIRACLE BOT  |  /SHIP";
        cairo_text_extents_t ext;
        cairo_text_extents(cr, header_text.c_str(), &ext);
        cairo_move_to(cr, (img_w / 2.0) - (ext.width / 2.0), 45);
        cairo_show_text(cr, header_text.c_str());

        // Линия прогресс-бара, соединяющая аватарки
        cairo_set_source_rgba(cr, 0.25, 0.22, 0.28, 1.0);
        cairo_set_line_width(cr, 6.0);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_move_to(cr, 260, 160);
        cairo_line_to(cr, 764, 160);
        cairo_stroke(cr);

        // Заполненная часть линии прогресса
        if (percent > 0) {
            double line_fill_end = 260 + (504.0 * (percent / 100.0));
            cairo_set_source_rgba(cr, 1.0, 0.35, 0.55, 1.0);
            cairo_set_line_width(cr, 6.0);
            cairo_move_to(cr, 260, 160);
            cairo_line_to(cr, line_fill_end, 160);
            cairo_stroke(cr);
        }

        // Текст статуса и процента над линией
        std::string match_status_text = std::to_string(percent) + "%  -  " + GetMatchTitle(percent);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_font_size(cr, 20.0);
        cairo_text_extents(cr, match_status_text.c_str(), &ext);
        cairo_move_to(cr, (img_w / 2.0) - (ext.width / 2.0), 120);
        cairo_show_text(cr, match_status_text.c_str());

        // Функция отрисовки аватарки
        auto draw_avatar = [&](const std::string& data, double x, double y, double r) {
            cairo_save(cr);

            // Обводка
            cairo_arc(cr, x, y, r + 4, 0, 2 * M_PI);
            cairo_set_source_rgba(cr, 1.0, 0.4, 0.6, 0.8);
            cairo_fill(cr);

            // Обрезка по кругу
            cairo_arc(cr, x, y, r, 0, 2 * M_PI);
            cairo_clip(cr);

            PngReadContext read_ctx{ data.data(), data.size(), 0 };
            cairo_surface_t* image = cairo_image_surface_create_from_png_stream(png_read_func, &read_ctx);

            if (image && cairo_surface_status(image) == CAIRO_STATUS_SUCCESS) {
                double src_w = cairo_image_surface_get_width(image);
                double src_h = cairo_image_surface_get_height(image);

                cairo_translate(cr, x - r, y - r);
                double scale = (r * 2.0) / std::min(src_w, src_h);
                cairo_scale(cr, scale, scale);

                cairo_set_source_surface(cr, image, 0, 0);
                cairo_paint(cr);
                cairo_surface_destroy(image);
            }
            cairo_restore(cr);
            };

        // Рисуем аватарки (слева и справа)
        draw_avatar(avatar1_data, 170, 205, 90);
        draw_avatar(avatar2_data, 854, 205, 90);

        // Отрисовка 5 сердечек под линией прогресса
        const int heart_w = 42, heart_h = 42;
        const int heart_spacing = 58;
        double total_hearts_w = (5 * heart_w) + (4 * 16);
        double hearts_start_x = (img_w / 2.0) - (total_hearts_w / 2.0);
        double hearts_start_y = 195;

        double remaining_percent = percent;
        for (int i = 0; i < 5; ++i) {
            double current_fill = 0;
            if (remaining_percent >= 20.0) {
                current_fill = 100.0;
                remaining_percent -= 20.0;
            }
            else {
                current_fill = (remaining_percent / 20.0) * 100.0;
                remaining_percent = 0;
            }

            draw_heart(cr, hearts_start_x + (i * heart_spacing), hearts_start_y, heart_w, heart_h, current_fill);
        }

        // --- АККУРАТНАЯ ПЛАШКА С АВТОМАТИЧЕСКИМ ПЕРЕНОСОМ СТРОК ---
        std::string wish_phrase = GetShipWish(percent, hash);

        cairo_set_font_size(cr, 15.0);

        // Ограничиваем ширину текста максимум 400 пикселями, чтобы он никогда не доставал до аватарок
        auto lines = wrap_text(cr, wish_phrase, 400.0);

        double line_height = 20.0;
        double box_padding_y = 12.0;
        double box_padding_x = 24.0;
        double box_h = (lines.size() * line_height) + (box_padding_y * 2.0);

        double max_line_w = 0;
        for (const auto& line : lines) {
            cairo_text_extents(cr, line.c_str(), &ext);
            if (ext.width > max_line_w) {
                max_line_w = ext.width;
            }
        }

        double box_w = max_line_w + (box_padding_x * 2.0);
        double box_x = (img_w / 2.0) - (box_w / 2.0);
        double box_y = 252.0;
        double radius = 8.0;

        cairo_save(cr);
        cairo_new_path(cr);
        cairo_arc(cr, box_x + radius, box_y + radius, radius, M_PI, 3.0 * M_PI / 2.0);
        cairo_arc(cr, box_x + box_w - radius, box_y + radius, radius, 3.0 * M_PI / 2.0, 2.0 * M_PI);
        cairo_arc(cr, box_x + box_w - radius, box_y + box_h - radius, radius, 0, M_PI / 2.0);
        cairo_arc(cr, box_x + radius, box_y + box_h - radius, radius, M_PI / 2.0, M_PI);
        cairo_close_path(cr);

        cairo_set_source_rgba(cr, 0.14, 0.13, 0.17, 0.95);
        cairo_fill_preserve(cr);
        cairo_set_source_rgba(cr, 1.0, 0.35, 0.55, 0.35);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        cairo_restore(cr);

        cairo_set_source_rgba(cr, 1.0, 0.8, 0.9, 1.0);
        double text_start_y = box_y + box_padding_y + 11.0;
        for (size_t i = 0; i < lines.size(); ++i) {
            cairo_text_extents(cr, lines[i].c_str(), &ext);
            double line_x = (img_w / 2.0) - (ext.width / 2.0);
            cairo_move_to(cr, line_x, text_start_y + (i * line_height));
            cairo_show_text(cr, lines[i].c_str());
        }
        // -----------------------------------------------------------

        // Никнеймы пользователей под аватарками
        cairo_set_font_size(cr, 22.0);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        auto draw_name = [&](const std::string& name, double x, double y) {
            std::string display_name = name;
            if (display_name.length() > 16) {
                display_name = display_name.substr(0, 15) + "…";
            }
            cairo_text_extents(cr, display_name.c_str(), &ext);
            cairo_move_to(cr, x - ext.width / 2.0, y);
            cairo_show_text(cr, display_name.c_str());
            };
        draw_name(name1, 170, 345);
        draw_name(name2, 854, 345);

        // Нижний футер
        std::string footer_text = "Compatibility determined by the Miracle Bot.";
        cairo_set_font_size(cr, 14.0);
        cairo_set_source_rgba(cr, 0.55, 0.55, 0.6, 1.0);
        cairo_text_extents(cr, footer_text.c_str(), &ext);
        cairo_move_to(cr, (img_w / 2.0) - (ext.width / 2.0), 430);
        cairo_show_text(cr, footer_text.c_str());

        ImageBuffer final_img;
        cairo_surface_write_to_png_stream(surface, write_to_buffer, &final_img);

        cairo_destroy(cr);
        cairo_surface_destroy(surface);

        std::string final_png_data(final_img.data.begin(), final_img.data.end());
        dpp::message final_msg(event.command.channel_id, "❤️ Результат шипперинга!");
        final_msg.add_file("ship.png", final_png_data);

        event.edit_original_response(final_msg);
        };

    bot.user_get(user1_id, [&bot, event, user2_id, generate_and_reply](const dpp::confirmation_callback_t& cb1) {
        if (cb1.is_error()) {
            event.edit_original_response(dpp::message("Не удалось найти первого пользователя."));
            return;
        }
        dpp::user_identified u1 = std::get<dpp::user_identified>(cb1.value);

        bot.user_get(user2_id, [&bot, event, u1, generate_and_reply](const dpp::confirmation_callback_t& cb2) {
            if (cb2.is_error()) {
                event.edit_original_response(dpp::message("Не удалось найти второго пользователя."));
                return;
            }
            dpp::user_identified u2 = std::get<dpp::user_identified>(cb2.value);

            std::string url1 = u1.get_avatar_url(512);
            std::string url2 = u2.get_avatar_url(512);
            std::string name1 = u1.format_username();
            std::string name2 = u2.format_username();

            bot.request(url1, dpp::m_get, [&bot, event, name1, name2, url2, generate_and_reply](const dpp::http_request_completion_t& req1) {
                if (req1.status != 200) {
                    event.edit_original_response(dpp::message("Ошибка скачивания первого аватара."));
                    return;
                }
                std::string data1 = req1.body;

                bot.request(url2, dpp::m_get, [&bot, event, name1, name2, data1, generate_and_reply](const dpp::http_request_completion_t& req2) {
                    if (req2.status != 200) {
                        event.edit_original_response(dpp::message("Ошибка скачивания второго аватара."));
                        return;
                    }
                    std::string data2 = req2.body;

                    generate_and_reply(name1, data1, name2, data2);
                    });
                });
            });
        });
}