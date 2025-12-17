#pragma once

const int MAP_WIDTH = 100;
const int MAP_HEIGHT = 100;
const int EDITOR_MAX_X = 500;
const int EDITOR_MAX_Y = 500;
const int GAME_DURATION_SECONDS = 30;
const int INITIAL_NPC_COUNT = 50;
const int DICE_SIDES = 6;

struct MovementConfig {
    int move_distance;
    int kill_distance;
};

const MovementConfig BULL_CONFIG = {30, 10};
const MovementConfig FROG_CONFIG = {1, 10};
const MovementConfig DRAGON_CONFIG = {50, 30};