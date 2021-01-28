#include <widebot.hpp>

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

namespace widebot {

  void WideBot::onMessage(Message message) {
    if (message.startsWith(prefix_)) { 
      std::cout << "Recieved command: " << message.content << std::endl;
      std::string cmd, args;
      parseCommand(message.content, &cmd, &args);

      if(cmd == commands.HELP) {
        sendMessage(message.channelID, "Caption and image attachment with `" + prefix_ + "wide [num_splits]` to split and widen and image for emojis!");
      } else if (cmd == commands.EMOJI) {
        std::string image_data_uri = "data:image/jpg;base64,";
        Magick::Image image("test.jpg");
        Magick::Blob blob;
        image.write(&blob);
        image_data_uri += blob.base64();
        std::vector<Snowflake<Role>> roles;
        roles.push_back(Snowflake<Role>("801225009636442133"));
        createServerEmoji(message.serverID, "test", image_data_uri, roles);
      } else if (cmd == commands.PREFIX) {
        if (args.empty()) {
          prefix_ = "!";
          sendMessage(message.channelID, "Resetting prefix to `" + prefix_ + "`.");
        } else {
          prefix_ = args;
          sendMessage(message.channelID, "Setting the command prefix to `" + args + "`.");
        }
      } else if (cmd == commands.WIDE) {
        if (message.attachments.empty()) {
          sendMessage(message.channelID, "Error: no attachment found");
          return;
        }

        bool create_emojis = true;

        int num_splits = parseNumSplits(args);
        std::cout << "Using " << num_splits << " splits.\n";
        if (num_splits < 1) {
          num_splits = 2;
          std::cout << "Defaulting to 2 splits\n";
        }

        std::cout << "Recieved attachment: " << std::endl;
        for (const auto& f : message.attachments) {
          std::cout << '\t' << f.filename << " at " << f.url << std::endl;

          std::list<Magick::Image> image;

          std::stringstream out;
          try { 

            curl_->setOpt(Options::Url(f.url));
            curl_->setOpt(Options::WriteStream(&out));
            curl_->perform();
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

          std::cout << "Writing and uploading... \n";
          for (int i = 0; i < num_splits; i++) {
            std::string name, format;
            for (size_t i = f.filename.length(); i > 0; i--) {
              if (f.filename[i] == '.') {
                name = f.filename.substr(0, i);
                format = f.filename.substr(i, std::string::npos);
                break;
              }
            }
            std::string out_name = "wide" + name + std::to_string(i+1);
            std::string out_filename = out_name + format;
            std::cout << "Writing " << out_filename << std::endl;
            Magick::writeImages(splits[i].begin(), splits[i].end(), out_filename);

            if (create_emojis) {
              std::vector<Snowflake<Role>> roles; // role have yet to be implemented in sleepy_discord
//            roles.push_back(Snowflake<Role>("801225009636442133"));
              if (!createEmoji(message.serverID, out_name, splits[i], roles)) {
                create_emojis = false;
              }
            }

            uploadFile(message.channelID, out_filename, "");
          }
        }
      }
    }
  }

  int WideBot::parseNumSplits(const std::string& args) {
    int num_splits = 0;
    // assumes the number of splits is the first integer argument
    for(size_t i = 0; i < args.length(); i++) {
      try {
        num_splits = std::stoi(args.substr(i));
        return num_splits;
      }
      catch (std::invalid_argument &e) {
        std::cerr << "Unable to parse splits argument\n";
      }
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

  int WideBot::parseCommand(const std::string& msg, std::string* cmd, std::string* args) {
    for(size_t i = prefix_.length(); i < msg.length(); i++) {
      if(msg[i] == ' ') { 
        cmd->assign(msg.substr(prefix_.length(), i - prefix_.length()));
        args->assign(msg.substr(i + 1));
        return 1;
      }
    }
    cmd->assign(msg.substr(prefix_.length()));
    return 0;
  }

  int WideBot::createEmoji(Snowflake<Server> serverID, const std::string &name, const std::list<Magick::Image> &image, std::vector<Snowflake<Role>> roles) {
    std::string image_data_uri = "data:image/";
    std::string format = image.front().magick();
    if (format == "JPEG") {
      image_data_uri += "jpeg";
    } else if (format  == "PNG") {
      image_data_uri += "png";
    } else if (format == "GIF") {
      image_data_uri += "gif";
    } else {
      return false;
    }
    image_data_uri += ";base64,";
    for (auto img : image) {
      Magick::Blob blob;
      img.write(&blob);
      image_data_uri += blob.base64();
    }
    try{
      createServerEmoji(serverID, name, image_data_uri, roles);
    }
    catch(std::exception& e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
    return true;
  }

} //namespace widebot
