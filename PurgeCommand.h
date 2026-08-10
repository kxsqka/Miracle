#pragma once
#include <dpp/dpp.h>

void RegisterPurgeCommand(dpp::cluster& bot);
void HandlePurgeCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake log_channel_id);