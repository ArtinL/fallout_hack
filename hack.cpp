#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cctype>
#include <cstdarg>

#include "hack.hpp"

const std::string difficulties[] = {"NOVICE", "ADVANCED", "EXPERT", "MASTER"};
const char specialChars[] = {'!', '*', '(', ')', '&', '^', '%', '$', '#', '{', '}',
                             '<', '>', '?', '/', '\\', '|', '~', '`', '@', '+', '=',
                             '-', '_', '.', ',', ';', ':', '[', ']', '"', '\''};

int main(int argc, char *argv[])
{
    bool debug = false;
    bool delay = true;

    for (int i = 1; i < argc; i++)
    {
        if (std::string(argv[i]) == "--debug")
        {
            debug = true;
        }
        else if (std::string(argv[i]) == "--nodelay")
        {
            delay = false;
        }
    }

    std::srand(std::time(nullptr));

    HackGame game(debug, delay);
    return game.hack();
}

HackGame::HackGame(bool dbg, bool dly) : debug(dbg), delay(dly), errormsg("")
{
    system("clear");
    selectedDifficulty = selectDifficulty();
    system("clear");
    setup_hack();
}

void HackGame::setup_hack()
{
    errormsg.clear();

    int wordLengths[4][3] = {
        {3, 4},      // Novice
        {5, 6, 7},   // Advanced
        {8, 9, 10},  // Expert
        {11, 12, 13} // Master
    };

    int numLengths = sizeof(wordLengths[selectedDifficulty]) / sizeof(int);
    chosenLength = wordLengths[selectedDifficulty][rand() % numLengths];

    words.clear();
    populateWords();

    std::random_shuffle(words.begin(), words.end());

    int numWords = rand() % 4 + 10;

    chosenWords.clear();
    correctWord = chooseWords(numWords);

    std::random_shuffle(chosenWords.begin(), chosenWords.end());

    for (int i = 0; i < GRID_WIDTH; i++)
    {
        for (int j = 0; j < GRID_HEIGHT; j++)
        {
            grid[i][j] = ' ';
        }
    }

    interlaceWords();
    fillChars();
    findCheats();
    displayStartup();
}

int HackGame::hack()
{
    int allowedAttempts = 4;
    bool gameWon = false;
    triedWords.clear();

    int startHex = rand() % 0x10000;
    int iter = 0;

    while (allowedAttempts > 0)
    {
        drawGrid((iter == 0), startHex, allowedAttempts);
        drawPrompts();
        if (gameWon)
            break;

        std::string word;
        InputResult result = detect_input(word);

        TriedWord triedWord;
        triedWord.word = word;

        if (result == REMOVE_DUD)
            if (!removeDud()) result = REPLENISH_TRIES;
            
        
        if (result == REPLENISH_TRIES) allowedAttempts = 4;
        
        triedWord.type = result;

        triedWord.likeness = likeness(word, correctWord);
        if (triedWord.likeness == chosenLength)
            gameWon = true;

        triedWords.push_back(triedWord);

        iter++;
        if (!gameWon && result == VALID_WORD)
            allowedAttempts--;
    }

    if (gameWon)
    {
        system("clear");
        std::cout.flush();
        displaySuccess();
        return 1;
    }
    else
    {
        system("clear");
        displayFailure();
        return 0;
    }
}


int HackGame::selectDifficulty()
{
    int selectedDifficulty = -1;

    while (selectedDifficulty < 0 || selectedDifficulty >= 4)
    {
        selectedDifficulty = -1;
        delay_printf(delay, "Please select a difficulty:\n");
        for (int i = 0; i < 4; i++)
        {
            delay_printf(delay, "%d: %s\n", i + (delay ? 1 : 0), difficulties[i].c_str());
        }

        delay_printf(delay, "Enter a number between 1 and %d: ", 4);
        std::cin >> selectedDifficulty;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        selectedDifficulty--;

        if (selectedDifficulty < 0 || selectedDifficulty >= 4)
        {
            std::cout << "Invalid selection. Please try again.\n";
        }
    }

    return selectedDifficulty;
}

void HackGame::populateWords()
{
    std::ifstream file("library.txt");
    if (!file)
    {
        std::cout << "Could not open library.txt\n";
        return;
    }

    std::string word;
    while (file >> word)
    {
        if (word.length() == (size_t)chosenLength)
        {
            words.push_back(word);
        }
    }

    file.close();
}

