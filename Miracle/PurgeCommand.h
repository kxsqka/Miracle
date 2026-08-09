#pragma once
#include <dpp/dpp.h>
#include <vector>

void RegisterPurgeCommand(dpp::cluster& bot);
void HandlePurgeCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake log_channel_id, const std::vector<dpp::snowflake>& blacklist);