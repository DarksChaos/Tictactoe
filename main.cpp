#include <iostream>
#include <vector>
#include <array>
#include <chrono>
#include <ratio>
#include <sstream>
#include <thread>

#ifdef NDEBUG
#define Log(a, b, c)
#else
auto start = std::chrono::high_resolution_clock::now();

#define Log(func, args, txt)                                                                                                                                                         \
    std::cout                                          /*<< '[' << std::chrono::duration_cast<std::chrono::milliseconds>\(std::chrono::high_resolution_clock::now() - start).count() \
                                             << "ms] " */                                                                                                                            \
        << func << '(' << args << "): " << txt << "\n" /*start = std::chrono::high_resolution_clock::now();*/
#endif

enum Entity
{
    PLAYING = -1,
    NONE = -1,
    OTHER = 0,
    LOSING = 0,
    EMPTY = 1,
    TIE = 1,
    YOU = 2,
    WINNING = 2,
};

using Position = std::array<Entity, 9>;

std::string entity_case_str(Entity e)
{
    switch (e)
    {
    case OTHER:
        return "other";
    case EMPTY:
        return "empty";
    case YOU:
        return "you";
    default:
        return "none";
    }
}

std::string entity_eval_str(Entity e)
{
    switch (e)
    {
    case PLAYING:
        return "playing";
    case TIE:
        return "egalité";
    case LOSING:
        return "perdant";
    case WINNING:
        return "gagnant";
    default:
        return "none";
    }
}

std::string position_str(Position, int);
std::string position_fast_str(Position);
Entity position_eval(Position);

struct TreeNode
{
    // game-related
    Position position;
    Entity player = YOU;
    // tree-related
    std::vector<TreeNode> branches = {};
    Entity eval = PLAYING;
    size_t depth = 0;
    int last_move = -1;
    std::vector<int> best_sequence = {};
    void clear() {
        branches.clear();
        best_sequence.clear();
        eval = PLAYING;
        depth = 0;
    }
    void play(int i)
    {
        if (position[i] == EMPTY)
        {
            position[i] = player;
            player = (player == OTHER ? YOU : OTHER);
        } else {
            std::exit(1);
        }
    }
    void print_fast_position()
    {
        std::cout << position_fast_str(position) << '\n';
    }

    void print_position(int depth = 0)
    {
        std::cout << position_str(position, depth) << '\n';
    }

    void print_branches(int depth = 0)
    {
        std::string spaces = std::string(depth * 4, ' ');
        std::cout << spaces << " --------\n";
        std::cout << spaces << " -> Depth: " << depth << '\n';
        std::cout << spaces << " -> Move: " << last_move << '\n';
        std::cout << spaces << " -> Evaluation: " << entity_eval_str(eval) << '\n';
        std::cout << spaces << " -> Moves: [ ";
        for (TreeNode &branch : branches)
        {
            std::cout << branch.last_move << ' ';
        }
        std::cout << "]\n"
                  << spaces << " -> Position: " << '\n';
        print_position(depth + 1); // offset by 8 spaces
        if (branches.size() > 0)
        {
            std::cout << spaces << " -> Branches:\n";
            for (TreeNode branch : branches)
            {
                branch.print_branches(depth + 1);
            }
        }
    }
};

char chars[3] = {'O', ' ', 'X'};

char case_char_or_pos(Entity square, int pos) {
    if(square == EMPTY) {
        return pos + '0';
    }
    return chars[square];
}

std::string position_str(Position position, int depth = 0)
{
    std::ostringstream os;
    std::string spaces = std::string(depth * 4, ' ');
    for (int i = 0; i < 9; i += 3)
    {
        os << spaces << case_char_or_pos(position[i], i) << '|' << case_char_or_pos(position[i+1], i+1) << '|' << case_char_or_pos(position[i+2], i+2);
        if (i < 6)
            os << '\n';
    }
    return os.str();
}


std::string position_fast_str(Position position)
{
    std::ostringstream os;
    os << "[ ";
    for (int i : position)
    {
        os << i << ' ';
    }
    os << ']';
    return os.str();
}

bool eq(Entity a, Entity b, Entity c)
{
    return a == b && a == c;
}

Entity position_eval(Position position)
{
#define NAME "position_eval"
#define ARGS position_fast_str(position)
    Log(NAME, ARGS, "Computing eval for position " << position_fast_str(position));
    if (position[0] != EMPTY && (eq(position[0], position[1], position[2]) || eq(position[0], position[4], position[8]) || eq(position[0], position[3], position[6])))
        return position[0];
    else if (position[8] != EMPTY && (eq(position[6], position[7], position[8]) || eq(position[2], position[5], position[8])))
        return position[8];
    else if (position[4] != EMPTY && (eq(position[3], position[4], position[5]) || eq(position[1], position[4], position[7]) || eq(position[2], position[4], position[6])))
        return position[4];
    else
    {
        for (int i = 0; i < 9; i++)
        {
            if (position[i] == EMPTY)
            {
                return PLAYING;
                break;
            }
        }
    }
    return TIE;
}

std::vector<int> list_moves(Position position)
{
#define NAME "list_moves"
#define ARGS position_fast_str(position)
    Log(NAME, ARGS, "Listing moves");
    std::vector<int> moves;
    for (int i = 0; i < 9; i++)
    {
        if (position[i] == EMPTY)
            moves.push_back(i);
    }
    Log(NAME, ARGS, "Done, " << moves.size() << " moves");
    return moves;
}

