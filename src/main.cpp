#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm> // std::clamp
#include <cmath>     // std::hypot, std::sin, std::cos
#include <numbers>   // std::numbers::pi_v
#include <string>

// Three different game states
enum class GameState
{
    Playing,
    Paused,
    GameOver
};

struct Ball // define 'Ball' type
{
    SDL_FRect rect;
    float v_x; // how much the the horizontal position changes per second
    float v_y; // how much the the vertical position changes per second
};

struct Paddle // define 'Paddle' type
{
    SDL_FRect rect;
};

enum class PaddleSide
{
    Left,
    Right
};

void move_ball(Ball& ball, float delta_t)
{
    ball.rect.x += ball.v_x * delta_t; // update the x-coordinate
    ball.rect.y += ball.v_y * delta_t; // update the y-coordinate
}

void handle_wall_collisions(Ball& ball, int window_height)
{
    // Check ceiling
    if (ball.rect.y <= 0.0f && ball.v_y < 0.0f)
    {
        ball.rect.y = 0.0f; // reset position to prevent boundary penetration
        ball.v_y = -ball.v_y; // reverse vertical velocity
    }

    // Check floor
    if (ball.rect.y + ball.rect.h >= window_height && ball.v_y > 0.0f)
    {
        ball.rect.y = window_height - ball.rect.h;
        ball.v_y = -ball.v_y;
    }
}

void reset_ball(Ball& ball, int window_width, int window_height)
{
    // Restore the ball's position to the centre.
    ball.rect.x = (window_width - ball.rect.w) / 2;
    ball.rect.y = (window_height - ball.rect.h) / 2;
}

void handle_goals(
    Ball& ball,
    int window_width,
    int window_height,
    int& player1_score,
    int& player2_score)
{
    // Detect goals, update the score, and reset the ball
    if (ball.rect.x >= window_width) // right boundary
    {
        player1_score += 1;
        reset_ball(ball, window_width, window_height);
        ball.v_x = -ball.v_x; // reverse velocity
    }
    if (ball.rect.x + ball.rect.w <= 0) // left boundary
    {
        player2_score += 1;
        reset_ball(ball, window_width, window_height);
        ball.v_x = -ball.v_x;
    }
}

void move_paddle(
    Paddle& paddle,
    const bool* keyboard,
    SDL_Scancode up_key,
    SDL_Scancode down_key,
    float paddle_speed,
    float window_height,
    float delta_t)
{
    if (keyboard[up_key]) // Move paddle upwards
    {
        paddle.rect.y -= paddle_speed * delta_t;
    }
    if (keyboard[down_key]) // Move downwards
    {
        paddle.rect.y += paddle_speed * delta_t;
    }
    // Clamp paddle position to window boundaries
    if (paddle.rect.y <= 0.0f)
    {
        paddle.rect.y = 0.0f;
    }
    if (paddle.rect.y + paddle.rect.h >= window_height)
    {
        paddle.rect.y = window_height - paddle.rect.h;
    }
}

void handle_paddle_collisions(
        Ball& ball,
        const Paddle& paddle,
        PaddleSide side)
{
    if (SDL_HasRectIntersectionFloat(&ball.rect, &paddle.rect))
    {
        // Calculate center of paddle and ball
        float paddle_center = paddle.rect.y + paddle.rect.h / 2.0f;
        float ball_center = ball.rect.y + ball.rect.h / 2.0f;

        // Calculate difference. If negative then ball is above paddle center
        float center_diff = ball_center - paddle_center;
        
        // Normalise collision point so that paddle height is irrelevant
        float collision_offset = center_diff / (paddle.rect.h / 2.0f);

        // Maximum outgoing angle that we will accept
        constexpr float max_bounce_angle = std::numbers::pi_v<float> / 3.0f;

        // Calculate outgoing angle
        collision_offset = std::clamp(collision_offset, -1.0f, 1.0f); // limit offset to [-1,1]
        float angle = collision_offset * max_bounce_angle;

        // Calculate ball speed from x and y velocity vectors
        float speed = std::hypot(ball.v_x, ball.v_y);

        // Left paddle collision
        // The horizontal velocity changes sign depending on which paddle is hit
        if (side == PaddleSide::Left && ball.v_x < 0.0f)
        {
            // Get x and y velocity components for left paddle
            ball.v_x = speed * std::cos(angle);
            ball.v_y = speed * std::sin(angle);
            ball.rect.x = paddle.rect.x + paddle.rect.w;           
        }
        // Right paddle collision
        else if (side == PaddleSide::Right && ball.v_x > 0.0f)
        {
            // Get x and y velocity components for right paddle
            ball.v_x = -speed * std::cos(angle);
            ball.v_y = speed * std::sin(angle);
            ball.rect.x = paddle.rect.x - ball.rect.w;
        }
    }
}

