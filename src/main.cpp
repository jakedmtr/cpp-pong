#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm> // std::clamp
#include <cmath>     // std::hypot, std::sin, std::cos
#include <numbers>   // std::numbers::pi_v

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
    if (ball.rect.y <= 0.0f)
    {
        ball.rect.y = 0.0f; // reset position to prevent boundary penetration
        ball.v_y = -ball.v_y; // reverse vertical velocity
    }

    // Check floor
    if (ball.rect.y + ball.rect.h >= window_height)
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

        std::cout << "Player 1 scores! "
                  << player1_score << " - "
                  << player2_score << "\n";

        reset_ball(ball, window_width, window_height);
        ball.v_x = -ball.v_x; // reverse velocity
    }
    if (ball.rect.x + ball.rect.w <= 0) // left boundary
    {
        player2_score += 1;

        std::cout << "Player 2 scores! "
            << player1_score << " - "
            << player2_score << "\n";

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
    constexpr float paddle_speed = 360.0f;

    // Ball dimenions
    constexpr float ball_h = 20.0f;
    constexpr float ball_w = 20.0f;

    // Initial ball velocities in pixels per second
    constexpr float initial_ball_v_x = 360.0f;
    constexpr float initial_ball_v_y = 180.0f;

    // Initial ball position
    constexpr float initial_ball_x = (window_width - ball_w) / 2;
    constexpr float initial_ball_y = (window_height - ball_h) / 2;

    // Create ball and paddle objects
    Ball ball{{initial_ball_x, initial_ball_y, ball_w, ball_h}, initial_ball_v_x, initial_ball_v_y};
    Paddle player1_paddle{{player1_paddle_x, player1_paddle_y, paddle_width, paddle_height}};
    Paddle player2_paddle{{player2_paddle_x, player2_paddle_y, paddle_width, paddle_height}};
    
    // Timer
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 previous_counter = SDL_GetPerformanceCounter();

    bool running{true};

    while (running)
    {   
        // Obtain current counter
        const Uint64 current_counter = SDL_GetPerformanceCounter();

        // Delta t = elapsed ticks / ticks per second
        const double elapsed_seconds = static_cast<double>(current_counter - previous_counter) / 
        static_cast<double>(frequency);
        
        // Save counter for the next frame
        previous_counter = current_counter;

        const float delta_t = static_cast<float>(elapsed_seconds);

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
                    }
                    // Unpause the game if P is pressed again while paused
                    else if (game_state == GameState::Paused)
                    {
                        game_state = GameState::Playing;
                    }
                }

                // R: Restart match after game over
                else if (event.key.scancode == SDL_SCANCODE_R && game_state == GameState::GameOver)
                {
                    // Reset scores
                    player1_score = 0;
                    player2_score = 0;

                    // Centre ball
                    ball.rect.x = initial_ball_x;
                    ball.rect.y = initial_ball_y;

                    // Restore ball velocities
                    ball.v_x = initial_ball_v_x;
                    ball.v_y = initial_ball_v_y;

                    // Centre both paddles
                    player1_paddle.rect.x = player1_paddle_x; 
                    player1_paddle.rect.y = player1_paddle_y; 
                    player2_paddle.rect.x = player2_paddle_x; 
                    player2_paddle.rect.y = player2_paddle_y; 

                    // Return to Playing
                    game_state = GameState::Playing;

                    std::cout << "\nNEW GAME\n";
                }
            }
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

            // End game upon a player reaching the maximum score
            if (player1_score >= winning_score || player2_score >= winning_score)
            {   
                char winning_player;
                if (player1_score >= winning_score)
                {
                    winning_player = '1';
                }
                else
                {
                    winning_player = '2';
                }

                game_state = GameState::GameOver;

                std::cout << "\nGAME OVER\nPlayer " << winning_player << " wins!" << std::endl;
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

        // Present the completed frame
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    // Release resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
