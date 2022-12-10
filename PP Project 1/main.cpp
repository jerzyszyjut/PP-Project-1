#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include"conio2.h"
#include<math.h>

#define BOARD_SIZE 13
#define BOARD_BEGGINING_X 50
#define BOARD_BEGGINING_Y 1
#define BOARD_GAP_SIZE_X 2
#define BOARD_GAP_SIZE_Y 1
#define BOARD_SIZE_LIMIT 20

#define TEXT_COLOR BLUE
#define BACKGROUND_COLOR DARKGRAY
#define BORDER_COLOR YELLOW
#define PLAYER_1_COLOR WHITE
#define PLAYER_2_COLOR BLACK

#define ARROW_DOWN 0x48
#define ARROW_UP 0x50
#define ARROW_LEFT 0x4b
#define ARROW_RIGHT 0x4d
#define ENTER 0xd
#define ESCAPE 0x1b

#define CURSOR 'X'

#define SAVE_FILE_NAME "game.bin"

typedef struct coordinates {
	int x;
	int y;
} coordinates_t;

typedef struct score {
	float white_player;
	float black_player;
} score_t;

typedef struct board {
	char **fields;
	int size;
} board_t;

typedef struct game {
	coordinates_t* cursor_position;
	board_t* board;
	board_t* previous_board;
	score_t* score;
	char current_player;
} game_t;

void draw_info(coordinates_t *start_coordinates, game_t *game) {
	char buffer[32];
	textcolor(TEXT_COLOR);
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("Jerzy, Szyjut, 193064");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("arrows  = cursor movement");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("q       = exit");
	gotoxy(start_coordinates->x, start_coordinates->y++);	
	cputs("n       = start new game");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("enter   = confirm choice and end player's turn");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("esc     = cancel current action");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("i       = place a stone on the board");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("s       = save the game state");
	gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("l       = load the game state");
	/*gotoxy(start_coordinates->x, start_coordinates->y++);
	cputs("f       = finish the game");*/
	gotoxy(start_coordinates->x, start_coordinates->y++);
	gotoxy(start_coordinates->x, start_coordinates->y++);
	printf("White player score: %.1f", game->score->white_player);
	gotoxy(start_coordinates->x, start_coordinates->y++);
	printf("Black player score: %.1f", game->score->black_player);
	gotoxy(start_coordinates->x, start_coordinates->y++);
	printf("Current coordinates: (%d, %d)", game->cursor_position->x, game->cursor_position->y);
}

void clear_screen() {
	textbackground(DARKGRAY);
	clrscr();
}

coordinates_t *get_cursor_position_from_board_position(coordinates_t *cursor_board_position) {
	coordinates_t* cursor_screen_position = new coordinates_t;
	cursor_screen_position->x = BOARD_GAP_SIZE_X * (cursor_board_position->x + 1) + BOARD_BEGGINING_X;
	cursor_screen_position->y = BOARD_GAP_SIZE_Y * (cursor_board_position->y + 1) + BOARD_BEGGINING_Y;
	return cursor_screen_position;
}

void draw_cursor(game_t *game, char current_player) {
	coordinates_t *cursor_position = get_cursor_position_from_board_position(game->cursor_position);
	gotoxy(cursor_position->x, cursor_position->y);
	if (current_player == 1) {
		textcolor(PLAYER_1_COLOR);
	}
	else if (current_player == -1) {
		textcolor(PLAYER_2_COLOR);
	}
	textbackground(BACKGROUND_COLOR);
	putch(CURSOR);
	delete cursor_position;
}

void handle_movement(coordinates_t *cursor_board_position, int board_size) {
	int zn;
	zn = getch();
	if (zn == ARROW_DOWN)
	{
		if (cursor_board_position->y > 0) {
			cursor_board_position->y--;
		}
	}
	else if (zn == ARROW_UP)
	{
		if (cursor_board_position->y < board_size - 1) {
			cursor_board_position->y++;
		}
	}
	else if (zn == ARROW_LEFT)
	{
		if (cursor_board_position->x > 0) {
			cursor_board_position->x--;
		}
	}
	else if (zn == ARROW_RIGHT) {
		if (cursor_board_position->x < board_size - 1) {
			cursor_board_position->x++;
		}
	};
}

