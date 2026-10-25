#include <iostream>

struct Ball // define 'Ball' type
{
    int x; // horizontal position of ball
    int y; // vertical position of ball
    int v_x; // how much the the horizontal position changes on each update
    int v_y; // how much the the vertical position changes on each update
};

int calculate_next_position(int position, int velocity)
{
    return position + velocity;
}

void reverse_velocity(int& velocity)
{
    velocity = -velocity;
}

void move_ball(Ball& ball)
{
    ball.x = calculate_next_position(ball.x, ball.v_x); // update the x-coordinate
    ball.y = calculate_next_position(ball.y, ball.v_y); // update the y-coordinate
}

void handle_wall_collisions(Ball& ball, int screen_height)
{
    if (ball.y <= 0) // reverse vertical direction of ball if it reaches the top boundary
    { 
        reverse_velocity(ball.v_y);
    }

    if (ball.y >= screen_height) // reverse vertical direction of ball if it reaches the bottom boundary
    { 
        reverse_velocity(ball.v_y);
    }
}

void print_ball_position(const Ball& ball)
{
    std::cout << "Ball position: (" << ball.x << ", " << ball.y << ")\n";
}

void print_scores(int player1_score, int player2_score)
{
    std::cout << "\nFinal score:\n";
    std::cout << "Player 1: " << player1_score << "\n";
    std::cout << "Player 2: " << player2_score << "\n";
}

int main ()
{
    int player1_score{0};
    int player2_score{0};
    const int screen_width{100}; // fixed boundary of right side of the screen
    const int screen_height{50}; // fixed boundary of bottom side of the screen
    int steps{0}; // counter for while loop
    const int limit{10}; // number of steps for while loop

    Ball ball{20, 20, 5, 10}; // create ball object of type 'Ball'

    while (steps < limit)
    {
        move_ball(ball);
        handle_wall_collisions(ball, screen_height);

        if (ball.x >= screen_width) // reverse horizontal direction of ball if it reaches the right boundary
        {
            reverse_velocity(ball.v_x);
            player1_score += 1;
        }

        if (ball.x <= 0) // reverse horizontal direction of ball if it reaches the left boundary
        {
            reverse_velocity(ball.v_x);
            player2_score += 1;
        }

        print_ball_position(ball);

        steps += 1;
    }

    print_scores(player1_score, player2_score);

    return 0;
}