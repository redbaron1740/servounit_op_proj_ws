#ifndef MENU_DISPLAY_HPP_
#define MENU_DISPLAY_HPP_

#include <ncurses.h>
#include <string>
#include <vector>
#include <unistd.h>

class Servounit_Operating_Prog {
private:
    std::vector<std::string> menuItems;
    size_t currentSelection;
    std::string title ;

public:
    Servounit_Operating_Prog(const std::string& menuTitle, const std::vector<std::string>& items) 
        : menuItems(items), currentSelection(0)  {

            title = menuTitle;

        }

    void display() {
        clear();
        
        // 제목 표시
        attron(A_BOLD);
        mvprintw(1, 2, "%s", title.c_str());
        attroff(A_BOLD);
        
        mvprintw(2, 2, "========================================");
        
        // 메뉴 항목 표시
        for (size_t i = 0; i < menuItems.size(); ++i) {
            if (i == currentSelection) {
                attron(A_REVERSE); // 선택된 항목 강조
                mvprintw(4 + i, 4, "> %s", menuItems[i].c_str());
                attroff(A_REVERSE);
            } else {
                mvprintw(4 + i, 4, "  %s", menuItems[i].c_str());
            }
        }
        
        // 도움말 표시
	    mvprintw(LINES - 4, 2, "========================================");
        mvprintw(LINES - 3, 2, "Arrow Up/Down: Move | Enter: Select | Q: Quit");
        mvprintw(LINES - 2, 2, "Copyright (C) 2026 ADUS Inc., All rights reserved.");
        
        refresh();
    }

    int run() {
        int ch;
        bool running = true;
        
        while (running) {
            display();
            ch = getch();
            
            switch (ch) {
                case KEY_UP:
                    if (currentSelection > 0) {
                        currentSelection--;
                    }
                    else //current == 0 이면 마지막 메뉴로 이동
                        currentSelection = menuItems.size() - 1;
                    break;
                    
                case KEY_DOWN:
                    if (currentSelection < menuItems.size() - 1) {
                        currentSelection++;
                    }
                    else //current == 마지막 메뉴 이면 첫 번째 메뉴로 이동
                        currentSelection = 0;
                    break;
                    
                case 10: // Enter 키
                case KEY_ENTER:
                    return currentSelection;
                    
                case 'q':
                case 'Q':
                    return -1; // 종료
                    
                default:
                    break;
            }
        }
        
        return -1;
    }
};

#endif // MENU_DISPLAY_HPP_