std::string HackGame::chooseWords(int numWords)
{
    std::string correctWord = words[0];
    chosenWords.push_back(correctWord);
    words.erase(words.begin());

    for (size_t i = 0, count = 0; i < words.size() && count < 2; i++)
    {
        if (likeness(words[i], correctWord) == (int)correctWord.length() - 1)
        {
            chosenWords.push_back(words[i]);
            words.erase(words.begin() + i);
            count++;
            i--;
        }
    }

    for (size_t i = 0, count = 0; i < words.size() && count < 2; i++)
    {
        if (likeness(words[i], correctWord) == (int)correctWord.length() - 3)
        {
            chosenWords.push_back(words[i]);
            words.erase(words.begin() + i);
            count++;
            i--;
        }
    }

    for (size_t i = 0, count = 0; i < words.size() && count < 2; i++)
    {
        if (likeness(words[i], correctWord) >= 2)
        {
            chosenWords.push_back(words[i]);
            words.erase(words.begin() + i);
            count++;
            i--;
        }
    }

    for (size_t i = 0, count = 0; i < words.size() && count < 2; i++)
    {
        if (likeness(words[i], correctWord) >= 1)
        {
            chosenWords.push_back(words[i]);
            words.erase(words.begin() + i);
            count++;
            i--;
        }
    }

    while (chosenWords.size() < static_cast<size_t>(numWords) && !words.empty())
    {
        int randomIndex = rand() % words.size();
        chosenWords.push_back(words[randomIndex]);
        words.erase(words.begin() + randomIndex);
    }

    return correctWord;
}

void HackGame::interlaceWords()
{
    int gridSize = GRID_WIDTH * GRID_HEIGHT;
    int numWords = chosenWords.size();
    int totalSpacing = gridSize - numWords * chosenLength;
    int minSpacing = totalSpacing / numWords;
    int maxSpacing = minSpacing + 2;

    int pos = 0;
    for (int i = 0; i < numWords; i++)
    {
        std::string &word = chosenWords[i];

        for (int j = 0; j < chosenLength; j++)
        {
            grid[pos / GRID_HEIGHT][pos % GRID_HEIGHT] = word[j];
            pos++;
        }

        int spacing = minSpacing + rand() % (maxSpacing - minSpacing + 1);
        pos += spacing;
    }
}

void HackGame::fillChars()
{
    int numSpecialChars = sizeof(specialChars) / sizeof(specialChars[0]);
    for (int i = 0; i < GRID_WIDTH; i++)
    {
        for (int j = 0; j < GRID_HEIGHT; j++)
        {
            if (!std::isalpha(grid[i][j]))
            {
                int randomIndex = rand() % numSpecialChars;
                grid[i][j] = specialChars[randomIndex];
            }
        }
    }
}

void HackGame::findCheats()
{
    std::vector<std::stack<char>> cheatStringBuilders(4);

    enum cheatType
    {
        PAREN,
        SQUARE,
        CURLY,
        ANGLE
    };

    auto isClosing = [](char c, int type) -> bool
    {
        return (type == PAREN && c == ')') ||
               (type == SQUARE && c == ']') ||
               (type == CURLY && c == '}') ||
               (type == ANGLE && c == '>');
    };

    auto openingType = [](char c) -> int
    {
        if (c == '(')
            return PAREN;
        if (c == '[')
            return SQUARE;
        if (c == '{')
            return CURLY;
        if (c == '<')
            return ANGLE;
        return -1;
    };

    int begin = 0;
    int end = 0;

    int gridLimit = GRID_HEIGHT * GRID_WIDTH;

    while (begin <= gridLimit)
    {
        int type = openingType(grid[0][begin]);
        if (type == -1)
        {
            begin++;
            end++;
            continue;
        }

        while (end <= gridLimit && end - begin < 12 && !std::isalpha(grid[0][end]))
        {

            char c = grid[0][end];
            cheatStringBuilders[type].push(c);

            if (isClosing(c, type))
            {
                std::string cheatString;

                while (!cheatStringBuilders[type].empty())
                {
                    cheatString = cheatStringBuilders[type].top() + cheatString;
                    cheatStringBuilders[type].pop();
                }

                cheats.push_back(cheatString);
                break;
            }

            end++;
        }

        while (!cheatStringBuilders[type].empty())
        {
            cheatStringBuilders[type].pop();
        }

        begin++;
        end = begin;
    }
}