TreeNode compute_tree(TreeNode &top)
{
#define NAME "compute_tree"
#define ARGS position_fast_str(top.position) << ", depth=" << depth << ", id=" << id
    static int total_positions = 0;
    static int depth = 0;
    total_positions++;
    depth++;
    int id = total_positions;
    Log(NAME, ARGS, "Computing tree for position " << position_fast_str(top.position));
    std::vector<int> moves = list_moves(top.position);
    Entity eval = position_eval(top.position);
    top.depth = depth;
    if (moves.size() == 0 || eval != PLAYING)
    {
        depth--;
        Log(NAME, ARGS, "Game finished, returning eval " << entity_eval_str(eval));
        top.eval = eval;
        return top;
    }
    Log(NAME, ARGS, "Move list:");
#ifndef NDEBUG
    for (int move : moves)
    {
        std::cout << move << ' ';
    }
    std::cout << '\n';
#endif
    Log(NAME, ARGS, "Entering loop");
    for (int move : moves)
    {
        Log(NAME, ARGS, "Entering loop iteration, move=" << move);
        TreeNode copy = {top.position, top.player};
        copy.last_move = move;
        Log(NAME, ARGS, "Copied tree position and player, move=" << move);
        copy.play(move);
        Log(NAME, ARGS, "Applied move " << move << " to branch");
        top.branches.push_back(compute_tree(copy));
        Log(NAME, ARGS, "Added branch after move " << move);
    }
    depth--;
    Log(NAME, ARGS, "Function call ended for position " << position_fast_str(top.position));
    return top;
}

TreeNode compute_tree(TreeNode &&top)
{
    return compute_tree(top);
}

void find_best_sequence(TreeNode &top)
{
#define NAME "find_best_sequence"
#define ARGS position_fast_str(top.position) << ", depth=" << top.depth
    Log(NAME, ARGS, "Function called");
    if(top.branches.size() == 0) {
        Log(NAME, ARGS, "Game finished, adding last move to top's best sequence");
        top.best_sequence.push_back(top.last_move);
        return;
    }

    // info about best branch
    bool best_set = false;
    int best_id = 0;
    size_t best_depth = 0;
    Entity best_eval = NONE;

    Log(NAME, ARGS, "Starting loop through " << top.branches.size() << " branches");
    for (size_t i = 0; i < top.branches.size(); i++)
    {
        TreeNode &branch = top.branches[i];
        Log(NAME, ARGS, "New loop, position = " << position_fast_str(branch.position));
        if (branch.branches.size() > 0)
        {
            Log(NAME, ARGS, "Game not finished yet, have to loop with branch as top");
            find_best_sequence(branch);
        }
        // no best set
        Log(NAME, ARGS, "Comparing element to best: eval = " << entity_eval_str(branch.eval) << " best_eval = " << entity_eval_str(best_eval) << ", depth = " << branch.depth << " best_depth = " << best_depth << ", id = " << i << " best_id = " << best_id);
        if (!best_set)
        {
            Log(NAME, ARGS, "Setting as best: No best set");
            best_set = true;
            best_id = i;
            best_depth = branch.depth;
            best_eval = branch.eval;
        }
        // strictly better position
        else if((top.player == YOU && branch.eval > best_eval) || (top.player == OTHER && branch.eval < best_eval)) {
            Log(NAME, ARGS, "Setting as best: Position is better");
            best_id = i;
            best_depth = branch.depth;
            best_eval = branch.eval;
        }
        // winning position, shorter path
        else if(top.player == branch.eval) {
            Log(NAME, ARGS, "Position is winning");
            if(branch.depth > best_depth) {
                Log(NAME, ARGS, "Setting as best: Position winning and shorter");
                best_id = i;
                best_depth = branch.depth;
                best_eval = branch.eval;
            }
        }
        // equivalent position, longer path
        else if(branch.eval == best_eval) {
            Log(NAME, ARGS, "Position is equivalent");
            if(branch.depth < best_depth) {
                Log(NAME, ARGS, "Setting as best: Position equivalent and longer");
                best_id = i;
                best_depth = branch.depth;
                best_eval = branch.eval;
            }
        } else {
            Log(NAME, ARGS, "Not setting element as best");
        }
    }
    top.best_sequence = top.branches[best_id].best_sequence;
    top.eval = best_eval;
    top.best_sequence.insert(top.best_sequence.begin(), top.branches[best_id].last_move);
}

void find_best_sequence(TreeNode &&top)
{
    find_best_sequence(top);
}

void bot_vs_bot() {
    TreeNode tree = compute_tree(TreeNode{ .position = {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, .player = YOU});
    tree.print_position();
    find_best_sequence(tree);
    for(int move: tree.best_sequence) {
        std::cout << move << '\n';
        tree.play(move);
        tree.print_position();
    }
    std::cout << '\n';
}

void player_vs_bot() {
    TreeNode tree = compute_tree(TreeNode{ .position = {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, .player = YOU});
    tree.clear();
    Entity starter;
    std::cout << "commencer ? [o/n] ";
    char input;
    std::cin >> input;
    if(input == 'o' || input == 'O') {
        starter = YOU;
    } else {
        starter = OTHER;
    }
    while(position_eval(tree.position) == PLAYING) {
        tree.clear();
        tree = compute_tree(tree);
        find_best_sequence(tree);
        std::cout << "------------\nevaluation: " << entity_eval_str(tree.eval) << '\n';
        tree.print_position();
        int move;
        if(tree.player == starter) {
            while(true) {
                std::cout << "votre tour: ";
                std::cin >> move;
                if(move < 0 || move > 8) {
                    std::cout << "coup inexistant\n";
                    continue;
                }
                if(tree.position[move] != EMPTY) {
                    std::cout << "coup interdit\n";
                    continue;
                }
                break;
            }
        } else {
            move = tree.best_sequence[0];
            std::cout << "mon coup: " << move << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        tree.play(move);
    }
    tree.print_position();
}

int main()
{
    player_vs_bot();
}