void draw_board(board_t *board, coordinates_t start) {
	int displayed_size = board->size;
	coordinates_t board_limits;
	board_limits.x = (displayed_size+ 1)*BOARD_GAP_SIZE_X + start.x;
	board_limits.y = (displayed_size+ 1)*BOARD_GAP_SIZE_Y + start.y;
	coordinates_t board_coordinates;
	for (int y = start.y; y <= board_limits.y; y++) {
		for (int x = start.x; x <= board_limits.x; x++) {
			gotoxy(x, y);
			board_coordinates = { ((x - start.x - 1) / BOARD_GAP_SIZE_X), ((y - start.y - 1) / BOARD_GAP_SIZE_Y) };
			if (x == start.x || x == board_limits.x || y == start.y || y == board_limits.y ) {
				textcolor(BORDER_COLOR);
				putch('*');
			}
			else {
				if (((x - start.x) % BOARD_GAP_SIZE_X == 0) && ((y - start.y) % BOARD_GAP_SIZE_Y == 0)) {
					switch (board->fields[board_coordinates.x][board_coordinates.y])
					{
					case -1:
						textcolor(BLACK);
						putch('O');
						break;
					case 1:
						textcolor(WHITE);
						putch('O');
						break;
					default:
						textcolor(TEXT_COLOR);
						putch('+');
						break;
					}
					textcolor(TEXT_COLOR);
				}
				else if ((BOARD_GAP_SIZE_X > 1 && BOARD_GAP_SIZE_Y > 1) && (x - start.x) % BOARD_GAP_SIZE_X == 0) {
					putch('|');
				}
				else if ((BOARD_GAP_SIZE_X > 1 && BOARD_GAP_SIZE_Y > 1) && (y - start.y) % BOARD_GAP_SIZE_Y == 0) {
					putch('-');
				}
			}
		}
	}
}

board_t *initialize_board(int size) {
	board_t *board = new board_t;
	board->size = size;

	char** fields = new char*[board->size];
	for (int i = 0; i < board->size; i++) {
		fields[i] = new char[board->size];
	}

	for (int i = 0; i < board->size; i++) {
		for (int j = 0; j < board->size; j++) {
			fields[i][j] = 0;
		}
	}

	board->fields = fields;
	return board;
}

board_t* copy_board(board_t* board) {
	board_t* new_board = new board_t;
	new_board->size = board->size;

	char** fields = new char* [new_board->size];
	for (int i = 0; i < new_board->size; i++) {
		fields[i] = new char[new_board->size];
	}

	for (int i = 0; i < new_board->size; i++) {
		for (int j = 0; j < new_board->size; j++) {
			fields[i][j] = board->fields[i][j];
		}
	}

	new_board->fields = fields;
	return new_board;
}

int compare_boards(board_t* board_1, board_t* board_2) {
	for (int i = 0; i < board_1->size; i++) {
		for (int j = 0; j < board_1->size; j++) {
			if (board_1->fields[i][j] != board_2->fields[i][j]) {
				return 0;
			}
		}
	}
	return 1;
}

game_t *initialize_game(int board_size) {
	game_t *game = new game_t;
	game->cursor_position = new coordinates_t;
	game->cursor_position->x = 0;
	game->cursor_position->y = 0;
	game->score = new score_t;
	game->score->white_player = 0;
	game->score->black_player = 0;
	game->board = initialize_board(board_size);
	game->previous_board = NULL;
	game->current_player = 1;
	return game;
}

void free_board_memory(board_t* board) {
	if (board != NULL) {
		for (int i = 0; i < board->size; i++) {
			delete board->fields[i];
		}
		delete board->fields;
	}
}

