#ifndef HACK_HPP
#define HACK_HPP

#include <string>
#include <vector>

#define GRID_WIDTH 36
#define GRID_HEIGHT 16

enum InputResult {
    ERROR = -1,
    REMOVE_DUD = 0,
    VALID_WORD = 1,
    REPLENISH_TRIES = 3
};

class HackGame {
public:
    // Constructor now handles difficulty selection internally
    HackGame(bool dbg = false, bool dly = true);
    int hack();
    
private:
    struct TriedWord {
        std::string word;
        int likeness;
        InputResult type;
    };

    int selectedDifficulty;
    bool debug;
    bool delay;
    int chosenLength;
    std::vector<std::string> words;
    std::vector<std::string> chosenWords;
    std::string correctWord;
    std::vector<std::string> cheats;
    std::vector<TriedWord> triedWords;

    char grid[GRID_WIDTH][GRID_HEIGHT];

    std::string errormsg;


    int selectDifficulty();
    void setup_hack();
    void populateWords();
    std::string chooseWords(int numWords);
    void interlaceWords();
    void fillChars();
    void findCheats();
    void drawGrid(bool firstDraw, int startHex, int allowedAttempts);
    void drawPrompts();
    InputResult detect_input(std::string& input);
    bool removeDud();
    int likeness(const std::string& word1, const std::string& word2);
    void delay_printf(bool useDelay, const char* format, ...);
    void displayStartup();
    void displaySuccess();
    void displayFailure();
    void displayError(const std::string& message);
    
};

#endif // HACK_HPP
