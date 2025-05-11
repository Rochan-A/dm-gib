#include "gib/game.h"
#include "third_party/concise_args/ConciseArgs.h"

int main(int argc, char **argv) {
  bool fullscreen = false;
  bool debug = false;

  ConciseArgs parser(argc, argv);
  parser.add<bool>(fullscreen, "f", "fullscreen", "Launch in full screen");
  parser.add<bool>(debug, "d", "debug", "Enable debug window");

  parser.parse();

  gib::Game game("Test");

  game.Run();

  return 0;
}