void free_game_memory(game_t *game) {
	if (game != NULL) {
		free_board_memory(game->board);
		delete game->board;
		free_board_memory(game->previous_board);
		delete game->previous_board;
		delete game->score;
		delete game->cursor_position;
	}
}

int display_confirmation_dialog(char* message, coordinates_t* dialog_position) {
	gotoxy(dialog_position->x, dialog_position->y++);
	textcolor(RED);
	cputs(message);
	gotoxy(dialog_position->x, dialog_position->y++);
	cputs("ENTER - Yes");
	gotoxy(dialog_position->x, dialog_position->y++);
	cputs("ESCAPE - No");
	char answer = 0;
	do {
		answer = getch();
		if (answer == ENTER) {
			return 1;
		}
		if (answer == ESCAPE) {
			return 0;
		}
	} while ((answer != ENTER) && (answer != ESCAPE));
}

void display_info_dialog(char* message, coordinates_t* dialog_position) {
	gotoxy(dialog_position->x, dialog_position->y++);
	textcolor(RED);
	cputs(message);
	gotoxy(dialog_position->x, dialog_position->y++);
	cputs("Press any button to continue");
	getch();
}

game *start_new_game(game_t* game, coordinates_t* confirmation_position) {
	char message[] = "Do you want to start new game?";
	int confirmation = display_confirmation_dialog(message, confirmation_position);
	if (confirmation == 1) {
		int board_size = game->board->size;
		free_game_memory(game);
		game = initialize_game(board_size);
	}
	return game;
}

int _count_group_liberties(board_t* board, coordinates_t* starting_position) {
	int liberties = 0;
	char current_color = board->fields[starting_position->x][starting_position->y];
	if (current_color == 0) {
		return liberties;
	}
	if (current_color == 2 || current_color == 1) {
		*(&(board->fields[starting_position->x][starting_position->y])) = 2;
	}
	else if (current_color == -2 || current_color == -1) {
		*(&(board->fields[starting_position->x][starting_position->y])) = -2;
	}
	if (starting_position->x > 0) {
		if (board->fields[starting_position->x - 1][starting_position->y] == current_color) {
			coordinates_t new_starting_position = { starting_position->x - 1, starting_position->y };
			liberties += _count_group_liberties(board, &new_starting_position);
		}
		else if (board->fields[starting_position->x - 1][starting_position->y] == 0) {
			liberties += 1;
		}
	}
	if (starting_position->x < (board->size - 1)) {
		if (board->fields[starting_position->x + 1][starting_position->y] == current_color) {
			coordinates_t new_starting_position = { starting_position->x + 1, starting_position->y };
			liberties += _count_group_liberties(board, &new_starting_position);
		}
		else if (board->fields[starting_position->x + 1][starting_position->y] == 0) {
			liberties += 1;
		}
	}
	if (starting_position->y > 0) {
		if (board->fields[starting_position->x][starting_position->y - 1] == current_color) {
			coordinates_t new_starting_position = { starting_position->x, starting_position->y - 1 };
			liberties += _count_group_liberties(board, &new_starting_position);
		}
		else if (board->fields[starting_position->x][starting_position->y - 1] == 0) {
			liberties += 1;
		}
	}
	if (starting_position->y < (board->size - 1)) {
		if (board->fields[starting_position->x][starting_position->y + 1] == current_color) {
			coordinates_t new_starting_position = { starting_position->x, starting_position->y + 1 };
			liberties += _count_group_liberties(board, &new_starting_position);
		}
		else if (board->fields[starting_position->x][starting_position->y + 1] == 0) {
			liberties += 1;
		}
	}
	return liberties;
}

int count_group_liberties(board_t* board, coordinates_t* starting_position) {
	board_t* new_board = copy_board(board);
	int liberties = _count_group_liberties(new_board, starting_position);
	free_board_memory(new_board);
	delete new_board;
	return liberties;
}

