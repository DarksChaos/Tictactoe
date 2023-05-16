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

#define Log(func, args, txt)\
std::cout /*<< '[' << std::chrono::duration_cast<std::chrono::milliseconds>\(std::chrono::high_resolution_clock::now() - start).count()\
<< "ms] " */<< func << '(' << args << "): " << txt << "\n"\
/*start = std::chrono::high_resolution_clock::now();*/
#endif

enum Entity {
    PLAYING = -1,
    OTHER = 0, LOSING = 0,
    EMPTY = 1, TIE = 1, 
    YOU = 2, WINNING = 2,
};

using Position = std::array<Entity, 9>;

std::string entity_case_str(Entity e) {
    switch(e) {
        case OTHER: return "other";
        case EMPTY: return "empty";
        case YOU: return "you";
        default: return "none";
    }
}

std::string entity_eval_str(Entity e) {
     switch(e) {
        case PLAYING: return "playing";
        case TIE: return "tie";
        case LOSING: return "losing";
        case WINNING: return "winning";
        default: return "none";
    }
}

std::string position_str(Position position);

struct TreeNode {
    // game-related
    Position position;
    Entity player = YOU;
    // tree-related
    std::vector<TreeNode> branches = {};
    Entity eval = PLAYING;
    size_t depth = 0;
    int last_move = -1;
 
    void play(int i) {
        if(position[i] == EMPTY) {
            position[i] = player;
            player = (player == YOU ? OTHER : YOU);
        }
    }
    void print_position() {
        std::cout << position_str(position) << '\n';
    }
};

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
    os << "[ ";
    for(int i: position) {
        os << i << ' ';
    }
    os << ']';
    return os.str();
}

bool eq(Entity a, Entity b, Entity c) {
    return a==b && a==c;
}

Entity position_eval(Position position) {
#define NAME "position_eval" 
#define ARGS position_fast_str(position)
    Log(NAME, ARGS, "Computing eval");
    if(position[0] != EMPTY && (
        eq(position[0], position[1], position[2])
        || eq(position[0], position[4], position[8])
        || eq(position[0], position[3], position[6])
    )) return  position[0];
    else if(position[8] != EMPTY && (
        eq(position[8], position[1], position[2])
        || eq(position[8], position[4], position[8])
    )) return  position[8];
    else if(position[4] != EMPTY && (
        eq(position[4], position[5], position[6])
        || eq(position[4], position[1], position[7])
    )) return position[4];
    else {
        for(int i = 0; i < 9; i++) {
            if(position[i] == EMPTY) {
                return PLAYING;
                break;
            }
        }
    }
    return TIE;
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


TreeNode compute_tree(TreeNode& top) {
#define NAME "compute_tree"
#define ARGS position_fast_str(top.position) << ", depth=" << depth << ", id=" << id
    static int total_positions = 0; 
    static int depth = 0;
    total_positions++; depth++;
    int id = total_positions;
    Log(NAME, ARGS, "Computing tree for position " << position_fast_str(top.position));
    std::vector<int> moves = list_moves(top.position);
    Entity eval = position_eval(top.position);
    if(moves.size() == 0 || eval != PLAYING) {
        depth--;
        Log(NAME, ARGS, "Game finished, returning eval " << entity_eval_str(eval));
        top.eval = eval;
        return top;
    }
    Log(NAME, ARGS, "Entering loop");
    for(int move: moves) {
        Log(NAME, ARGS, "Entering loop iteration, move=" << move);
        TreeNode copy = { top.position, top.player, {}};
        Log(NAME, ARGS, "Copied tree position and player, move=" << move);
        copy.play(move);
        Log(NAME, ARGS, "Applied move " << move << " to branch");
        top.branches.push_back(compute_tree(copy));
        Log(NAME, ARGS, "Added branch after move " << move);
    }
    depth--;
    Log(NAME, ARGS, "Returning top");
    return top;
}

TreeNode compute_tree(TreeNode&& top) {
    return compute_tree(top);
}

int main() {
    std::cout << "x: vous, o: opposant\n";
    std::cout << "tableau:\n";
    TreeNode tree = compute_tree(TreeNode {{YOU, EMPTY, EMPTY, EMPTY, OTHER, OTHER, EMPTY, EMPTY, YOU}});
    tree.print_position();
}