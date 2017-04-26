
/*
									贪吃蛇
	这是我用C语言实现的。




日期：2016年12月23日。
*/

#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <conio.h>
#include <stdlib.h>
#include <process.h>
#include <Windows.h>

#define VERSION 1.0
#define AUTHOR_WORDS "用C语言实现的第一款游戏.虽然不是很好,但这是我的第一款游戏.值得留念!.\n我的联系方式: (1)QQ: 270638232\n谢谢!\n"

#define com_update system("cls")
#define com_pause system("pause")

#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

#define XSIZE_MAX 80
#define YSIZE_MAX 24

#define NULL_FRAME ' '
#define DEFAULT_FRAME '#'

#define DEFAULT_SNAKE_HEAD '@'
#define DEFAULT_SNAKE_FRAME '*'

#define DEFAULT_FOOD '$'

#define GAME_OVER "Game Over"

#define DEFAULT_SCORE 10

unsigned int XSIZE = 0;
unsigned int YSIZE = 0;

unsigned int score = 0;

struct Map
{
	char map[YSIZE_MAX][XSIZE_MAX];
	char frame;
};

// 食物结构体。
struct Food
{
	unsigned int x;
	unsigned int y;
	char food_ch;
	int state;
};

// 蛇结构体。使用链表实现“贪吃蛇”。
struct Snake
{
	unsigned int x; // “x”代表蛇的一节所在的x坐标。
	unsigned int y; // “y”代表蛇的一节所在的y坐标。
	char item_ch; // “item_ch”代表蛇的一节的字符。
	int direction;
	struct Snake *next; // “next”代表蛇的下一节。
};

// 游戏框架结构体。存放着“贪吃蛇”游戏的基本数据。
struct Game
{
	struct Snake snake; // 蛇结构体。
	struct Food food; // 食物结构体。
	struct Map map; // 地图结构体。
	int start; // 游戏状态。“0”代表游戏没有开始，非“0”代表游戏开始。
};

struct UserData
{
	char food_ch;
	char frame;
	int direction;
	unsigned int speed;
	int end;
};

size_t strCopy(char *dest, const char *src);
void gameStart(struct Game *game,char frame,char food); // 游戏初始化函数。
int gameOver(struct Game *game); // 结束游戏并销毁对象。然后输出分数并显示“Game Over”。
void updateMap(const struct Map *map);
void addSnakeItem(struct Snake *snake,char item_ch);
void destroySnake(struct Snake *snake);
void updateSnake(const struct Snake *snake,struct Map *map);
int moveSnake(struct Snake *snake, struct Map *map, struct Food *food, int direction);
int checkSnakeMove(const struct Snake *snake, const struct Map *map);
int updateFood(struct Food *food, struct Map *map);
unsigned int __stdcall autoGame(void *buf);
int usingSetting(struct UserData *ud, const char *setting_file);

int main(int argc, char *argv[])
{
	struct UserData d;
	int ch = 0;

	system("title 贪吃蛇");

	if (usingSetting(&d, "setting"))
	{
		puts("Setting Error");
		com_pause;
		return -1;
	}

	while (1)
	{
		puts("\n\n\n\n\n                                    * 贪 吃 蛇 *\n\n                    (1) 开始游戏  (2) 作者的话  (3) 退出游戏\n");

		ch = _getch();

		switch (ch)
		{
		case '1':
			com_update;

			d.direction = RIGHT;
			d.end = 0;

			if (!(HANDLE)_beginthreadex(NULL, 0, autoGame, &d, 0, NULL))
				return -1;

			while (!d.end)
			{
				ch = _getch();

				if (!isascii(ch))
					continue;

				d.direction = ch;
			}

			com_update;

			break;

		case '2':
			com_update;

			puts(AUTHOR_WORDS);

			com_pause;
			com_update;
			break;

		case '3':
			return 0;

		default:
			com_update;
			break;
		}
	}
}

