#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

typedef struct single_game {
    int score;
    unsigned short rows[4];
    int lost;
    int won;
    int number_moves;
} game_t;

float expectedScore(game_t *, int, int);



short moveRight[65536];
short moveLeft[65536];
short moveDown[65536];
short moveUp[65536];
float rowHeuristics[65536];


short * moveList[4] = {moveRight, moveLeft, moveDown, moveUp};


#define RIGHT 0
#define LEFT 1
#define DOWN 2
#define UP 3
#define MAX_DEPTH 3



unsigned short mask_arr[4] = {0b1111000000000000, 0b0000111100000000, 0b0000000011110000, 0b0000000000001111};



void initialize_game(game_t * game){
    game -> won = 0;
    game -> lost = 0;
    game -> score = 0;
    game -> number_moves = 0;
    unsigned short arr[4] = {0,0,0,0};
    memcpy(game -> rows, arr, 4 * sizeof(short));
}



void initialize_move_list(char * filename, short * arr){
    FILE* file = fopen(filename, "r");
    char temp[16];
    for(int i = 0; i < 65536; i++){
        fscanf(file, "%s", temp);
        short val = atoi(temp);
        arr[i] = val;
    }
    fclose(file);
}

int place_random_tile(game_t * game){
    unsigned int tile_val;
    double random = (double) rand() / (double) RAND_MAX;
    if(random > 0.9){
        tile_val = 2;
    } else {
        tile_val = 1;
    }

    int open_tiles[16];
    int count = 0;
    for(int i = 0; i < 16; i++){
         unsigned int row = i / 4;
         unsigned  int col = i % 4;
         if((game ->rows[row] & mask_arr[col]) == 0){
             open_tiles[count] = i;
             count += 1;
         }
    }
    if(count == 0){
        return 0;
    }
    unsigned int random_val = open_tiles[rand() % count];
    game -> rows[random_val /4] = (game -> rows[random_val / 4] | tile_val << (12 - (4 * (random_val % 4))));

    return 1;


}


int get_next_move(game_t * game){


    int best_move = -1;
    float best_score = 0;
    for(int i = 0; i < 4; i ++){
        int valid_move = 0;
        float temp_score = 0;
        game_t new_game;
        initialize_game(&new_game);
        memcpy(new_game.rows, game -> rows, 4 * sizeof(unsigned short));
        if(i < 2) {
            for (int j = 0; j < 4; j++) {
                if(new_game.rows[j] != moveList[i][new_game.rows[j]]) {
                    new_game.rows[j] = moveList[i][new_game.rows[j]];
                    valid_move = 1;
                }
                temp_score += rowHeuristics[new_game.rows[j]];
            }
            temp_score = expectedScore(&new_game, 0, MAX_DEPTH);
        } else {

            unsigned short cols[4] = {0,0,0,0};
            cols[0] = (new_game.rows[0] & mask_arr[0]) | ((new_game.rows[1] & mask_arr[0]) >> 4) | ((new_game.rows[2] & mask_arr[0]) >> 8) | ((new_game.rows[3] & mask_arr[0]) >> 12);
            cols[1] = ((new_game.rows[0] & mask_arr[1]) << 4) | (new_game.rows[1] & mask_arr[1]) | ((new_game.rows[2] & mask_arr[1]) >> 4) | ((new_game.rows[3] & mask_arr[1]) >> 8);
            cols[2] = ((new_game.rows[0] & mask_arr[2]) << 8) | ((new_game.rows[1] & mask_arr[2]) << 4) | (new_game.rows[2] & mask_arr[2])  | ((new_game.rows[3] & mask_arr[2]) >> 4);
            cols[3] = ((new_game.rows[0] & mask_arr[3]) << 12) | ((new_game.rows[1] & mask_arr[3]) << 8) | ((new_game.rows[2] & mask_arr[3]) <<4) | (new_game.rows[3] & mask_arr[3]);


            for (int j = 0; j < 4; j++) {
                if(cols[j] != moveList[i][cols[j]]) {
                    cols[j] = moveList[i][cols[j]];
                    valid_move = 1;
                }
            }
            new_game.rows[0] = (cols[0] & mask_arr[0]) | ((cols[1] & mask_arr[0]) >> 4) | ((cols[2] & mask_arr[0]) >> 8) | ((cols[3] & mask_arr[0]) >> 12);
            new_game.rows[1] = ((cols[0] & mask_arr[1]) << 4) | (cols[1] & mask_arr[1]) | ((cols[2] & mask_arr[1]) >> 4) | ((cols[3] & mask_arr[1]) >> 8);
            new_game.rows[2] = ((cols[0] & mask_arr[2]) << 8) | ((cols[1] & mask_arr[2]) << 4) | (cols[2] & mask_arr[2])  | ((cols[3] & mask_arr[2]) >> 4);
            new_game.rows[3] = ((cols[0] & mask_arr[3]) << 12) | ((cols[1] & mask_arr[3]) << 8) | ((cols[2] & mask_arr[3]) <<4) | (cols[3] & mask_arr[3]);
            temp_score = expectedScore(&new_game, 0, MAX_DEPTH);

        }
        if((temp_score > best_score) && valid_move){
            best_score = temp_score;
            best_move = i;
        }
    }
    return best_move;
}



