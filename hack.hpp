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

    char grid[GRID_WIDTH][GRID_HEIGHT];

    std::string errormsg;


    int selectDifficulty();
    void setup_hack(int diff);
    void populateWords(std::vector<std::string>& words, int chosenLength);
    std::string chooseWords(std::vector<std::string>& words, std::vector<std::string>& chosenWords, int numWords);
    void interlaceWords(std::vector<std::string>& chosenWords, int wordLength);
    void fillChars();
    void findCheats();
    void drawGrid(bool useDelay, int startHex, int allowedAttempts);
    void drawPrompts(std::vector<TriedWord>& triedWords, int chosenLength);
    InputResult detect_input(std::vector<std::string>& chosenWords, std::string& input,
                                std::vector<TriedWord>& triedWords);
    bool removeDud(std::vector<std::string>& chosenWords, const std::string& correctWord);
    int likeness(const std::string& word1, const std::string& word2);
    void delay_printf(bool useDelay, const char* format, ...);
    void displaySuccess(int chosenLength);
    void displayFailure();
    void displayError(const std::string& message);
    
};

#endif // HACK_HPP
