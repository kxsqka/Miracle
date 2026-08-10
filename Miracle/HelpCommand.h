#pragma once
#include <dpp/dpp.h>
#include "Permissions.h"

void RegisterHelpCommand(dpp::cluster& bot);
void HandleHelpCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, const PermissionManager& perm_manager);
void RegisterHelpHandlers(dpp::cluster& bot, const PermissionManager& perm_manager);