float iterate_moves(game_t * game, int current_depth, int max_depth){
    float best_score = 0;
    for(int i = 0; i < 4; i ++){
        int valid_move = 0;
        float temp_score = 0;
        game_t new_game;
        initialize_game(&new_game);
        memcpy(new_game.rows, game -> rows, 4 * sizeof(unsigned short));
        if(i < 2) {
            for (int j = 0; j < 4; j++) {
                if(new_game.rows[j] != moveList[i][new_game.rows[j]]) {
                    new_game.rows[j] = moveList[i][new_game.rows[j]];
                    valid_move = 1;
                }
                temp_score += rowHeuristics[new_game.rows[j]];
            }
            temp_score = expectedScore(&new_game, current_depth + 1,max_depth);
        } else {

            unsigned short cols[4] = {0,0,0,0};
            cols[0] = (new_game.rows[0] & mask_arr[0]) | ((new_game.rows[1] & mask_arr[0]) >> 4) | ((new_game.rows[2] & mask_arr[0]) >> 8) | ((new_game.rows[3] & mask_arr[0]) >> 12);
            cols[1] = ((new_game.rows[0] & mask_arr[1]) << 4) | (new_game.rows[1] & mask_arr[1]) | ((new_game.rows[2] & mask_arr[1]) >> 4) | ((new_game.rows[3] & mask_arr[1]) >> 8);
            cols[2] = ((new_game.rows[0] & mask_arr[2]) << 8) | ((new_game.rows[1] & mask_arr[2]) << 4) | (new_game.rows[2] & mask_arr[2])  | ((new_game.rows[3] & mask_arr[2]) >> 4);
            cols[3] = ((new_game.rows[0] & mask_arr[3]) << 12) | ((new_game.rows[1] & mask_arr[3]) << 8) | ((new_game.rows[2] & mask_arr[3]) <<4) | (new_game.rows[3] & mask_arr[3]);


            for (int j = 0; j < 4; j++) {
                if(cols[j] != moveList[i][cols[j]]) {
                    cols[j] = moveList[i][cols[j]];
                    valid_move = 1;
                }
            }
            new_game.rows[0] = (cols[0] & mask_arr[0]) | ((cols[1] & mask_arr[0]) >> 4) | ((cols[2] & mask_arr[0]) >> 8) | ((cols[3] & mask_arr[0]) >> 12);
            new_game.rows[1] = ((cols[0] & mask_arr[1]) << 4) | (cols[1] & mask_arr[1]) | ((cols[2] & mask_arr[1]) >> 4) | ((cols[3] & mask_arr[1]) >> 8);
            new_game.rows[2] = ((cols[0] & mask_arr[2]) << 8) | ((cols[1] & mask_arr[2]) << 4) | (cols[2] & mask_arr[2])  | ((cols[3] & mask_arr[2]) >> 4);
            new_game.rows[3] = ((cols[0] & mask_arr[3]) << 12) | ((cols[1] & mask_arr[3]) << 8) | ((cols[2] & mask_arr[3]) <<4) | (cols[3] & mask_arr[3]);
            temp_score = expectedScore(&new_game, current_depth + 1, max_depth);

        }
        if((temp_score > best_score) && valid_move){
            best_score = temp_score;

        }
    }
    return best_score;
}


float expectedScore(game_t * game, int current_depth, int max_depth){


    float score = 0;
    game_t new_game;
    initialize_game(&new_game);
    memcpy(new_game.rows, game -> rows, 4 * sizeof(unsigned short));

    int open_tiles[16];
    int count = 0;
    for(int i = 0; i < 16; i++){
        unsigned int row = i / 4;
        unsigned  int col = i % 4;
        if((game ->rows[row] & mask_arr[col]) == 0){
            open_tiles[count] = i;
            count += 1;
        }
    }


    for(int position = 0; position < count; position++) {
        game->rows[open_tiles[position] / 4] = (game->rows[open_tiles[position] / 4] | (2 << (12 - (4 * (open_tiles[position] % 4)))));
        if(current_depth >= max_depth) {
            for(int row_count = 0; row_count < 4; row_count++) {
                score += (0.9 / count) * rowHeuristics[game->rows[row_count]];
            }
        } else {
            score += (0.9 / count) * iterate_moves(game, current_depth, max_depth);

        }

        game->rows[open_tiles[position] / 4] = (game->rows[open_tiles[position] / 4] ^ (2 << (12 - (4 * (open_tiles[position] % 4)))));
        game->rows[open_tiles[position] / 4] = (game->rows[open_tiles[position] / 4] | (4 << (12 - (4 * (open_tiles[position] % 4)))));
        if(current_depth >= max_depth) {
            for(int row_count = 0; row_count < 4; row_count++) {
                score += (0.1 / count) * rowHeuristics[game->rows[row_count]];
            }
        }else {
            score += (0.9 / count) * iterate_moves(game, current_depth, max_depth);


        }
        game->rows[open_tiles[position] / 4] = (game->rows[open_tiles[position] / 4] ^ (4 << (12 - (4 * (open_tiles[position] % 4)))));

    }



    return score;

}