void HackGame::drawGrid(bool firstDraw, int startHex, int allowedAttempts)
{
    system("clear");

    delay_printf(firstDraw, "ROBCO INDUSTRIES (TM) TERMLINK PROTOCOL\n");
    delay_printf(firstDraw, "Enter password now\n\n");
    delay_printf(firstDraw, "%d ATTEMPT(S) REMAINING: ", allowedAttempts);
    for (int i = 0; i < allowedAttempts; i++)
    {
        delay_printf(firstDraw, "\u25A0 ");
    }
    delay_printf(firstDraw, "\n\n");
    if (allowedAttempts == 1)
    {
        std::cout << "Warning! Terminal lockout imminent!\n";
    }
    delay_printf(firstDraw, "\n");

    for (int i = 0; i < GRID_WIDTH; i++)
    {
        int row = (i % 18) + 8;
        int col = (i < 18) ? 0 : 25;

        printf("\033[%d;%dH", row, col);
        delay_printf(firstDraw, "0x%04X ", startHex + i * GRID_HEIGHT);
        for (int j = 0; j < GRID_HEIGHT; j++)
        {
            delay_printf(firstDraw && (j % 5 == 0), "%c", grid[i][j]);
        }
    }
}

void HackGame::drawPrompts()
{
    int coord2Y = 23;
    bool won = false;

    for (int i = triedWords.size() - 1; i >= 0 && coord2Y >= 7; i--)
    {
        const TriedWord &word = triedWords[i];

        if (word.type == VALID_WORD)
        {
            if (word.likeness != chosenLength)
            {
                printf("\033[%d;50H> %d/%d correct", coord2Y--, word.likeness, chosenLength);
                printf("\033[%d;50H> Entry Denied", coord2Y--);
                printf("\033[%d;50H> %s", coord2Y--, word.word.c_str());
            }
            else
            {
                // game won
                coord2Y = 25;
                printf("\033[%d;50H> is accessed.", coord2Y--);
                printf("\033[%d;50H> while system", coord2Y--);
                printf("\033[%d;50H> Please wait", coord2Y--);
                printf("\033[%d;50H> Exact Match!", coord2Y--);
                printf("\033[%d;50H> %s", coord2Y--, word.word.c_str());

                won = true;
            }
        }
        else if (word.type == REMOVE_DUD)
        {
            printf("\033[%d;50H> Dud removed", coord2Y--);
            printf("\033[%d;50H> %s", coord2Y--, word.word.c_str());
        }
        else if (word.type == REPLENISH_TRIES)
        {
            printf("\033[%d;50H> replenished.", coord2Y--);
            printf("\033[%d;50H> Allowance", coord2Y--);
            printf("\033[%d;50H> %s", coord2Y--, word.word.c_str());
        }
        else
        {
            printf("\033[%d;50H> Error", coord2Y--);
            printf("\033[%d;50H> %s", coord2Y--, word.word.c_str());
        }
    }
    if (won)
    {
        std::cout.flush();
        usleep(3000000);
        return;
    }
    printf("\033[25;50H> ");
}

InputResult HackGame::detect_input(std::string &input)
{
    char tempInput[16];
    std::cin >> tempInput;
    input = tempInput;
    int inputLength = input.length();

    std::transform(input.begin(), input.end(), input.begin(), ::toupper);

    // check if input is empty
    if (input.empty()) return ERROR;


    bool hasSpecialChar = false;

    for (char c : input)
    {
        if (!std::isalpha(c))
        {
            hasSpecialChar = true;
            break;
        }
    }

    if (hasSpecialChar)
    {
        for (const auto &cheat : cheats)
        {
            if (cheat == input)
            {

                auto it = std::find(cheats.begin(), cheats.end(), cheat);
                if (it != cheats.end()) cheats.erase(it);
                return rand() % 4 == 0 ? REPLENISH_TRIES : REMOVE_DUD;
            }
        }
    }
    else
    {
        for (const auto &tried : triedWords)
        {
            if (tried.word == input) return ERROR;
            
        }

        for (const auto &word : chosenWords)
        {
            if (word == input) return VALID_WORD;
            
        }
    }

    return ERROR;
}