void gameStart(struct Game *game, char frame,char food)
{
	game->snake.x = XSIZE / 2;
	game->snake.y = YSIZE / 2;
	game->snake.item_ch = DEFAULT_SNAKE_HEAD;
	game->snake.direction = RIGHT;
	game->snake.next = NULL;

	addSnakeItem(&game->snake, DEFAULT_SNAKE_FRAME);
	addSnakeItem(&game->snake, DEFAULT_SNAKE_FRAME);
	addSnakeItem(&game->snake, DEFAULT_SNAKE_FRAME);

	game->food.x = 0;
	game->food.y = 0;
	game->food.food_ch = food;
	game->food.state = 0;

	for (unsigned int i = 0; i != YSIZE; ++i)
	{
		if (i == 0 || i == YSIZE - 1)
		{
			char *p = game->map.map[i];

			for (; p != (game->map.map[i]) + XSIZE - 1; ++p)
			{
				*p = frame;
			}

			*p = '\0';

			continue;
		}

		for (unsigned int j = 0; j != XSIZE - 1; ++j)
		{
			if (j >= 1 && j < XSIZE - 2)
			{
				game->map.map[i][j] = NULL_FRAME;
				continue;
			}

			game->map.map[i][j] = frame;
		}

		game->map.map[i][XSIZE - 1] = '\0';
	}

	game->map.frame = frame;
	
	score = 0;
	game->start = 1;

	updateSnake(&game->snake, &game->map);
}

int gameOver(struct Game *game)
{
	int x = XSIZE / 2 - 5;
	char *p = &(game->map.map[YSIZE / 2][x]);

	game->start = 0;
	game->food.state = 0;

	destroySnake(&game->snake);

	strCopy(p, GAME_OVER);

	updateMap(&game->map);

	return 0;
}

size_t strCopy(char *dest, const char *src)
{
	char *ret = dest;

	while(*src != '\0')
	{
		*dest++ = *src++;
	}

	return dest - ret;
}

void updateMap(const struct Map *map)
{
	com_update;
	for (unsigned int i = 0; i != YSIZE; ++i)
		printf("%s\n",map->map[i]);

	printf("Score: %u", score);
}

void addSnakeItem(struct Snake *snake, char item_ch)
{
	struct Snake *p = (struct Snake*)malloc(sizeof(struct Snake));
	p->item_ch = item_ch;
	p->next = NULL;

	struct Snake *pre = snake;
	struct Snake *cur = snake->next;

	while (cur)
	{
		pre = cur;
		cur = cur->next;
	}

	switch (pre->direction)
	{
	case UP:
		p->direction = UP;
		p->x = pre->x;
		p->y = pre->y + 1;
		break;

	case DOWN:
		p->direction = DOWN;
		p->x = pre->x;
		p->y = pre->y - 1;
		break;

	case LEFT:
		p->direction = LEFT;
		p->x = pre->x + 1;
		p->y = pre->y;
		break;

	case RIGHT:
		p->direction = RIGHT;
		p->x = pre->x - 1;
		p->y = pre->y;
		break;
	}

	pre->next = p;
}

void destroySnake(struct Snake *snake)
{
	struct Snake *p = snake->next;

	while (p)
	{
		struct Snake *next = p->next;
		free(p);
		p = next;
	}

	snake->next = NULL;
}

void updateSnake(const struct Snake *snake, struct Map *map)
{
	const struct Snake *p = snake;

	while (p)
	{
		map->map[p->y][p->x] = p->item_ch;
		p = p->next;
	}

	updateMap(map);
}

