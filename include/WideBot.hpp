#pragma once

#include <sleepy_discord/sleepy_discord.h>
#include <curlpp/Easy.hpp>
#include <Magick++.h>

#include <list>
#include <string>
#include <vector>

class WideBot : public SleepyDiscord::DiscordClient {
public:
  WideBot(const std::string token, SleepyDiscord::Mode mode, cURLpp::Easy* curl)
  : SleepyDiscord::DiscordClient(token, mode), curl(curl) {}
  
  void onMessage(SleepyDiscord::Message message) override;

private:
  cURLpp::Easy* curl = nullptr;

  std::string wideCommand = "!wide";

  int parseNumSplits(std::string &message);
  std::vector<std::list<Magick::Image>> splitImage(const std::list<Magick::Image> &image, const int &num_splits);
};