int remove_group(board_t *board, coordinates_t *starting_position) {
	int removed_stones = 0;
	char current_color = board->fields[starting_position->x][starting_position->y];
	if (current_color == 0) {
		return 0;
	}
	*(&(board->fields[starting_position->x][starting_position->y])) = 0;
	removed_stones++;
	if (starting_position->x > 0) {
		if (board->fields[starting_position->x - 1][starting_position->y] == current_color) {
			coordinates_t new_starting_position = { starting_position->x - 1, starting_position->y };
			removed_stones += remove_group(board, &new_starting_position);
		}
	}
	if (starting_position->x < (board->size - 1)) {
		if (board->fields[starting_position->x + 1][starting_position->y] == current_color) {
			coordinates_t new_starting_position = { starting_position->x + 1, starting_position->y };
			removed_stones += remove_group(board, &new_starting_position);
		}
	}
	if (starting_position->y > 0) {
		if (board->fields[starting_position->x][starting_position->y - 1] == current_color) {
			coordinates_t new_starting_position = { starting_position->x, starting_position->y - 1 };
			removed_stones += remove_group(board, &new_starting_position);
		}
	}
	if (starting_position->y < (board->size - 1)) {
		if (board->fields[starting_position->x][starting_position->y + 1] == current_color) {
			coordinates_t new_starting_position = { starting_position->x, starting_position->y + 1 };
			removed_stones += remove_group(board, &new_starting_position);
		}
	}
	return removed_stones;
}

int get_liberties_after_placing_stone(game_t* game, coordinates_t *cursor_position) {
	board_t* new_board = copy_board(game->board);
	new_board->fields[game->cursor_position->x][game->cursor_position->y] = game->current_player;
	int liberties = _count_group_liberties(new_board, cursor_position);
	free_board_memory(new_board);
	delete new_board;
	return liberties;
}

int can_stone_be_placed_here(game_t *game) {
	int liberties = get_liberties_after_placing_stone(game, game->cursor_position);
	char other_player = game->current_player * -1;

	if (liberties > 0) {
		return 1;
	}
	else {
		if (game->cursor_position->x > 0) {
			if (game->board->fields[game->cursor_position->x - 1][game->cursor_position->y] == other_player) {
				coordinates_t neighbour_liberties_position = { game->cursor_position->x - 1 , game->cursor_position->y };
				int neighbour_liberties = get_liberties_after_placing_stone(game, &neighbour_liberties_position);
				if (neighbour_liberties == 0) {
					return 1;
				}
			}
		}
		if (game->cursor_position->x < (game->board->size - 1)) {
			if (game->board->fields[game->cursor_position->x + 1][game->cursor_position->y] == other_player) {
				coordinates_t neighbour_liberties_position = { game->cursor_position->x + 1 , game->cursor_position->y };
				int neighbour_liberties = get_liberties_after_placing_stone(game, &neighbour_liberties_position);
				if (neighbour_liberties == 0) {
					return 1;
				}
			}
		}
		if (game->cursor_position->y > 0) {
			if (game->board->fields[game->cursor_position->x][game->cursor_position->y - 1] == other_player) {
				coordinates_t neighbour_liberties_position = { game->cursor_position->x, game->cursor_position->y - 1 };
				int neighbour_liberties = get_liberties_after_placing_stone(game, &neighbour_liberties_position);
				if (neighbour_liberties == 0) {
					return 1;
				}
			}
		}
		if (game->cursor_position->y < (game->board->size - 1)) {
			if (game->board->fields[game->cursor_position->x][game->cursor_position->y + 1] == other_player) {
				coordinates_t neighbour_liberties_position = { game->cursor_position->x , game->cursor_position->y + 1 };
				int neighbour_liberties = get_liberties_after_placing_stone(game, &neighbour_liberties_position);
				if (neighbour_liberties == 0) {
					return 1;
				}
			}
		}
	}
	return 0;
}

