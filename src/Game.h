#pragma once
#include <SDL.h>
#include "Renderer.h"
#include "Input.h"
#include "Camera.h"
#include "Player.h"
#include "TileMap.h"
#include "LivingMap.h"
#include "DialogueSystem.h"
#include "VoidSystem.h"
#include "NPC.h"
#include "ParticleSystem.h"
#include "AssetManager.h"
#include "EmotionSystem.h"
#include <vector>

enum class GameState {
    INTRO,
    EXPLORE,
    MAPPING,
    ENDING
};

class Game {
public:
    bool init();
    void run();
    void shutdown();

private:
    SDL_Window* window = nullptr;
    Renderer renderer;
    Input input;
    Camera camera;
    Player player;
    TileMap tileMap;
    LivingMap livingMap;
    DialogueSystem dialogueSystem;
    VoidSystem voidSystem;
    ParticleSystem particles;
    AssetManager assets;
    EmotionSystem emotions;
    std::vector<NPC> npcs;

    GameState state = GameState::INTRO;
    float gameTime = 0;
    float stateTime = 0;

    // Intro state
    int introPage = 0;
    float introCharTimer = 0;
    int introCharIndex = 0;
    std::string introDisplayed;

    // Ending state
    float endingTimer = 0;
    int endingPhase = 0;

    // Fade
    float fadeAlpha = 1.0f;
    float fadeTarget = 0.0f;
    float fadeSpeed = 1.0f;
    bool fadingToState = false;
    GameState fadeNextState = GameState::INTRO;

    // Ink mote timer
    float inkMoteTimer = 0;

    // Consequence notification
    float consequenceTimer = 0;
    std::string consequenceText;

    void update(float dt);
    void render();

    void updateIntro(float dt);
    void renderIntro();

    void updateExplore(float dt);
    void renderExplore();

    void updateMapping(float dt);
    void renderMapping();

    void updateEnding(float dt);
    void renderEnding();

    void renderUI();
    void renderFade();
    void renderConsequenceNotification();

    void startFadeTo(GameState next, float speed = 1.5f);
    void setupNPCs();

    std::vector<std::string> getIntroTexts() const;
    NPC* getNearestInteractableNPC();
};
