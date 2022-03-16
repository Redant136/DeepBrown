/**
 * Chess bot by Antoine Chevalier
 * */

#include "chess.h"

// object containing all the sprites
kNgine::SpriteMap *spriteMap;
// the main chess board
board *chessBoard;
// the engine pieces
std::vector<EngineChessPiece *> enginePieces;

using namespace Book_Builder;

bool playerColor = true;
phys_player *player;
bot_player *bot;

// UI to pick color and number of seconds per move
class OptionPicker : public kNgine::LayerRenderer
{
public:
  // the color the user chose
  bool colorChosen = playerColor;
  // the characters the user typed
  std::vector<char> charTyped = std::vector<char>();
  // the time per input
  f32 timePassed = 0;
  // setup as UI layer
  OptionPicker()
  {
    this->layer = layerO(kNgine::renderingLayerOrder, UI);
  }
  // shade the background, display current color and display text
  void render()
  {
    // shade background
    // set draw color to transparent black and draw over the whole window
    kRenderer_setDrawColor(v4(0, 0, 0, 0.5));
    kRenderer_drawRectV4(v4(-1, -1, 2, 2));

    // set color to white and display text
    kRenderer_setDrawColor(v4(1, 1, 1, 1));
    kRenderer_displayText(v3(-0.6, 0.8, 0), v3(0, 0, 0),
                          "Pick the starting color and the number of", 0.5);
    kRenderer_displayText(v3(-0.6, 0.7, 0), v3(0, 0, 0),
                          "seconds per move the bot will have.", 0.5);
    kRenderer_displayText(v3(-0.4, 0.6, 0), v3(0, 0, 0),
                          "Press enter when ready.", 0.5);

    // display the chose color using sprites of the kings and shading the one that was not chose
    kRenderer_setDrawColor(v4(1, 1, 1, 0.4));
    if (!colorChosen)
    {
      kRenderer_drawStoredTexture_defaultShader(spriteMap->texIndex[pieceType::K], v3(-0.3, -0.1, 0), 0.3, 0.3, v3(0, 0, 0));
    }
    else
    {
      kRenderer_drawStoredTexture_defaultShader(spriteMap->texIndex[pieceType::K * 2 + 1], v3(0, -0.1, 0), 0.3, 0.3, v3(0, 0, 0));
    }
    kRenderer_setDrawColor(v4(1, 1, 1, 1));
    if (colorChosen)
    {
      kRenderer_drawStoredTexture_defaultShader(spriteMap->texIndex[pieceType::K], v3(-0.3, -0.1, 0), 0.3, 0.3, v3(0, 0, 0));
    }
    else
    {
      kRenderer_drawStoredTexture_defaultShader(spriteMap->texIndex[pieceType::K * 2 + 1], v3(0, -0.1, 0), 0.3, 0.3, v3(0, 0, 0));
    }

    // if text was entered, display it
    if (charTyped.size() > 0)
    {
      char *typed = new char[charTyped.size() + 1];
      memcpy(typed, charTyped.data(), sizeof(char) * charTyped.size());
      // most strings in c end with a \0 characters
      typed[charTyped.size()] = '\0';
      kRenderer_displayText(v3(-0.3, -0.3, 0), v3(0, 0, 0),
                            typed, 0.5);
      delete[] typed;
    }
  }
  void update(std::vector<kNgine::msg> msgs)
  {
    v2 cursorPos = v2(0, 0);
    char keyTyped = 0;
    bool del = false;
    bool submit = false;
    // go throught engine messages to get cursor position, keys and time elapsed
    for (u32 i = 0; i < msgs.size(); i++)
    {
      if (msgs[i].msgType == kNgine::msg::CURSOR)
      {
        cursorPos = msgs[i].cursorPos;
      }
      else if (msgs[i].msgType == kNgine::msg::ASCII_KEY)
      {
        for (char c = '0'; c <= '9'; c++)
        {
          if (msgs[i].key & KeyBitmap(CharToKey(c)))
          {
            keyTyped = c;
          }
        }
        if (msgs[i].key & KeyBitmap(KEY_ENTER))
        {
          submit = true;
        }
        else if (msgs[i].key & KeyBitmap(KEY_BACKSPACE))
        {
          del = true;
        }
      }
      else if (msgs[i].msgType == kNgine::msg::NONASCII_KEY)
      {
        if (msgs[i].key & KeyBitmap(MOUSE_LEFT))
        {
          colorChosen = cursorPos.x < 0;
        } 
      }
      else if (msgs[i].msgType == kNgine::msg::TIME_ELAPSED)
      {
        timePassed += msgs[i].time;
      }
    }

    // if user typed and a set time limit was elapsed to not double key strokes that last more than one frame
    if (keyTyped != 0 && timePassed > 0.3)
    {
      charTyped.push_back(keyTyped);
      timePassed = 0;
    }
    // if user entered backspace
    if (del && timePassed > 0.3)
    {
      if(charTyped.size()>0){
        charTyped.erase(charTyped.end()-1);
        timePassed = 0;
      }
    }
    // if user typed enter
    else if (submit)
    {
      kNgine::disableObject(this);
      player->color = colorChosen;
      bot->color = !colorChosen;
      if (charTyped.size() > 0)
      {
        char *typed = new char[charTyped.size() + 1];
        memcpy(typed, charTyped.data(), sizeof(char) * charTyped.size());
        typed[charTyped.size()] = '\0';
        bot->secPerMove = stoi(std::string(typed));
        delete[] typed;
      }
      kNgine::enableObject(player);
      kNgine::enableObject(bot);
    }
  }
};

