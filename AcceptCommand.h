#pragma once
#include <dpp/dpp.h>

void RegisterAcceptCommand(dpp::cluster& bot);
void HandleAcceptCommand(dpp::cluster& bot, const dpp::slashcommand_t& event, dpp::snowflake archive_channel_id);