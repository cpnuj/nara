#include <cassert>
#include <ncurses.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include "ai.hpp"
#include "bench.hpp"
#include "board.hpp"
#include "eval.hpp"
#include "log.hpp"

struct cursor_t
{
    int x;
    int y;

    cursor_t(int _x = 0, int _y = 0) : x(_x), y(_y)
    {
    }
    void set(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
    void goup(void)
    {
        if (x > 0)
            x--;
    }
    void godown(void)
    {
        if (x < nara::gomoku_board::WIDTH)
            x++;
    }
    void goleft(void)
    {
        if (y > 0)
            y--;
    }
    void goright(void)
    {
        if (y < nara::gomoku_board::WIDTH - 1)
            y++;
    }
};

void display(nara::gomoku_board const &board, cursor_t const &cursor)
{
    clear();

    move(0, 0);
    for (int i = 0; i < nara::gomoku_board::WIDTH; i++)
    {
        for (int j = 0; j < nara::gomoku_board::WIDTH; j++)
        {
            switch (board.getchess(i, j))
            {
            case nara::BLACK:
                printw("X");
                break;
            case nara::WHITE:
                printw("O");
                break;
            case nara::EMPTY:
                printw("-");
                break;
            }
            printw(" ");
        }
        printw("\n");
    }

    move(16, 0);
    printw("BLACK: %d\n", nara::evaluate(board, nara::BLACK));
    printw("WHITE: %d\n", nara::evaluate(board, nara::WHITE));

    // move(16, 0);
    // auto [res_blk, res_wht] = sboard.summary();

    // printw("result score: %d\n", sboard.total_score(nara::BLACK));
    // printw("SICK2: %d\n", res_blk.pattern_cnt[nara::SICK2]);
    // printw("FLEX2: %d\n", res_blk.pattern_cnt[nara::FLEX2]);
    // printw("SICK3: %d\n", res_blk.pattern_cnt[nara::SICK3]);
    // printw("FLEX3: %d\n", res_blk.pattern_cnt[nara::FLEX3]);
    // printw("SICK4: %d\n", res_blk.pattern_cnt[nara::SICK4]);
    // printw("FLEX4: %d\n", res_blk.pattern_cnt[nara::FLEX4]);
    // printw("FLEX5: %d\n", res_blk.pattern_cnt[nara::FLEX5]);
    // printw("to FLEX4: ");
    // for(auto p : sboard.to_flex4_points(nara::BLACK))
    // {
    //     printw("[%d, %d] ", p.x, p.y);
    // }
    // printw("to FIVE: ");
    // for(auto p : sboard.to_five_points(nara::BLACK))
    // {
    //     printw("[%d, %d] ", p.x, p.y);
    // }
    // printw("\n");

    // printw("result score: %d\n", sboard.total_score(nara::WHITE));
    // printw("SICK2: %d\n", res_wht.pattern_cnt[nara::SICK2]);
    // printw("FLEX2: %d\n", res_wht.pattern_cnt[nara::FLEX2]);
    // printw("SICK3: %d\n", res_wht.pattern_cnt[nara::SICK3]);
    // printw("FLEX3: %d\n", res_wht.pattern_cnt[nara::FLEX3]);
    // printw("SICK4: %d\n", res_wht.pattern_cnt[nara::SICK4]);
    // printw("FLEX4: %d\n", res_wht.pattern_cnt[nara::FLEX4]);
    // printw("FLEX5: %d\n", res_wht.pattern_cnt[nara::FLEX5]);
    // printw("to FLEX4: ");
    // for(auto p : sboard.to_flex4_points(nara::WHITE))
    // {
    //     printw("[%d, %d] ", p.x, p.y);
    // }
    // printw("\n");
    // printw("to FIVE: ");
    // for(auto p : sboard.to_five_points(nara::WHITE))
    // {
    //     printw("[%d, %d] ", p.x, p.y);
    // }
    // printw("\n");

    refresh();

    move(cursor.x, cursor.y * 2);
}

enum gomoku_action
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CHOOSE,
    QUIT,
    NONE
};

gomoku_action getaction(void)
{
    switch (getch())
    {
    case 'q':
    case 'Q':
        return QUIT;

    case ' ':
        return CHOOSE;

    // handle arrow keys, code from https://stackoverflow.com/a/11432632
    case '\033': {
        getch(); // skip the [
        switch (getch())
        { // the real value
        case 'A':
            return UP;
        case 'B':
            return DOWN;
        case 'C':
            return RIGHT;
        case 'D':
            return LEFT;
        default:
            return NONE;
        }
    }

    default:
        return NONE;
    }
}

int main()
{
    logger.open("LOG");

    nara::gomoku_board board = nara::gomoku_board();

    cursor_t cursor;

    // ai mode

    std::cout << "Please choose your chess, b for X, others for O: ";
    std::string choose;
    std::cin >> choose;
    std::cout << choose;

    nara::gomoku_chess my_chess;
    nara::gomoku_chess ai_chess;

    my_chess = (choose == "b") ? nara::BLACK : nara::WHITE;
    ai_chess = nara::oppof(my_chess);

    nara::gomoku_ai ai = nara::gomoku_ai(ai_chess);

    initscr();
    display(board, cursor);

    nara::gomoku_chess turn = nara::BLACK;
    nara::point_t last_move{0, 0};
    while (nara::get_winner(board, last_move) == 0)
    {
        if (turn == ai_chess)
        {
            nara::point_t ai_next;

            auto start = std::chrono::system_clock::now();

            ai_next = ai.get_next(board);

            auto end = std::chrono::system_clock::now();
            logger << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << std::endl;
            logger.flush();

            last_move = ai_next;

            if (board.getchess(ai_next) != nara::EMPTY)
                assert(false);

            board.setchess(ai_next, ai_chess);
            cursor.set(ai_next.x, ai_next.y);
        }
        else
        { // player turn
            for (;;)
            {
                gomoku_action action = getaction();
                if (action == QUIT)
                    goto quit;
                if (action == CHOOSE)
                {
                    if (board.getchess(cursor.x, cursor.y) == nara::EMPTY)
                    {
                        board.setchess(cursor.x, cursor.y, my_chess);
                        last_move = {cursor.x, cursor.y};
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                switch (action)
                {
                case UP:
                    cursor.goup();
                    break;
                case DOWN:
                    cursor.godown();
                    break;
                case LEFT:
                    cursor.goleft();
                    break;
                case RIGHT:
                    cursor.goright();
                    break;
                default: {
                }
                }
                display(board, cursor);
            }
        }
        display(board, cursor);
        turn = nara::oppof(turn);
    }

    move(15, 0);
    if (nara::get_winner(board, last_move) == nara::BLACK)
        printw("Black win!");
    else
        printw("White win!");
    refresh();
    getaction();

quit:
    logger.close();
    endwin();
    return 0;
}