int moveSnake(struct Snake *snake,struct Map *map,struct Food *food,int direction)
{
	int prex = snake->x, prey = snake->y, pred = snake->direction;
	int curx = 0, cury = 0, curd = 0;

	struct Snake *p = snake->next;

	switch (direction)
	{
	case UP:			
		
		if (snake->direction == DOWN)
			return moveSnake(snake,map,food,pred);	

		snake->y -= 1;

		if (checkSnakeMove(snake, map) == 1)
			return 0;
		else if (checkSnakeMove(snake, map) == 2)
			return -1;

		if (map->map[snake->y][snake->x] == food->food_ch)
		{
			score += DEFAULT_SCORE;
			food->state = 0;

			addSnakeItem(snake, DEFAULT_SNAKE_FRAME);
			updateFood(food, map);
		}

		snake->direction = UP;
		break;

	case DOWN:

		if (snake->direction == UP)
			return moveSnake(snake, map, food, pred);

		snake->y += 1;

		if (checkSnakeMove(snake, map) == 1)
			return 0;
		else if (checkSnakeMove(snake, map) == 2)
			return -1;

		if (map->map[snake->y][snake->x] == food->food_ch)
		{
			score += DEFAULT_SCORE;
			food->state = 0;

			addSnakeItem(snake, DEFAULT_SNAKE_FRAME);
			updateFood(food, map);
		}

		snake->direction = DOWN;
		break;

	case LEFT:

		if (snake->direction == RIGHT)
			return moveSnake(snake, map, food, pred);

		snake->x -= 1;

		if (checkSnakeMove(snake, map) == 1)
			return 0;
		else if (checkSnakeMove(snake, map) == 2)
			return -1;

		if (map->map[snake->y][snake->x] == food->food_ch)
		{
			score += DEFAULT_SCORE;
			food->state = 0;

			addSnakeItem(snake, DEFAULT_SNAKE_FRAME);
			updateFood(food, map);
		}

		snake->direction = LEFT;
		break;

	case RIGHT:

		if (snake->direction == LEFT)
			return moveSnake(snake, map, food, pred);

		snake->x += 1;

		if (checkSnakeMove(snake, map) == 1)
			return 0;
		else if (checkSnakeMove(snake, map) == 2)
			return -1;

		if (map->map[snake->y][snake->x] == food->food_ch)
		{
			score += DEFAULT_SCORE;
			food->state = 0;

			addSnakeItem(snake, DEFAULT_SNAKE_FRAME);
			updateFood(food, map);
		}

		snake->direction = RIGHT;
		break;

	default:
		return 0;
	}

	while (p)
	{
		curx = p->x;
		cury = p->y;
		curd = p->direction;

		map->map[p->y][p->x] = NULL_FRAME;

		p->x = prex;
		p->y = prey;
		p->direction = pred;

		prex = curx;
		prey = cury;
		pred = curd;

		p = p->next;
	}

	return 1;
}

int checkSnakeMove(const struct Snake *snake, const struct Map *map)
{
	if ((snake->x >= XSIZE - 1) || (snake->y >= YSIZE))
		return 1;

	if (map->map[snake->y][snake->x] == DEFAULT_SNAKE_FRAME || map->map[snake->y][snake->x] == map->frame)
		return 2;

	return 0;
}

int updateFood(struct Food *food, struct Map *map)
{
	if (!food->state)
	{
		srand((unsigned int)time(NULL));

		food->x = rand() % (XSIZE - 2) + 1;
		food->y = rand() % (YSIZE - 1) + 1;

		if (map->map[food->y][food->x] == DEFAULT_SNAKE_FRAME ||
			map->map[food->y][food->x] == DEFAULT_SNAKE_HEAD ||
			map->map[food->y][food->x] == map->frame)
			return 0;

		map->map[food->y][food->x] = food->food_ch;
		food->state = 1;

		return 1;
	}

	return 0;
}

unsigned int __stdcall autoGame(void *buf)
{
	struct Game g;
	struct UserData *ud = (struct UserData*)buf;

	gameStart(&g, ud->frame, ud->food_ch);

	while (g.start)
	{
		updateFood(&g.food, &g.map);

		if (moveSnake(&g.snake, &g.map, &g.food, ud->direction) == -1)
		{
			gameOver(&g);
			continue;
		}

		updateSnake(&g.snake, &g.map);
		Sleep(ud->speed);
	}

	ud->end = 1;

	return 0;
}

int usingSetting(struct UserData *ud, const char *setting_file)
{
	FILE *fp;

	if (fopen_s(&fp, setting_file, "r"))
		return -1;

	if (fscanf_s(fp, "speed: %u\r", &ud->speed) == EOF)
	{
		puts("\"speed\"setting error!");
		return -2;
	}
	else if (fscanf_s(fp, "height: %u\r", &YSIZE) == EOF)
	{
		puts("\"height\"setting error!");
		return -3;
	}
	else if (fscanf_s(fp, "width: %u\r", &XSIZE) == EOF)
	{
		puts("\"width\"setting error!");
		return -4;
	}
	else if (fscanf_s(fp, "frame: %c\r", &ud->frame, 1) == EOF)
	{
		puts("\"frame\"setting error!");
		return -5;
	}
	else if (fscanf_s(fp, "food: %c\r", &ud->food_ch, 1) == EOF)
	{
		puts("\"food\"setting error!");
		return -5;
	}

	if (YSIZE < 5 || YSIZE > YSIZE_MAX)
		YSIZE = YSIZE_MAX;
	else if (XSIZE < 5 || XSIZE > XSIZE_MAX)
		XSIZE = XSIZE_MAX;

	fclose(fp);
	return 0;
}