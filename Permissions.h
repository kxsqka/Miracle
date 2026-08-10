#pragma once
#include <dpp/dpp.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

enum class RoleCategory {
    Admin,
    Anketolog,
    GameMaster
};

struct PermissionManager {
    std::vector<dpp::snowflake> blacklist = {};

    std::vector<dpp::snowflake> admin_users = {
        985311680537956383ULL
    };

    std::vector<dpp::snowflake> admins = { 1501989416472805396ULL };
    std::vector<dpp::snowflake> anketologs = { 1501989416455897295ULL };
    std::vector<dpp::snowflake> game_masters = { 1501989416455897296ULL };

    std::unordered_map<std::string, std::vector<RoleCategory>> command_access = {
        { "accept", { RoleCategory::Anketolog, RoleCategory::Admin } },
        { "purge",  { RoleCategory::GameMaster, RoleCategory::Admin } }
    };

    bool HasAccess(const dpp::interaction_create_t& event) const {
        dpp::snowflake user_id = event.command.usr.id;

        if (std::find(blacklist.begin(), blacklist.end(), user_id) != blacklist.end()) {
            return false;
        }

        if (std::find(admin_users.begin(), admin_users.end(), user_id) != admin_users.end()) {
            return true;
        }

        std::string cmd_name = event.command.get_command_name();
        auto it = command_access.find(cmd_name);

        if (it == command_access.end()) {
            return true;
        }

        if (event.command.app_permissions.has(dpp::p_administrator)) {
            return true;
        }

        const auto& user_roles = event.command.member.get_roles();

        for (RoleCategory category : it->second) {
            const std::vector<dpp::snowflake>* target_roles = nullptr;

            switch (category) {
            case RoleCategory::Admin:      target_roles = &admins; break;
            case RoleCategory::Anketolog:  target_roles = &anketologs; break;
            case RoleCategory::GameMaster: target_roles = &game_masters; break;
            }

            if (target_roles) {
                for (auto role_id : user_roles) {
                    if (std::find(target_roles->begin(), target_roles->end(), role_id) != target_roles->end()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    // Проверка доступа к категории модерации для меню помощи
    bool HasModerationAccess(const dpp::interaction_create_t& event) const {
        dpp::snowflake user_id = event.command.usr.id;

        if (std::find(blacklist.begin(), blacklist.end(), user_id) != blacklist.end()) {
            return false;
        }

        if (std::find(admin_users.begin(), admin_users.end(), user_id) != admin_users.end()) {
            return true;
        }

        if (event.command.app_permissions.has(dpp::p_administrator)) {
            return true;
        }

        const auto& user_roles = event.command.member.get_roles();

        auto has_any = [&](const std::vector<dpp::snowflake>& target_roles) {
            for (auto r_id : target_roles) {
                if (std::find(user_roles.begin(), user_roles.end(), r_id) != user_roles.end()) {
                    return true;
                }
            }
            return false;
            };

        return has_any(admins) || has_any(anketologs) || has_any(game_masters);
    }
};