bool HackGame::removeDud()
{
    if (chosenWords.size() == 1) return false;
    

    int randomIndex;
    std::string word;

    do
    {
        randomIndex = rand() % chosenWords.size();
        word = chosenWords[randomIndex];
    } while (word == correctWord);

    chosenWords.erase(chosenWords.begin() + randomIndex);

    for (int i = 0; i < GRID_WIDTH; i++)
    {
        for (int j = 0; j < GRID_HEIGHT; j++)
        {
            if (strncmp(&grid[i][j], word.c_str(), word.length()) == 0)
            {
                for (size_t k = 0; k < word.length(); k++)
                {
                    grid[i][j + k] = '.';
                }
            }
        }
    }

    return true;
}

int HackGame::likeness(const std::string &word1, const std::string &word2)
{
    int count = 0;
    for (size_t i = 0; i < word1.length() && i < word2.length(); i++)
    {
        if (word1[i] == word2[i])
            count++;
        
    }
    return count;
}

void HackGame::delay_printf(bool useDelay, const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    for (char *p = buffer; *p != '\0'; p++)
    {
        putchar(*p);
        fflush(stdout);
        if (useDelay)
        {
            usleep(10000);
        }
    }
}

void HackGame::displayStartup()
{
    system("clear");
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int consoleWidth = w.ws_col;

    const char *message = "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK\n\n\n";
    int padding = (consoleWidth - strlen(message)) / 2;

    printf("\033[0;%dH", padding);

    delay_printf(1, "%s", message);
    usleep(1500000);
    delay_printf(1, "> set terminal/inquire\n\n");
    usleep(500000);
    delay_printf(1, "RIT-V300\n\n");
    usleep(1500000);
    delay_printf(1, "> set file/protection=owner:rwed accounts.f\n");
    usleep(750000);
    delay_printf(1, "> set halt restart/maint\n\n");
    usleep(500000);
    delay_printf(1, "Initializing Robco Industries(TM) MF Boot Agent v2.3.0\n");
    delay_printf(1, "RETROS BIOS\n");
    delay_printf(1, "RBIOS-4.02.08.00 52EE5.E7.E8\n");
    delay_printf(1, "Copyright 2201-2203 RobCo Ind.\n");
    delay_printf(1, "Uppermem: 64 KB\n");
    delay_printf(1, "Root (5A8)\n");
    delay_printf(1, "Maintenance Mode\n\n");
    usleep(1500000);
    delay_printf(1, "> run debug/accounts.f\n\n");
    usleep(1500000);

    std::cout.flush();
    system("clear");
}

void HackGame::displaySuccess()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int consoleWidth = w.ws_col;

    const char *message = "WELCOME TO ROBCO INDUSTRIES (TM) TERMLINK\n\n\n";
    int padding = (consoleWidth - strlen(message)) / 2; // Calculate padding needed

    printf("\033[0;%dH", padding);

    delay_printf(1, "%s", message);
    usleep(1500000);
    delay_printf(1, "> LOGON ADMIN\n\n");
    usleep(500000);
    delay_printf(1, "ENTER PASSWORD NOW\n\n> ");
    usleep(1500000);

    for (int i = 0; i < chosenLength; i++)
    {
        delay_printf(1, "*");
    }

    delay_printf(1, "\n\n");
    usleep(500000);
    delay_printf(1, "ACCESS GRANTED\n\n");
    std::cout.flush();
}

void HackGame::displayFailure()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int consoleWidth = w.ws_col;
    int consoleHeight = w.ws_row;

    const char *message1 = "TERMINAL LOCKED";
    const char *message2 = "PLEASE CONTACT AN ADMINISTRATOR";
    int paddingX1 = (consoleWidth - strlen(message1)) / 2;
    int paddingX2 = (consoleWidth - strlen(message2)) / 2;
    int paddingY = consoleHeight / 2;

    printf("\033[%d;%dH", paddingY - 2, paddingX1);
    delay_printf(1, "%s", message1);

    printf("\033[%d;%dH", paddingY, paddingX2);
    delay_printf(1, "%s", message2);

    // Wait for input instead of infinite loop
    std::cin.ignore();
    std::cin.get();
}

void HackGame::displayError(const std::string &message)
{
    errormsg = message;
}
