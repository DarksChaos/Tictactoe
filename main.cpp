#include <iostream>
#include <vector>
#include <array>
#include <chrono>
#include <ratio>
#include <sstream>

#ifdef NDEBUG
#define Log(a, b, c)
#else
auto start = std::chrono::high_resolution_clock::now();
#define Log(func, args, txt) std::cout << '[' << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() << "ms] " << func << '(' << args << "): " << txt << "\n";start = std::chrono::high_resolution_clock::now();
#endif

enum Entity {
    PLAYING = -1, 
    OTHER = 0, LOSING = 0,
    EMPTY = 1, TIE = 1, 
    YOU = 2, WINNING = 2,
};

using Position = std::array<Entity, 9>;

char chars[3] = {'o', ' ', 'x'};

std::string position_str(Position position) {
    std::ostringstream os;
    for(int i = 0; i < 9; i+=3) {
        os << chars[position[i]] << '|' << chars[position[i+1]] << '|' << chars[position[i+2]];
        if(i < 6)
            os << '\n';
    }
    return os.str();
}

std::string position_fast_str(Position position) {
    std::ostringstream os;
    os << '[';
    for(int i: position) {
        os << i << ' ';
    }
    os << ']';
    return os.str();
}

void print_position(Position position) {
    std::cout << position_str(position) << '\n';
}

struct TreeNode {
    Position position;
    Entity player = YOU;
    std::vector<TreeNode> next;
    void play(int i) {
        if(position[i] == EMPTY) {
            position[i] = player;
            player = (player == YOU ? OTHER : YOU);
        }
    }
    void print() {
        print_position(position);
    }
};

bool eq(Entity a, Entity b, Entity c) {
    return a==b && a==c;
}

Entity game_status(Position position) {
#define NAME "game_status" 
#define ARGS position_fast_str(position)
    Log(NAME, ARGS, "Computing status");
    Entity status;
    if(position[0] != EMPTY && (
        eq(position[0], position[1], position[2])
        || eq(position[0], position[4], position[8])
        || eq(position[0], position[3], position[6])
    )) status = position[0];
    else if(position[8] != EMPTY && (
        eq(position[8], position[1], position[2])
        || eq(position[8], position[4], position[8])
    )) status = position[8];
    else if(position[4] != EMPTY && (
        eq(position[4], position[5], position[6])
        || eq(position[4], position[1], position[7])
    )) status = position[4];
    else {
        status = TIE;
        for(int i = 0; i < 9; i++) {
            if(position[i] == EMPTY) {
                status = PLAYING;
                break;
            }
        }
    }
    Log(NAME, ARGS, "Done");
    return status;
}

std::vector<int> list_moves(Position position) {
#define NAME "list_moves"
#define ARGS position_fast_str(position)
    Log(NAME, ARGS, "Listing moves");
    std::vector<int> moves;
    for(int i = 0; i < 9; i++) {
        if(position[i] == EMPTY)
            moves.push_back(i);
    }
    Log(NAME, ARGS, "Done, " << moves.size() << " moves");
    return moves;
}

int total_positions = 0; 
int depth = 0;
TreeNode compute_tree(TreeNode& top) {
#define NAME "compute_tree"
#define ARGS position_fast_str(top.position) << ", depth=" << depth << ", id=" << id
    total_positions++; depth++;
    int id = total_positions;
    int cur_total = total_positions;
    Log(NAME, ARGS, "Computing tree");
    std::vector<int> moves = list_moves(top.position);
    if(moves.size() == 0 || game_status(top.position) != PLAYING) {
        depth--;
        Log(NAME, ARGS, "Returning: Game finished");
        return top;
    }
    Log(NAME, ARGS, "Entering loop");
    for(int move: moves) {
        Log(NAME, ARGS, "Entering loop iteration, move=" << move);
        TreeNode copy = { top.position, top.player, {}};
        Log(NAME, ARGS, "Copied tree position and player, move=" << move);
        copy.play(move);
        Log(NAME, ARGS, "Applied move=" << move << "computing tree for new board=" << position_fast_str(copy.position));
        top.next.push_back(compute_tree(copy));
        Log(NAME, ARGS, "");
    }
    Log(NAME, ARGS, "Exitting loop");
    depth--;
    return top;
}

int main() {
    std::cout << "x: vous, o: opposant\n";
    std::cout << "tableau:\n";
    TreeNode game {{EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, YOU, {}};
    game.print();
    TreeNode tree = compute_tree(game);
    std::cout << "Nombre de positions: " << total_positions << "\n";
}