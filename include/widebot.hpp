#pragma once

#include <sleepy_discord/sleepy_discord.h>
#include <curlpp/Easy.hpp>
#include <Magick++.h>

#include <list>
#include <string>
#include <vector>

namespace widebot{
  
  const struct Command { 
    std::string HELP = "help";
    std::string PREFIX = "prefix";
    std::string WIDE = "wide";
  } commands;

  class WideBot : public SleepyDiscord::DiscordClient {
  public:
    WideBot(const std::string token, SleepyDiscord::Mode mode, cURLpp::Easy* curl)
    : SleepyDiscord::DiscordClient(token, mode), curl_(curl) {}
    
    void onMessage(SleepyDiscord::Message message) override;

    void prefix(const std::string& p) { prefix_ = p; }
    std::string prefix() { return prefix_; }

    std::string getFullCommand(std::string command) { return std::string(prefix_) + command; }

  private:

    cURLpp::Easy* curl_ = nullptr;
    std::string prefix_ = "!";

    // Seperates command from arguments, returns 1 any arguments were found
    int parseCommand(const std::string& msg, std::string* cmd, std::string* args);
    int parseNumSplits(const std::string& args);
    std::vector<std::list<Magick::Image>> splitImage(const std::list<Magick::Image> &image, const int &num_splits);
  };

} //namespace widebot
