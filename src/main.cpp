#include <iostream>

void print_scores(int player1_score, int player2_score)
{
    std::cout << "\nFinal score:\n";
    std::cout << "Player 1: " << player1_score << "\n";
    std::cout << "Player 2: " << player2_score << "\n";
}

int calculate_next_position(int position, int velocity)
{
    return position + velocity;
}

int main ()
{
    int player1_score{0};
    int player2_score{0};
    int screen_width{100}; // fixed boundary of right side of the screen
    int ball_x{80}; // horizontal position of ball
    int ball_velocity_x{-10}; // how much the the horizontal poisition changes on each update
    int steps{0};

    while (steps < 8)
    {
        ball_x = calculate_next_position(ball_x, ball_velocity_x);
        
        if (ball_x >= screen_width)
        {
            ball_velocity_x = -ball_velocity_x;
            player1_score += 1;
        }

        if (ball_x <= 0)
        {
            ball_velocity_x = -ball_velocity_x;
            player2_score += 1;
        }

        std::cout << "Ball position: " << ball_x << "\n";
        steps += 1;
    }

    print_scores(player1_score, player2_score);

    return 0;
}