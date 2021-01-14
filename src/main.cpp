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

  std::string filename = "gif.gif";
  std::list<Magick::Image> images;
  std::cout << "Reading images " << filename << std::endl;
  try{
    Magick::readImages(&images, filename);
  }
  catch (std::exception & e) {
    std::cerr << e.what() << std::endl;
  }
  std::cout << "Opened " << filename << " with " << images.size() << " frames\n";
  Magick::writeImages(images.begin(), images.end(), "test.gif");
  std::cout << "Wrote gif to test.gif\n";

  WideBot wideBot(token,
                  SleepyDiscord::USER_CONTROLED_THREADS,
                  &curl);
  wideBot.run();

  return 0;
}
