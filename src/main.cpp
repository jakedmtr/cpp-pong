#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

struct Ball // define 'Ball' type
{
    SDL_FRect rect;
    float v_x; // how much the the horizontal position changes on each update
    float v_y; // how much the the vertical position changes on each update
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

void move_ball(Ball& ball)
{
    ball.rect.x += ball.v_x; // update the x-coordinate
    ball.rect.y += ball.v_y; // update the y-coordinate
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
    float window_height)
{
    if (keyboard[up_key]) // Move paddle upwards
    {
        paddle.rect.y -= paddle_speed;
    }
    if (keyboard[down_key]) // Move downwards
    {
        paddle.rect.y += paddle_speed;
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
        // Left paddle collision
        if (side == PaddleSide::Left && ball.v_x < 0.0f)
        {
            ball.v_x = -ball.v_x;
            ball.rect.x = paddle.rect.x + paddle.rect.w;           
        }
        // Right paddle collision
        else if (side == PaddleSide::Right && ball.v_x > 0.0f)
        {
            ball.v_x = -ball.v_x;
            ball.rect.x = paddle.rect.x - ball.rect.w;
        }
    }
}

int main(int, char*[])
{
    const int window_width{1024};
    const int window_height{768};
    int player1_score{0};
    int player2_score{0};

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

    // Speed of paddle
    const float paddle_speed = 5.0f;

    Ball ball{{200.0f, 150.0f, 20.0f, 20.0f}, 6.0f, 3.0f};
    Paddle player1_paddle{{paddle_margin, (window_height - paddle_height) / 2.0f, paddle_width, paddle_height}};
    Paddle player2_paddle{{window_width - (paddle_width + paddle_margin), (window_height - paddle_height) / 2.0f, paddle_width, paddle_height}};
   
    bool running{true};

    while (running)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }
        
        // Read the keyboard
        const bool* keyboard = SDL_GetKeyboardState(nullptr);
        
        // Left paddle control
        move_paddle(
            player1_paddle,
            keyboard,
            SDL_SCANCODE_W,
            SDL_SCANCODE_S,
            paddle_speed,
            window_height
        );

        // Right paddle control
        move_paddle(
            player2_paddle,
            keyboard,
            SDL_SCANCODE_UP,
            SDL_SCANCODE_DOWN,
            paddle_speed,
            window_height
        );

        move_ball(ball);
        
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
