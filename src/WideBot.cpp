#include <WideBot.hpp>

#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

#include <Magick++.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include <vector>

using namespace SleepyDiscord;
using namespace cURLpp;

void WideBot::onMessage(Message message) {
  if (message.startsWith("!hello")) {
    std::string response = "Hello, " + message.author.username + "!";
    sendMessage(message.channelID, response);
  }

  if (message.startsWith(wideCommand)) {
    if (message.attachments.empty()) {
      sendMessage(message.channelID, "Error: no attachment found");
      return;
    }

    int num_splits = parseNumSplits(message.content);
    if (num_splits < 1) {
      num_splits = 2;
      std::cout << "Defaulting to 2 splits\n";
    } else {
      std::cout << "Using " << num_splits << " splits.\n";
    }

    std::cout << "Recieved attachment: " << std::endl;
    for (const auto& f : message.attachments) {
      std::cout << '\t' << f.filename << " at " << f.url << std::endl;

      std::list<Magick::Image> image;

      std::stringstream out;
      try { 

        curl->setOpt(Options::Url(f.url));
        curl->setOpt(Options::WriteStream(&out));
        curl->perform();
      }
      catch (cURLpp::RuntimeError& e) {
        std::cerr << e.what() << std::endl;
        sendMessage(message.channelID, e.what()); // TODO make this friendly
      }
      catch (cURLpp::LogicError& e) {
        std::cerr << e.what() << std::endl;
        sendMessage(message.channelID, e.what()); // TODO make this friendly
      }
      out.seekg(0, std::ios::end);
      long size = out.tellg();
      out.seekg(0, std::ios::beg);
      char data[size];
      out.read(data, size);
      Magick::Blob blob(data, size);

      std::list<Magick::Image> raw_image;
      Magick::readImages(&raw_image, blob);
      Magick::coalesceImages(&image, raw_image.begin(), raw_image.end());

      auto splits = splitImage(image, num_splits);

      std::cout << "Writing and uploading... ";
      for (int i = 0; i < num_splits; i++) {
        std::string format = image.front().magick();
        for(int i = 0; i < format.length(); i++) {
          format[i] = std::tolower(format[i]);
        }
        std::string out_filename = "wide" + image.front().fileName() + std::to_string(i+1) + '.' + format;
        std::cout << "Writing " << out_filename << std::endl;
        Magick::writeImages(splits[i].begin(), splits[i].end(), out_filename);

        uploadFile(message.channelID, out_filename, "");
      }
      std::cout << "done.\n";
    }
  }
}

int WideBot::parseNumSplits(std::string& message) {
  int num_splits = 0;
  std::string msg = message.substr(wideCommand.length(), std::string::npos);
  if (msg[0] == ' ') { msg.erase(0, 1); }
  std::cout << "Message: " << msg << std::endl;
  try {
    num_splits = std::stoi(msg);
  }
  catch (std::invalid_argument &e) {
    std::cerr << "Unknown number of splits argument " << msg << std::endl;
    return 0;
  }
  return  num_splits;
}

std::vector<std::list<Magick::Image>> WideBot::splitImage(const std::list<Magick::Image> &image, const int &num_splits) {
  std::vector<std::list<Magick::Image>> splits;

  for(int i = 0; i < num_splits; i++) {
    splits.emplace_back(std::list<Magick::Image>());
  }

  std::cout << "Transforming image... ";
  for(auto& img : image) {
    Magick::Geometry sizeOrig = img.size();
    sizeOrig.aspect(true);
    size_t offset = 0;
    for(int i = 0; i < num_splits; i++) {
      Magick::Geometry size;
      size = sizeOrig;
      size.width(sizeOrig.width() / num_splits);
      size.xOff(offset);
      offset += size.width();

      Magick::Image split = img;
      split.crop(size);
      split.repage();
      split.resize(sizeOrig);
      splits[i].emplace_back(split);
    }
  }
  std::cout << "done.\n";

  return splits;
}