void remove_neighbours_if_possible(coordinates_t *cursor_position, board_t *board, char current_player, score_t *score) {
	int score_points = 0;
	if (cursor_position->x > 0) {
		if (board->fields[cursor_position->x - 1][cursor_position->y] != current_player) {
			coordinates_t neighbour_liberties_position = { cursor_position->x - 1 , cursor_position->y };
			int neighbour_liberties = count_group_liberties(board, &neighbour_liberties_position);
			if (neighbour_liberties == 0) {
				score_points += remove_group(board, &neighbour_liberties_position);
			}
		}
	}
	if (cursor_position->x < (board->size - 1)) {
		if (board->fields[cursor_position->x + 1][cursor_position->y] != current_player) {
			coordinates_t neighbour_liberties_position = { cursor_position->x + 1 , cursor_position->y };
			int neighbour_liberties = count_group_liberties(board, &neighbour_liberties_position);
			if (neighbour_liberties == 0) {
				score_points += remove_group(board, &neighbour_liberties_position);
			}
		}
	}
	if (cursor_position->y > 0) {
		if (board->fields[cursor_position->x][cursor_position->y - 1] != current_player) {
			coordinates_t neighbour_liberties_position = { cursor_position->x, cursor_position->y - 1 };
			int neighbour_liberties = count_group_liberties(board, &neighbour_liberties_position);
			if (neighbour_liberties == 0) {
				score_points += remove_group(board, &neighbour_liberties_position);
			}
		}
	}
	if (cursor_position->y < (board->size - 1)) {
		if (board->fields[cursor_position->x][cursor_position->y + 1] != current_player) {
			coordinates_t neighbour_liberties_position = { cursor_position->x , cursor_position->y + 1 };
			int neighbour_liberties = count_group_liberties(board, &neighbour_liberties_position);
			if (neighbour_liberties == 0) {
				score_points += remove_group(board, &neighbour_liberties_position);
			}
		}
	}
	if (score != NULL) {
		if (current_player == 1) {
			score->white_player += (float)score_points;
		}
		else if (current_player == -1) {
			score->white_player += (float)score_points;
		}
	}
}

int check_ko_rule (game_t *game) {
	int result = 0;
	board_t* next_board = copy_board(game->board);
	next_board->fields[game->cursor_position->x][game->cursor_position->y] = game->current_player;
	remove_neighbours_if_possible(game->cursor_position, next_board, game->current_player, NULL);
	if (game->previous_board == NULL || compare_boards(next_board, game->previous_board) != 1) {
		result = 1;
	}
	free_board_memory(next_board);
	delete next_board;
	return result;
}

void place_stone(game_t *game, coordinates_t *dialog_position) {
	if (game->board->fields[game->cursor_position->x][game->cursor_position->y] == 0 && can_stone_be_placed_here(game) == 1 && check_ko_rule(game) == 1) {
			char message[] = "Do you want to place stone on this coordinates?";
			int confirmation = display_confirmation_dialog(message, dialog_position);
			if (confirmation == 1) {
				if (game->previous_board != NULL) {
					free_board_memory(game->previous_board);
				}
				game->previous_board = copy_board(game->board);
				game->board->fields[game->cursor_position->x][game->cursor_position->y] = game->current_player;
				remove_neighbours_if_possible(game->cursor_position, game->board, game->current_player, game->score);
				game->current_player = -1 * game->current_player;
			}
	}
	else {
		char message[] = "You cannot place stone here";
		display_info_dialog(message, dialog_position);
	}
}

