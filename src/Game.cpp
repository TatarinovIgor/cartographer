#include "Game.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>

bool Game::init() {
    srand((unsigned)time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "The Cartographer of Unmade Places",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    if (!renderer.init(window)) {
        SDL_Log("Renderer init failed");
        return false;
    }

    if (!assets.init()) {
        SDL_Log("Asset manager init failed");
        return false;
    }

    tileMap.init();

    player.init(25 * Constants::TILE_SIZE, 19 * Constants::TILE_SIZE);

    camera.x = player.x - Constants::SCREEN_WIDTH / 2.0f;
    camera.y = player.y - Constants::SCREEN_HEIGHT / 2.0f;

    setupNPCs();

    state = GameState::INTRO;
    fadeAlpha = 1.0f;
    fadeTarget = 0.0f;
    fadeSpeed = 0.8f;

    return true;
}

void Game::run() {
    Uint64 lastTime = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();

    while (!input.shouldQuit()) {
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)(now - lastTime) / freq;
        lastTime = now;

        dt = std::min(dt, 0.05f);

        input.update();

        if (input.isKeyPressed(SDL_SCANCODE_ESCAPE) && state == GameState::EXPLORE &&
            !dialogueSystem.isActive() && !mappingSystem.active) {
            break;
        }

        update(dt);
        render();
    }
}