int main(int argc, const char **argv)
{
  // initialize random value, if we don't we will always get the same random seed
  seedRandomNumberGenerator();

  kNgine::setupDefaultLayers();
  std::vector<chessPiece *> pieces = std::vector<chessPiece *>();
  // setup the window
  kNgine::window_name = "chess";
  kNgine::window_size = {600, 600};
  chessCam *cam = new chessCam(8, 600, 600);
  cam->fovType = kNgine::Camera::MIN_WH;
  cam->position = v3(3.5, 3.5, 0);
  kNgine::addObject(cam);

  { //setup spr map
    spriteMap = new kNgine::SpriteMap();
    for (int i = pieceType::P; i <= pieceType::K; i++)
    {
      spriteMap->list.push_back(kNgine::importSprite((std::string("chess_pieces/Chess_") + (char)tolower(getPieceCode((pieceType)i)) + "lt60.png").c_str()));
    }
    for (int i = pieceType::P; i <= pieceType::K; i++)
    {
      spriteMap->list.push_back(kNgine::importSprite((std::string("chess_pieces/Chess_") + (char)tolower(getPieceCode((pieceType)i)) + "dt60.png").c_str()));
    }
    kNgine::addObject(spriteMap);
  }
  chessBoard = new board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  { // pieces
    pieces = chessBoard->pieces;
    for (int i = 0; i < pieces.size(); i++)
    {
      enginePieces.push_back(new EngineChessPiece(pieces[i]));
      kNgine::addObject(enginePieces[i]);
    }
  }

  // Book_Builder::setupOpening(16);
  player = new phys_player(playerColor, chessBoard);
  bot = new bot_player(!playerColor, chessBoard);
  kNgine::addObject(player);
  kNgine::addObject(bot);
  kNgine::disableObject(player);
  kNgine::disableObject(bot);

  // audio player
  kNgine::AudioEngine *audio = new kNgine::AudioEngine();
  kNgine::addObject(audio);
  // event to play the sound
  kNgine::addEvent({"chess_tock", [audio](void *arg) -> void *
                    {
                      audio->play("chess_tock.wav", kNgine::audiofiletype::wav);
                      return NULL;
                    }});

  kNgine::addObject(new OptionPicker());

  kNgine::start();
  kNgine::cleanup();
  delete chessBoard;
  return 0;
}
