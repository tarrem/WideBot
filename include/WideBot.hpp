#pragma once

#include <sleepy_discord/sleepy_discord.h>
#include <curlpp/Easy.hpp>

class WideBot : public SleepyDiscord::DiscordClient {
public:
  WideBot(const std::string token, SleepyDiscord::Mode mode, cURLpp::Easy* curl)
  : SleepyDiscord::DiscordClient(token, mode), curl(curl) {}
  
  void onMessage(SleepyDiscord::Message message) override;

private:
  cURLpp::Easy* curl = nullptr;
};