void Game::shutdown() {
    assets.shutdown();
    renderer.shutdown();
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::update(float dt) {
    gameTime += dt;
    stateTime += dt;

    // Fade logic
    if (fadeAlpha != fadeTarget) {
        float dir = (fadeTarget > fadeAlpha) ? 1.0f : -1.0f;
        fadeAlpha += dir * fadeSpeed * dt;
        fadeAlpha = std::clamp(fadeAlpha, 0.0f, 1.0f);

        if (fadingToState && dir > 0 && fadeAlpha >= 0.99f) {
            state = fadeNextState;
            stateTime = 0;
            fadingToState = false;
            fadeTarget = 0.0f;
        }
    }

    particles.update(dt);
    camera.updateShake(dt);

    switch (state) {
        case GameState::INTRO:   updateIntro(dt);   break;
        case GameState::EXPLORE: updateExplore(dt);  break;
        case GameState::MAPPING: updateMapping(dt);  break;
        case GameState::ENDING:  updateEnding(dt);   break;
    }
}

void Game::render() {
    renderer.clear({15, 12, 10, 255});

    switch (state) {
        case GameState::INTRO:   renderIntro();   break;
        case GameState::EXPLORE: renderExplore();  break;
        case GameState::MAPPING: renderMapping();  break;
        case GameState::ENDING:  renderEnding();   break;
    }

    renderFade();
    renderer.present();
}

// --- INTRO ---

std::vector<std::string> Game::getIntroTexts() const {
    return {
        "In a world where geography is alive, mountains migrate across plains,\n"
        "rivers forget their banks, and forests fall asleep and vanish\n"
        "without warning... maps are sacred.\n\n"
        "Cartographers are the most powerful people alive.",

        "Sable was once among them. She drew maps of extraordinary precision.\n"
        "But then she drew a city that didn't exist.\n\n"
        "Three days later, it materialized out of thin air.\n"
        "No one believed her. They exiled her for it.",

        "Now a blank white void is quietly eating the edges of the world.\n"
        "Towns vanish. Roads dissolve. The land forgets itself.\n\n"
        "The Cartographers' Guild has no choice.\n"
        "You can't fight what you can't draw.",

        "They send Sable in alone.\n\n"
        "Her mission: map the void before it swallows everything.\n"
        "Her tools: ink, parchment, and the strange gift\n"
        "of seeing places that don't exist yet.\n\n"
        "                    [Press Enter]"
    };
}

void Game::updateIntro(float dt) {
    auto texts = getIntroTexts();

    if (introPage < (int)texts.size()) {
        introCharTimer += dt;
        while (introCharTimer >= Constants::TYPEWRITER_SPEED * 0.8f &&
               introCharIndex < (int)texts[introPage].size()) {
            introCharTimer -= Constants::TYPEWRITER_SPEED * 0.8f;
            introDisplayed += texts[introPage][introCharIndex];
            introCharIndex++;
        }
    }

    if (input.isKeyPressed(SDL_SCANCODE_RETURN) || input.isKeyPressed(SDL_SCANCODE_SPACE)) {
        if (introCharIndex < (int)texts[introPage].size()) {
            introDisplayed = texts[introPage];
            introCharIndex = (int)texts[introPage].size();
        } else {
            introPage++;
            introCharIndex = 0;
            introCharTimer = 0;
            introDisplayed = "";

            if (introPage >= (int)texts.size()) {
                startFadeTo(GameState::EXPLORE, 1.2f);
            }
        }
    }
}

void Game::renderIntro() {
    renderer.clear({8, 6, 4, 255});

    TTF_Font* titleFont = assets.getFont("title");
    TTF_Font* bodyFont = assets.getFont("body");

    if (titleFont) {
        float titleAlpha = std::min(stateTime * 2.0f, 1.0f);
        Constants::Colors::Color titleColor = {
            210, 170, 60, (unsigned char)(255 * titleAlpha)
        };
        renderer.drawTextCentered("The Cartographer of Unmade Places", 80, titleColor, titleFont);
    }

    if (bodyFont && !introDisplayed.empty()) {
        renderer.drawTextWrapped(introDisplayed, 180, 200,
            Constants::SCREEN_WIDTH - 360, Constants::Colors::UI_TEXT, bodyFont);
    }

    TTF_Font* smallFont = assets.getFont("small");
    if (smallFont && introPage < (int)getIntroTexts().size()) {
        float blink = sin(gameTime * 2.0f) * 0.3f + 0.7f;
        Constants::Colors::Color hintColor = {180, 170, 150, (unsigned char)(180 * blink)};
        renderer.drawText("[Space/Enter] to continue",
            Constants::SCREEN_WIDTH / 2 - 90, Constants::SCREEN_HEIGHT - 50,
            hintColor, smallFont);
    }
}

// --- EXPLORE ---

void Game::updateExplore(float dt) {
    if (dialogueSystem.isActive()) {
        dialogueSystem.update(input, dt);
        return;
    }

    player.update(input, tileMap, dt);
    camera.follow(player.centerX(), player.centerY(), dt);

    for (auto& npc : npcs)
        npc.update(dt);

    voidSystem.update(tileMap, particles, camera, dt);

    // Ink motes around Sable
    inkMoteTimer += dt;
    if (inkMoteTimer > 0.5f) {
        inkMoteTimer = 0;
        particles.spawnInkMotes(player.centerX(), player.centerY(), 1);
    }

    // NPC interaction
    if (input.isKeyPressed(SDL_SCANCODE_E)) {
        NPC* nearest = getNearestInteractableNPC();
        if (nearest) {
            dialogueSystem.start(nearest->dialogue);
            if (!nearest->talked && nearest->inkReward > 0) {
                player.addInk(nearest->inkReward);
            }
            nearest->talked = true;
        }
    }

    // Enter mapping mode
    if (input.isKeyPressed(SDL_SCANCODE_M) || input.isKeyPressed(SDL_SCANCODE_TAB)) {
        mappingSystem.enter(player.tileX(), player.tileY());
        state = GameState::MAPPING;
        stateTime = 0;
    }

    // Check for ending trigger
    if (voidSystem.shouldTriggerEnding(tileMap, mappingSystem.tilesRevealed)) {
        startFadeTo(GameState::ENDING, 0.8f);
    }
}

void Game::renderExplore() {
    tileMap.render(renderer, camera, gameTime);

    for (auto& npc : npcs)
        npc.render(renderer, camera, gameTime, assets);

    player.render(renderer, camera, gameTime);

    particles.render(renderer, camera);

    // Interaction prompt
    NPC* nearest = getNearestInteractableNPC();
    TTF_Font* smallFont = assets.getFont("small");
    if (nearest && smallFont) {
        float sx = camera.screenX(nearest->centerX()) - 25;
        float sy = camera.screenY(nearest->y) - 35;
        renderer.drawText("[E] Talk", (int)sx, (int)sy,
            Constants::Colors::UI_HIGHLIGHT, smallFont);
    }

    renderUI();
    mappingSystem.renderMinimap(renderer, tileMap, player);
    dialogueSystem.render(renderer, assets);
}

// --- MAPPING ---

void Game::updateMapping(float dt) {
    mappingSystem.update(input, tileMap, player, particles, camera, dt);

    if (!mappingSystem.active) {
        state = GameState::EXPLORE;
        stateTime = 0;
    }

    if (voidSystem.shouldTriggerEnding(tileMap, mappingSystem.tilesRevealed)) {
        mappingSystem.exit();
        startFadeTo(GameState::ENDING, 0.8f);
    }
}

void Game::renderMapping() {
    tileMap.render(renderer, camera, gameTime);

    for (auto& npc : npcs)
        npc.render(renderer, camera, gameTime, assets);

    player.render(renderer, camera, gameTime);
    particles.render(renderer, camera);

    mappingSystem.render(renderer, camera, assets, gameTime);
    renderUI();
    mappingSystem.renderMinimap(renderer, tileMap, player);
}

// --- ENDING ---

void Game::updateEnding(float dt) {
    endingTimer += dt;

    if (endingTimer > 2.0f && endingPhase == 0)
        endingPhase = 1;
    if (endingTimer > 6.0f && endingPhase == 1)
        endingPhase = 2;
    if (endingTimer > 12.0f && endingPhase == 2)
        endingPhase = 3;
    if (endingTimer > 18.0f && endingPhase == 3)
        endingPhase = 4;

    if (endingPhase >= 4 && input.isKeyPressed(SDL_SCANCODE_RETURN)) {
        // Could restart or quit
    }
}

void Game::renderEnding() {
    renderer.clear({8, 6, 4, 255});

    TTF_Font* titleFont = assets.getFont("title");
    TTF_Font* bodyFont = assets.getFont("body");
    TTF_Font* smallFont = assets.getFont("small");

    float alpha = std::min(endingTimer * 0.5f, 1.0f);
    Constants::Colors::Color textColor = {220, 210, 195, (unsigned char)(255 * alpha)};

    if (endingPhase >= 1 && bodyFont) {
        renderer.drawTextWrapped(
            "The last void tile dissolves under your ink. For a moment,\n"
            "there is silence. The parchment trembles in your hands.",
            180, 120, Constants::SCREEN_WIDTH - 360, textColor, bodyFont);
    }

    if (endingPhase >= 2 && bodyFont) {
        float a2 = std::min((endingTimer - 6.0f) * 0.5f, 1.0f);
        Constants::Colors::Color c2 = {220, 210, 195, (unsigned char)(255 * a2)};
        renderer.drawTextWrapped(
            "Something stirs in the space you've drawn. Not a city this time.\n"
            "Not a mountain or a river. Something that has no name yet.\n\n"
            "It is vast. It is patient. And it knows your hand.",
            180, 250, Constants::SCREEN_WIDTH - 360, c2, bodyFont);
    }

    if (endingPhase >= 3 && bodyFont) {
        float a3 = std::min((endingTimer - 12.0f) * 0.5f, 1.0f);
        Constants::Colors::Color c3 = {210, 170, 60, (unsigned char)(255 * a3)};
        renderer.drawTextWrapped(
            "The void wasn't destroying the world.\n"
            "It was waiting to be drawn.\n\n"
            "And now Sable must decide: does she finish the drawing?\n"
            "Does she give form to something the world was wise\n"
            "to leave undrawn?",
            180, 390, Constants::SCREEN_WIDTH - 360, c3, bodyFont);
    }

    if (endingPhase >= 4) {
        float a4 = std::min((endingTimer - 18.0f) * 0.5f, 1.0f);
        if (titleFont) {
            Constants::Colors::Color titleC = {210, 170, 60, (unsigned char)(255 * a4)};
            renderer.drawTextCentered("To be continued...", 560, titleC, titleFont);
        }
        if (smallFont) {
            Constants::Colors::Color hintC = {180, 170, 150, (unsigned char)(150 * a4)};
            renderer.drawText("Thank you for playing the demo.",
                Constants::SCREEN_WIDTH / 2 - 120, 620, hintC, smallFont);
        }
    }
}

// --- UI ---

void Game::renderUI() {
    TTF_Font* uiFont = assets.getFont("ui");
    TTF_Font* smallFont = assets.getFont("small");
    if (!uiFont) return;

    // Ink meter
    int barX = 15, barY = 15, barW = 150, barH = 20;
    renderer.fillRectAbsolute(barX - 1, barY - 1, barW + 2, barH + 2, Constants::Colors::UI_BG);
    renderer.drawRectAbsolute(barX - 1, barY - 1, barW + 2, barH + 2,
        Constants::Colors::UI_HIGHLIGHT);

    float inkRatio = (float)player.ink / Constants::MAX_INK;
    int fillW = (int)(barW * inkRatio);
    Constants::Colors::Color inkFill = Constants::Colors::INK_DARK;
    inkFill.a = 220;
    renderer.fillRectAbsolute(barX, barY, fillW, barH, inkFill);

    if (smallFont) {
        std::string inkText = "Ink: " + std::to_string(player.ink) + "/" +
                              std::to_string(Constants::MAX_INK);
        renderer.drawText(inkText, barX + 5, barY + 2, Constants::Colors::UI_TEXT, smallFont);
    }

    // Controls hint (bottom left)
    if (smallFont && state == GameState::EXPLORE && !dialogueSystem.isActive()) {
        renderer.drawText("[WASD] Move  [E] Interact  [M] Map  [Esc] Quit",
            10, Constants::SCREEN_HEIGHT - 25, {150, 140, 130, 150}, smallFont);
    }
}

void Game::renderFade() {
    if (fadeAlpha > 0.01f) {
        Constants::Colors::Color fadeColor = {8, 6, 4, (unsigned char)(255 * fadeAlpha)};
        renderer.fillRectAbsolute(0, 0, Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT,
            fadeColor);
    }
}

void Game::startFadeTo(GameState next, float speed) {
    fadeTarget = 1.0f;
    fadeSpeed = speed;
    fadingToState = true;
    fadeNextState = next;
}

// --- NPC SETUP ---

void Game::setupNPCs() {
    // The Innkeeper - near the inn building (12,13)
    NPC innkeeper(14 * Constants::TILE_SIZE, 17.5f * Constants::TILE_SIZE,
                  "Maren (Innkeeper)", Constants::Colors::NPC_INNKEEPER);
    innkeeper.inkReward = 20;
    innkeeper.dialogue = {
        {"Maren", "Another day walking backward through this cursed town... "
         "Oh! A visitor? We don't get many of those. Not anymore.",
         Constants::Colors::NPC_INNKEEPER, {}, 1},

        {"Maren", "The void, you see. It crept in three moons ago. "
         "Started at the north edge. Ate old Harren's farm whole. "
         "Not a sound. Just... white.",
         Constants::Colors::NPC_INNKEEPER, {}, 2},

        {"Maren", "You're the cartographer they sent? The disgraced one? "
         "Ha! Disgraced or not, you're all we've got.",
         Constants::Colors::NPC_INNKEEPER,
         {{"Tell me about the void.", 3},
          {"Why does everyone walk backward?", 4}}, -1},

        {"Maren", "It's not like fog or snow. It's... nothing. "
         "Pure nothing. My maps go blank near it. "
         "Here -- take some of my ink. You'll need it more than I do.",
         Constants::Colors::NPC_INNKEEPER, {}, -1},

        {"Maren", "Oh, that. The town's been wrong since the void arrived. "
         "Tuesdays we walk backward. Wednesdays the well speaks. "
         "Thursdays... we don't talk about Thursdays.\n"
         "Take this ink. Map that void before it maps us.",
         Constants::Colors::NPC_INNKEEPER, {}, -1}
    };

    // The Lost Cartographer - near the workshop (30,14)
    NPC cartographer(31 * Constants::TILE_SIZE, 18.5f * Constants::TILE_SIZE,
                     "Edris (Cartographer)", Constants::Colors::NPC_CARTO);
    cartographer.inkReward = 30;
    cartographer.dialogue = {
        {"Edris", "Sable? Is that... it IS you. "
         "They told me you were exiled. They told me you were mad.",
         Constants::Colors::NPC_CARTO, {}, 1},

        {"Edris", "I was sent here six weeks ago to map the void's advance. "
         "My maps... they just show white. Empty parchment. "
         "I've never felt so useless.",
         Constants::Colors::NPC_CARTO,
         {{"Your maps can't see it?", 2},
          {"I can map it. I've always seen what others can't.", 3}}, -1},

        {"Edris", "No. None of ours can. The void doesn't exist yet, Sable. "
         "That's the trick. You can't map what isn't there.\n"
         "But you... you mapped a city before it existed. "
         "Maybe you can map this too.",
         Constants::Colors::NPC_CARTO, {}, 4},

        {"Edris", "I know you can. I always believed you about the city. "
         "I just... I was too afraid to say so.\n"
         "Take my ink reserves. All of them. You'll need every drop.",
         Constants::Colors::NPC_CARTO, {}, 4},

        {"Edris", "Press M or Tab when you're near the void. "
         "Enter mapping mode. Use your ink to draw what should be there.\n"
         "Trust your instincts, Sable. Your maps have never been wrong.",
         Constants::Colors::NPC_CARTO, {}, -1}
    };

    // The Child - near the town square
    NPC child(23 * Constants::TILE_SIZE, 21.5f * Constants::TILE_SIZE,
              "Lira (Child)", Constants::Colors::NPC_CHILD);
    child.inkReward = 10;
    child.dialogue = {
        {"Lira", "You can see it too, can't you? "
         "The shapes in the white?",
         Constants::Colors::NPC_CHILD, {}, 1},

        {"Lira", "Everyone says the void is empty. "
         "But I see things moving in there. "
         "Big things. Patient things.",
         Constants::Colors::NPC_CHILD,
         {{"What do you see?", 2},
          {"You should stay away from the void.", 3}}, -1},

        {"Lira", "Something that wants to be real. "
         "It's been waiting a very long time. "
         "It whispers sometimes. It says: 'draw me.'\n\n"
         "...I think it's lonely.",
         Constants::Colors::NPC_CHILD, {}, 4},

        {"Lira", "I can't stay away. It sings to me.\n"
         "Don't worry. It doesn't want to hurt us. "
         "It just wants someone to see it. "
         "To make it real.",
         Constants::Colors::NPC_CHILD, {}, 4},

        {"Lira", "Here. I found this ink by the void's edge. "
         "It was just... sitting there. Like a gift.\n"
         "I think it wants you to have it.",
         Constants::Colors::NPC_CHILD, {}, -1}
    };

    npcs.push_back(innkeeper);
    npcs.push_back(cartographer);
    npcs.push_back(child);
}

NPC* Game::getNearestInteractableNPC() {
    NPC* nearest = nullptr;
    float minDist = Constants::INTERACT_RANGE;

    for (auto& npc : npcs) {
        float dist = npc.distanceTo(player.centerX(), player.centerY());
        if (dist < minDist) {
            minDist = dist;
            nearest = &npc;
        }
    }
    return nearest;
}

void Game::setupIntroDialogue() {}