int main(int, char*[])
{
    // Set current game state as playing
    GameState game_state = GameState::Playing;

    // Menu selection: 0 = first option, 1 = Quit Game
    int selected_menu_option = 0;

    // Set window dimenions
    const int window_width{1024};
    const int window_height{768};

    // Initialise both player scores to zero
    int player1_score{0};
    int player2_score{0};

    // The score for one player to reach in order for them to win the game
    constexpr int winning_score = 5;

    // Initialise SDL's video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL initialisation failed: "
                  << SDL_GetError() << "\n";
        return 1;
    }

    // Create our window
    SDL_Window* window = SDL_CreateWindow("Pong", window_width, window_height, 0);

    if (window == nullptr)
    {
        std::cerr << "Window creation failed: "
                  << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // Create a renderer for drawing into the window
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (renderer == nullptr)
    {
        std::cerr << "Renderer creation failed: "
                  << SDL_GetError() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "Window and renderer created successfully!\n";

    // Initialize SDL3_ttf
    if (!TTF_Init())
    {
        std::cerr << "SDL_ttf initialization failed: "
                  << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "Font library initialized successfully!\n";

    // Load font
    TTF_Font* font = TTF_OpenFont("assets/score.ttf", 48.0f);
    TTF_Font* paused_font = TTF_OpenFont("assets/score.ttf", 68.0f);

    // Check if font loads correctly
    if (font == nullptr || paused_font == nullptr)
    {
        std::cerr << "Failed to load font: "
                  << SDL_GetError() << '\n';

        if (font != nullptr)
        {
            TTF_CloseFont(font);
        }
        if (paused_font != nullptr)
        {
            TTF_CloseFont(paused_font);
        }
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    // Paddle dimensions
    constexpr float paddle_width = 16.0f;
    constexpr float paddle_height = 100.0f;
    constexpr float paddle_margin = 30.0f;

    // Left paddle initial position
    constexpr float player1_paddle_x = paddle_margin;
    constexpr float player1_paddle_y = (window_height - paddle_height) / 2.0f;

    // Right paddle initial position
    constexpr float player2_paddle_x = window_width - (paddle_width + paddle_margin);
    constexpr float player2_paddle_y = (window_height - paddle_height) / 2.0f;

    // Speed of paddle in pixels per second
    constexpr float paddle_speed = 450.0f;

    // Ball dimenions
    constexpr float ball_h = 20.0f;
    constexpr float ball_w = 20.0f;

    // Initial ball velocities in pixels per second
    constexpr float initial_ball_v_x = 550.0f;
    constexpr float initial_ball_v_y = 250.0f;

    // Initial ball position
    constexpr float initial_ball_x = (window_width - ball_w) / 2;
    constexpr float initial_ball_y = (window_height - ball_h) / 2;

    // Create ball and paddle objects
    Ball ball{{initial_ball_x, initial_ball_y, ball_w, ball_h}, initial_ball_v_x, initial_ball_v_y};
    Paddle player1_paddle{{player1_paddle_x, player1_paddle_y, paddle_width, paddle_height}};
    Paddle player2_paddle{{player2_paddle_x, player2_paddle_y, paddle_width, paddle_height}};
    
    // Define colour for rendered text
    SDL_Color white{255, 255, 255, 255};

    // Cached scores
    int displayed_player1_score = -1; // start at -1 to check the first scoreboard update occurs
    int displayed_player2_score = -1;

    // Scoreboard textures
    SDL_Texture* player1_score_texture = nullptr;
    SDL_Texture* player2_score_texture = nullptr;

    // Scoreboard destination rectangles
    SDL_FRect player1_score_rect{};
    SDL_FRect player2_score_rect{};

    // Shared menu textures and rectangles
    SDL_Texture* paused_texture = nullptr;
    SDL_Texture* resume_texture = nullptr;
    SDL_Texture* play_again_texture = nullptr;
    SDL_Texture* quit_texture = nullptr;
    SDL_FRect paused_rect{};
    SDL_FRect resume_rect{};
    SDL_FRect play_again_rect{};
    SDL_FRect quit_rect{};

    // Winner title is created when a match ends
    SDL_Texture* winner_texture = nullptr;
    SDL_FRect winner_rect{};

    // Pause surface
    SDL_Surface* paused_surface = TTF_RenderText_Blended(
        paused_font,
        "PAUSED",
        0,
        white
    );

    // Resume surface
    SDL_Surface* resume_surface = TTF_RenderText_Blended(
        font,
        "RESUME",
        0,
        white
    );

    // Play Again surface
    SDL_Surface* play_again_surface = TTF_RenderText_Blended(
        font,
        "PLAY AGAIN",
        0,
        white
    );

    // Quit surface
    SDL_Surface* quit_surface = TTF_RenderText_Blended(
        font,
        "QUIT GAME",
        0,
        white
    );

    if (paused_surface != nullptr)
    {
        // Record dimensions before destroying the surface
        float paused_width = static_cast<float>(paused_surface->w);
        float paused_height = static_cast<float>(paused_surface->h);  

        // Create a NEW texture
        SDL_Texture* paused_texture_new =
        SDL_CreateTextureFromSurface(renderer, paused_surface);

        // We no longer need the temporary surface
        SDL_DestroySurface(paused_surface);

        if (paused_texture_new != nullptr)
        {
            paused_texture = paused_texture_new;

            paused_rect = {
                (window_width - paused_width) / 2.0f,
                window_height * 0.15f,
                paused_width,
                paused_height
            };
        }
        else
        {
            std::cerr << "Failed to create pause texture: "
                    << SDL_GetError() << '\n';
        }
    }

    if (resume_surface != nullptr)
    {
        float width = static_cast<float>(resume_surface->w);
        float height = static_cast<float>(resume_surface->h);

        resume_texture = SDL_CreateTextureFromSurface(
            renderer,
            resume_surface
        );

        SDL_DestroySurface(resume_surface);

        if (resume_texture != nullptr)
        {
            resume_rect = {
                (window_width - width) / 2.0f,
                window_height * 0.45f,
                width,
                height
            };
        }
        else
        {
            std::cerr << "Failed to create resume texture: "
                    << SDL_GetError() << '\n';
        }
    }
    else
    {
        std::cerr << "Failed to render resume text: "
                << SDL_GetError() << '\n';
    }

    if (play_again_surface != nullptr)
    {
        const float width = static_cast<float>(play_again_surface->w);
        const float height = static_cast<float>(play_again_surface->h);

        play_again_texture = SDL_CreateTextureFromSurface(
            renderer,
            play_again_surface
        );
        SDL_DestroySurface(play_again_surface);

        if (play_again_texture != nullptr)
        {
            play_again_rect = {
                (window_width - width) / 2.0f,
                window_height * 0.45f,
                width,
                height
            };
        }
        else
        {
            std::cerr << "Failed to create Play Again texture: "
                      << SDL_GetError() << '\n';
        }
    }
    else
    {
        std::cerr << "Failed to render Play Again text: "
                  << SDL_GetError() << '\n';
    }

    if (quit_surface != nullptr)
    {
        float width = static_cast<float>(quit_surface->w);
        float height = static_cast<float>(quit_surface->h);

        quit_texture = SDL_CreateTextureFromSurface(
            renderer,
            quit_surface
        );

        SDL_DestroySurface(quit_surface);

        if (quit_texture != nullptr)
        {
            quit_rect = {
                (window_width - width) / 2.0f,
                window_height * 0.55f,
                width,
                height
            };
        }
        else
        {
            std::cerr << "Failed to create quit texture: "
                    << SDL_GetError() << '\n';
        }
    }
    else
    {
        std::cerr << "Failed to render quit text: "
                << SDL_GetError() << '\n';
    }

    // Small pause instruction text
    SDL_Texture* pause_hint_texture = nullptr;
    SDL_FRect pause_hint_rect{};

    SDL_Surface* pause_hint_surface = TTF_RenderText_Blended(
        font,
        "PRESS P TO PAUSE",
        0,
        white
    );

    if (pause_hint_surface != nullptr)
    {
        constexpr float scale = 0.4f;

        float width = pause_hint_surface->w * scale;
        float height = pause_hint_surface->h * scale;

        pause_hint_texture = SDL_CreateTextureFromSurface(
            renderer,
            pause_hint_surface
        );

        SDL_DestroySurface(pause_hint_surface);

        pause_hint_rect = {
            (window_width - width) / 2.0f,
            8.0f,
            width,
            height
        };
    }
    else
    {
        std::cerr << "Failed to create pause hint: "
                << SDL_GetError() << '\n';
    }

    // Timer
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 previous_counter = SDL_GetPerformanceCounter();

    bool running{true};

    // Shared reset for Play Again (and the existing R shortcut)
    auto start_new_match = [&]()
    {
        player1_score = 0;
        player2_score = 0;

        ball.rect.x = initial_ball_x;
        ball.rect.y = initial_ball_y;
        ball.v_x = initial_ball_v_x;
        ball.v_y = initial_ball_v_y;

        player1_paddle.rect.x = player1_paddle_x;
        player1_paddle.rect.y = player1_paddle_y;
        player2_paddle.rect.x = player2_paddle_x;
        player2_paddle.rect.y = player2_paddle_y;

        SDL_DestroyTexture(winner_texture);
        winner_texture = nullptr;
        winner_rect = {};
        selected_menu_option = 0;
        game_state = GameState::Playing;

        std::cout << "\nNEW GAME\n";
    };

    while (running)
    {   
        // Define maximum delta_t to prevent large time intervals from producing unwanted movement
        constexpr float max_delta_t = 0.05f; // 50 ms

        // Obtain current counter
        const Uint64 current_counter = SDL_GetPerformanceCounter();

        // Delta t = elapsed ticks / ticks per second
        const double elapsed_seconds = static_cast<double>(current_counter - previous_counter) / 
        static_cast<double>(frequency);
        
        // Save counter for the next frame
        previous_counter = current_counter;

        // Take the minimum of actual frame time and max allowed delta_t, so we never exceed max
        const float delta_t = std::min(static_cast<float>(elapsed_seconds), max_delta_t);

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
            {
                // P: Pause or unpause the game (depending on the current game state)
                if (event.key.scancode == SDL_SCANCODE_P)
                {
                    // Pause the game if P the key is pressed
                    if (game_state == GameState::Playing)
                    {
                        game_state = GameState::Paused;
                        selected_menu_option = 0;
                    }
                    // Unpause the game if P is pressed again while paused
                    else if (game_state == GameState::Paused)
                    {
                        game_state = GameState::Playing;
                    }
                }

                // Change selection down in either menu
                else if (event.key.scancode == SDL_SCANCODE_DOWN &&
                        (game_state == GameState::Paused || game_state == GameState::GameOver))
                {
                    selected_menu_option = std::clamp(selected_menu_option + 1, 0, 1);
                }

                // Change selection up in either menu
                else if (event.key.scancode == SDL_SCANCODE_UP &&
                        (game_state == GameState::Paused || game_state == GameState::GameOver))
                {
                    selected_menu_option = std::clamp(selected_menu_option - 1, 0, 1);
                }

                // Enter: activate the current menu's selected option
                else if (event.key.scancode == SDL_SCANCODE_RETURN &&
                        (game_state == GameState::Paused || game_state == GameState::GameOver))
                {
                    if (selected_menu_option == 1)
                    {
                        // Both menus share Quit Game
                        running = false;
                    }
                    else if (game_state == GameState::Paused)
                    {
                        // Resume the paused match
                        game_state = GameState::Playing;
                    }
                    else
                    {
                        // Play Again from the Game Over menu
                        start_new_match();
                    }
                }

                // Keep R as a shortcut for starting a new match after game over
                else if (event.key.scancode == SDL_SCANCODE_R &&
                         game_state == GameState::GameOver)
                {
                    start_new_match();
                }
            }
        }

        if (!running)
        {
            break;
        }
        
        if (game_state == GameState::Playing)
        {
            // Read the keyboard
            const bool* keyboard = SDL_GetKeyboardState(nullptr);

            // Left paddle control
            move_paddle(
                player1_paddle,
                keyboard,
                SDL_SCANCODE_W,
                SDL_SCANCODE_S,
                paddle_speed,
                window_height,
                delta_t
            );

            // Right paddle control
            move_paddle(
                player2_paddle,
                keyboard,
                SDL_SCANCODE_UP,
                SDL_SCANCODE_DOWN,
                paddle_speed,
                window_height,
                delta_t
            );

            move_ball(ball, delta_t);
            
            // Check for paddle collisions
            handle_paddle_collisions(ball, player1_paddle, PaddleSide::Left);
            handle_paddle_collisions(ball, player2_paddle, PaddleSide::Right);

            // Check for ceiling and floor collisions
            handle_wall_collisions(ball, window_height);
            
            // Score if ball passes one of the boundaries
            handle_goals(
                ball,
                window_width,
                window_height,
                player1_score,
                player2_score
            );

            // End the match and create the winner title only once
            if (player1_score >= winning_score || player2_score >= winning_score)
            {
                const bool player1_won = (player1_score >= winning_score);
                const char* winner_text = player1_won ? "PLAYER 1 WINS" : "PLAYER 2 WINS";

                SDL_Surface* winner_surface = TTF_RenderText_Blended(
                    paused_font,
                    winner_text,
                    0,
                    white
                );
                if (winner_surface != nullptr)
                {
                    const float width = static_cast<float>(winner_surface->w);
                    const float height = static_cast<float>(winner_surface->h);

                    winner_texture = SDL_CreateTextureFromSurface(renderer, winner_surface);
                    SDL_DestroySurface(winner_surface);

                    if (winner_texture != nullptr)
                    {
                        winner_rect = {
                            (window_width - width) / 2.0f,
                            window_height * 0.15f,
                            width,
                            height
                        };
                    }
                    else
                    {
                        std::cerr << "Failed to create winner texture: "
                                  << SDL_GetError() << '\n';
                    }
                }
                else
                {
                    std::cerr << "Failed to render winner text: "
                              << SDL_GetError() << '\n';
                }

                selected_menu_option = 0; // Play Again is the default selection
                game_state = GameState::GameOver;
                std::cout << "\nGAME OVER\n" << winner_text << '\n';
            }
        }

    if (player1_score != displayed_player1_score)
    {
        // Convert the current score into text
        std::string player1_score_text = std::to_string(player1_score);

        // Create a new surface containing the updated score
        SDL_Surface* surface1 = TTF_RenderText_Blended(
            font,
            player1_score_text.c_str(),
            0,
            white
        );

        if (surface1 != nullptr)
        {
            // Record dimensions before destroying the surface
            float width = static_cast<float>(surface1->w);
            float height = static_cast<float>(surface1->h);

            // Create a NEW texture
            SDL_Texture* new_texture =
            SDL_CreateTextureFromSurface(renderer, surface1);

            // We no longer need the temporary surface
            SDL_DestroySurface(surface1);

            if (new_texture != nullptr)
            {
                // Destroy the previous score texture
                SDL_DestroyTexture(player1_score_texture);

                // Store the new texture
                player1_score_texture = new_texture;

                // Update the rectangle for rendering
                player1_score_rect = {
                    window_width * 0.25f - width / 2.0f,
                    40.0f,
                    width,
                    height
                };

                // Record which score the texture now represents
                displayed_player1_score = player1_score;
            }
        }
    }

    if (player2_score != displayed_player2_score)
    {
        // Convert the current score into text
        std::string player2_score_text = std::to_string(player2_score);

        // Create a new surface containing the updated score
        SDL_Surface* surface2 = TTF_RenderText_Blended(
            font,
            player2_score_text.c_str(),
            0,
            white
        );

        if (surface2 != nullptr)
        {
            // Record dimensions before destroying the surface
            float width = static_cast<float>(surface2->w);
            float height = static_cast<float>(surface2->h);

            // Create a NEW texture
            SDL_Texture* new_texture2 =
            SDL_CreateTextureFromSurface(renderer, surface2);

            // We no longer need the temporary surface
            SDL_DestroySurface(surface2);

            if (new_texture2 != nullptr)
            {
                // Destroy the previous score texture
                SDL_DestroyTexture(player2_score_texture);

                // Store the new texture
                player2_score_texture = new_texture2;

                // Update the rectangle for rendering
                player2_score_rect = {
                    window_width * 0.75f - width / 2.0f,
                    40.0f,
                    width,
                    height
                };

                // Record which score the texture now represents
                displayed_player2_score = player2_score;
            }
        }
    }

        // Set the drawing colour
        SDL_SetRenderDrawColor(renderer, 20, 100, 90, 255);

        // Fill the window with the current drawing colour
        SDL_RenderClear(renderer);

        // Draw a white rectangle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &ball.rect);

        // Draw player 1 paddle
        SDL_RenderFillRect(renderer, &player1_paddle.rect);

        // Draw player 2 paddle
        SDL_RenderFillRect(renderer, &player2_paddle.rect);

        // Render scoreboard textures
        if (player1_score_texture != nullptr)
        {
            SDL_RenderTexture(
                renderer,
                player1_score_texture,
                nullptr,
                &player1_score_rect
            );
        }
        if (player2_score_texture != nullptr)
        {
            SDL_RenderTexture(
                renderer,
                player2_score_texture,
                nullptr,
                &player2_score_rect
            );
        }

        // Render pause menu
        if (game_state == GameState::Paused && paused_texture != nullptr)
        {
            SDL_RenderTexture(
                renderer,
                paused_texture,
                nullptr,
                &paused_rect
            );
        }

        // Render 'resume' menu option
        if (game_state == GameState::Paused && resume_texture != nullptr)
        {
            // Highlight menu selection
            if (selected_menu_option == 0)
            {
                SDL_SetTextureColorMod(resume_texture, 255, 220, 100);
            }
            else
            {
                SDL_SetTextureColorMod(resume_texture, 255, 255, 255);
            }
            
            SDL_RenderTexture(
                renderer,
                resume_texture,
                nullptr,
                &resume_rect
            );
        }
        
        // Render the Game Over title and Play Again option
        if (game_state == GameState::GameOver)
        {
            if (winner_texture != nullptr)
            {
                SDL_RenderTexture(renderer, winner_texture, nullptr, &winner_rect);
            }

            if (play_again_texture != nullptr)
            {
                if (selected_menu_option == 0)
                {
                    SDL_SetTextureColorMod(play_again_texture, 255, 220, 100);
                }
                else
                {
                    SDL_SetTextureColorMod(play_again_texture, 255, 255, 255);
                }

                SDL_RenderTexture(renderer, play_again_texture, nullptr, &play_again_rect);
            }
        }

        // Quit Game appears in both the Pause and Game Over menus
        if ((game_state == GameState::Paused || game_state == GameState::GameOver) &&
            quit_texture != nullptr)
        {
            // Highlight menu selection
            if (selected_menu_option == 1)
            {
                SDL_SetTextureColorMod(quit_texture, 255, 220, 100);
            }
            else
            {
                SDL_SetTextureColorMod(quit_texture, 255, 255, 255);
            }

            SDL_RenderTexture(
                renderer,
                quit_texture,
                nullptr,
                &quit_rect
            );
        }

        // Display pause instructions during gameplay
        if (game_state == GameState::Playing &&
            pause_hint_texture != nullptr)
        {
            SDL_RenderTexture(
                renderer,
                pause_hint_texture,
                nullptr,
                &pause_hint_rect
            );
        }

        // Present the completed frame
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    // Release resources
    if (player1_score_texture != nullptr)
    {
        SDL_DestroyTexture(player1_score_texture);
    }
    if (player2_score_texture != nullptr)
    {
        SDL_DestroyTexture(player2_score_texture);
    }
    if (paused_texture != nullptr)
    {
        SDL_DestroyTexture(paused_texture);
    }
    if (resume_texture != nullptr)
    {
        SDL_DestroyTexture(resume_texture);
    }
    if (play_again_texture != nullptr)
    {
        SDL_DestroyTexture(play_again_texture);
    }
    if (quit_texture != nullptr)
    {
        SDL_DestroyTexture(quit_texture);
    }
    if (winner_texture != nullptr)
    {
        SDL_DestroyTexture(winner_texture);
    }
    if (pause_hint_texture != nullptr)
    {
        SDL_DestroyTexture(pause_hint_texture);
    }
    TTF_CloseFont(font);
    TTF_CloseFont(paused_font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