void save_game_state(game_t* game) {
	FILE* file = fopen(SAVE_FILE_NAME, "wb");
	if (file != NULL) {
		fwrite(&(game->current_player), sizeof(game->current_player), 1, file);
		fwrite(&(game->cursor_position->x), sizeof(game->cursor_position->x), 1, file);
		fwrite(&(game->cursor_position->y), sizeof(game->cursor_position->x), 1, file);
		fwrite(&(game->score->white_player), sizeof(game->score->white_player), 1, file);
		fwrite(&(game->score->black_player), sizeof(game->score->black_player), 1, file);
		fwrite(&(game->board->size), sizeof(game->board->size), 1, file);
		for (int x = 0; x < game->board->size; x++) {
			for (int y = 0; y < game->board->size; y++) {
				fwrite(&(game->board->fields[x][y]), sizeof(char), 1, file);
			}
		}		
		if (game->previous_board != NULL) {
			fwrite(&(game->previous_board->size), sizeof(game->previous_board->size), 1, file);
			for (int x = 0; x < game->previous_board->size; x++) {
				for (int y = 0; y < game->previous_board->size; y++) {
					fwrite(&(game->previous_board->fields[x][y]), sizeof(char), 1, file);
				}
			}
		}
		else{
			int minus_one = -1;
			fwrite(&minus_one, sizeof(minus_one), 1, file);
		}
		fclose(file);
	}
}

game_t* load_game_state() {
	FILE* file = fopen(SAVE_FILE_NAME, "rb");
	if (file != NULL) {
		game_t* game = new game_t;
		fread(&(game->current_player), sizeof(game->current_player), 1, file);
		game->cursor_position = new coordinates_t;
		fread(&(game->cursor_position->x), sizeof(game->cursor_position->x), 1, file);
		fread(&(game->cursor_position->y), sizeof(game->cursor_position->x), 1, file);
		game->score = new score_t;
		fread(&(game->score->white_player), sizeof(game->score->white_player), 1, file);
		fread(&(game->score->black_player), sizeof(game->score->black_player), 1, file);
		game->board = new board_t;
		fread(&(game->board->size), sizeof(game->board->size), 1, file);
		game->board->fields = new char*[game->board->size];
		for (int x = 0; x < game->board->size; x++) {
			game->board->fields[x] = new char[game->board->size];
			for (int y = 0; y < game->board->size; y++) {
				fread(&(game->board->fields[x][y]), sizeof(char), 1, file);
			}
		}
		game->previous_board = new board_t;
		fread(&(game->previous_board->size), sizeof(game->previous_board->size), 1, file);
		if (game->previous_board->size != -1) {
			game->previous_board->fields = new char* [game->previous_board->size];
			for (int x = 0; x < game->previous_board->size; x++) {
				game->previous_board->fields[x] = new char[game->previous_board->size];
				for (int y = 0; y < game->previous_board->size; y++) {
					fread(&(game->previous_board->fields[x][y]), sizeof(char), 1, file);
				}
			}
		}
		else {
			delete game->previous_board;
			game->previous_board = NULL;
		}
		fclose(file);
		return game;
	}
	return NULL;
}

int main() {
	int zn = 0;
	coordinates_t cursor_board_position = { 0, 0 };
	game_t *game = initialize_game(BOARD_SIZE);

#ifndef __cplusplus
	Conio2_Init();
#endif

	settitle("Jerzy, Szyjut, 193064"); 
	_setcursortype(_NOCURSOR);
	do {
		clear_screen();
		
		coordinates_t info_start_coordinates = { 1, 1 };
		draw_info(&info_start_coordinates, game);

		draw_board(game->board, { BOARD_BEGGINING_X, BOARD_BEGGINING_Y });

		draw_cursor(game, game->current_player);
		
		switch (zn = getch())
		{
			case 0:
				handle_movement(game->cursor_position, game->board->size);
				break;
			case 'i':
				place_stone(game, &info_start_coordinates);
				break;
			case 'n':
				game = start_new_game(game, &info_start_coordinates);
			case 's':
				save_game_state(game);
				break;
			case 'l':
				game_t * new_game;
				new_game = load_game_state();
				if (new_game != NULL) {
					free_game_memory(game);
					game = new_game;
				}
				break;
			default:
				break;
		}
	} while (zn != 'q');

	_setcursortype(_NORMALCURSOR);
	free_game_memory(game);
	return 0;
}
