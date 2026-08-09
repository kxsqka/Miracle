#pragma once
#include <dpp/dpp.h>
#include <vector>

void RegisterAcceptCommand(dpp::cluster& bot);
void HandleAcceptCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake archive_channel_id, const std::vector<dpp::snowflake>& blacklist);