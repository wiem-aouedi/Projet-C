# Catch The Falling Items Game

## Description
"Catch The Falling Items" is a simple arcade game where the player controls a basket at the bottom of the screen to catch falling items. The goal is to catch positive items (hearts) to score points while avoiding negative items (bombs). The game ends when the player misses too many hearts or catches too many bombs.

The game features different menus (main and options), sound effects, and background music. Players can adjust the volume through the options menu, and there are buttons for replaying or exiting the game after a game over.

## Features
- **Catch items**: The player controls a basket to catch falling items.
- **Score system**: The player earns points for catching hearts and loses points for catching bombs.
- **Game Over**: The game ends if the player misses too many hearts (more than 10).
- **Victory Condition**: The player wins if they score 50 points.
- **Menus**: The game includes a start menu, options menu (for volume control), and a game over screen with options to replay or exit.
- **Sound Effects**: Sound effects for collecting items, game over, and background music.

## Requirements
- SDL2 library
- SDL2_ttf (for font rendering)
- SDL2_mixer (for audio)
- SDL2_image (for image handling)

### Installation
1. Clone the repository:

   ```bash
   git clone https://github.com/yourusername/catch-the-falling-items.git
   cd catch-the-falling-items
