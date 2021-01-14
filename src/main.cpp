#include <WideBot.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <sstream>
#include <fstream>

#include <list>
#include <Magick++.h>

int main(int argv, char** argc) {
  std::stringstream buf;
  std::ifstream f("token.txt");
  if (!f) {
    std::cerr << "Error opening token file\n";
    return 1;
  }
  buf << f.rdbuf();
  f.close();
  std::string token = buf.str();
  token = token.substr(0, token.length()-1);

  Magick::InitializeMagick(nullptr);

  cURLpp::initialize();
  cURLpp::Easy curl;

  WideBot wideBot(token,
                  SleepyDiscord::USER_CONTROLED_THREADS,
                  &curl);
  wideBot.run();

  return 0;
}
