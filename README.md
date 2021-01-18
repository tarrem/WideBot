# WideBot

Discord bot to create images in the style of wide variations of Twitch emotes such as [peepoHappy]() [widepeepoHappy]().

## Building

Requirements:
- cURLpp
- ImageMagick

1. Create a bot and obtain the bot token at https://discord.com/developers.

2. Place token in token.txt

3. Build and run the bot
```
git clone --recursive https://github.com/tarrem/WideBot.git
mkdir build && cd build
cmake ..
cmake --build .
./WideBot
```

### Build notes:
Make sure the ImageMagick library and headers are the same version, otherwise it causes linking issues.
See https://yourwaifu.dev/sleepy-discord/setup-standard-cli.html#common-errors for errors with Sleepy Discord.

## Usage

Attach and caption and image or gif with `!wide [num_splits]` to stretch and split the images

## To-Do
- [ ] Re-optimize gifs transformation
- [ ] Option to automatically upload guild emoji
- [ ] Option to stretch without splitting
- [ ] Delete cached images
- [ ] Custom command prefix