int main() {
    game_t game;
    initialize_game(&game);
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    srand(time(0));

    printf("%d\n", game.won);
    printf("%d %d %d %d\n", game.rows[0], game.rows[1], game.rows[2], game.rows[3]);
    initialize_move_list("MoveRight.txt", moveRight);
    initialize_move_list("MoveLeft.txt", moveLeft);
    initialize_move_list("MoveDown.txt", moveDown);
    initialize_move_list("MoveUp.txt", moveUp);
    initialize_move_list("CombinedDictionary.txt", rowHeuristics);

    for(int games = 0; games < 10; games++) {
        game_t game;
        initialize_game(&game);
        place_random_tile(&game);
        place_random_tile(&game);


        while (game.lost == 0) {

            int move = get_next_move(&game);


            if (move == -1) {
                game.lost = 1;
                break;

            }
            if (move < 2) {
                for (int r = 0; r < 4; r++) {
                    game.rows[r] = moveList[move][game.rows[r]];
                }
            } else {

                unsigned short cols[4] = {0, 0, 0, 0};
                cols[0] = (game.rows[0] & mask_arr[0]) | ((game.rows[1] & mask_arr[0]) >> 4) |
                          ((game.rows[2] & mask_arr[0]) >> 8) | ((game.rows[3] & mask_arr[0]) >> 12);
                cols[1] = ((game.rows[0] & mask_arr[1]) << 4) | (game.rows[1] & mask_arr[1]) |
                          ((game.rows[2] & mask_arr[1]) >> 4) | ((game.rows[3] & mask_arr[1]) >> 8);
                cols[2] = ((game.rows[0] & mask_arr[2]) << 8) | ((game.rows[1] & mask_arr[2]) << 4) |
                          (game.rows[2] & mask_arr[2]) | ((game.rows[3] & mask_arr[2]) >> 4);
                cols[3] = ((game.rows[0] & mask_arr[3]) << 12) | ((game.rows[1] & mask_arr[3]) << 8) |
                          ((game.rows[2] & mask_arr[3]) << 4) | (game.rows[3] & mask_arr[3]);


                for (int j = 0; j < 4; j++) {
                    cols[j] = moveList[move][cols[j]];
                }

                game.rows[0] =
                        (cols[0] & mask_arr[0]) | ((cols[1] & mask_arr[0]) >> 4) | ((cols[2] & mask_arr[0]) >> 8) |
                        ((cols[3] & mask_arr[0]) >> 12);
                game.rows[1] =
                        ((cols[0] & mask_arr[1]) << 4) | (cols[1] & mask_arr[1]) | ((cols[2] & mask_arr[1]) >> 4) |
                        ((cols[3] & mask_arr[1]) >> 8);
                game.rows[2] =
                        ((cols[0] & mask_arr[2]) << 8) | ((cols[1] & mask_arr[2]) << 4) | (cols[2] & mask_arr[2]) |
                        ((cols[3] & mask_arr[2]) >> 4);
                game.rows[3] = ((cols[0] & mask_arr[3]) << 12) | ((cols[1] & mask_arr[3]) << 8) |
                               ((cols[2] & mask_arr[3]) << 4) | (cols[3] & mask_arr[3]);


            }

            game.number_moves += 1;


            int new_tile = place_random_tile(&game);
            if (new_tile == 0) {
                game.lost = 0;
            }
            printf("%d\n", game.number_moves);


        }

        printf("%d\n", game.number_moves);

        for (int row = 0; row < 4; row++) {
            printf("%d\t%d\t%d\t%d\n", ((game.rows[row] & mask_arr[0]) >> 12), ((game.rows[row] & mask_arr[1]) >> 8),
                   ((game.rows[row] & mask_arr[2]) >> 4), (game.rows[row] & mask_arr[3]));
        }


    }





    return 0;
}





