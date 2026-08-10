#pragma once
#include <dpp/dpp.h>

void RegisterShipCommand(dpp::cluster& bot);
void HandleShipCommand(dpp::cluster& bot, const dpp::slashcommand_t& event);