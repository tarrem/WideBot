#include <WideBot.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <Magick++.h>
#include <vector>
#include <fstream>
#include <iostream>

using namespace SleepyDiscord;
using namespace cURLpp;

void WideBot::onMessage(Message message) {
  if (message.startsWith("!hello")) {
    std::string response = "Hello, " + message.author.username + "!";
    sendMessage(message.channelID, response);
  }

  if (!message.attachments.empty()) {
    std::cout << "Recieved attachment: " << std::endl;
    for (const auto& f : message.attachments) {
      std::cout << '\t' << f.filename << " at " << f.url << std::endl;
   
      try { 
        std::ofstream out(f.filename, std::ios_base::binary | std::ios_base::out);

        curl->setOpt(Options::Url(f.url));
        curl->setOpt(Options::WriteStream(&out));
        curl->perform();
        out.close();
      }
      catch (cURLpp::RuntimeError& e) {
        std::cerr << e.what() << std::endl;
      }
      catch (cURLpp::LogicError& e) {
        std::cerr << e.what() << std::endl;
      }

      std::vector<Magick::Image> images;
      try {
        Magick::readImages(&images, f.filename);
      }
      catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
      }
      for (auto& image : images) {
        //TODO resize each frame
        
      }
      Magick::writeImages(images.begin(), images.end(), "test.gif");
    }
  }
}
