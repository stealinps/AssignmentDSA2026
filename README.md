
# Data Structures And Algorithms Group Assignments 

## Project Planned: RPG Game (SegFault)
<img width="1584" height="1232" alt="gamelogichirarchy" src="https://github.com/user-attachments/assets/7b678070-a3a7-42a2-b29c-56f533bee4f1" />

## [Documentation (Google Docs)](https://docs.google.com/document/d/1PfxYm6z618VuIBea9-IZOxNvSm35JHu_nA49DuNBKIo/edit?usp=sharing)


> [!IMPORTANT]
> **Use ```make windows```, ```make linux``` to compile the code**

### Rules

- No built-in headers (all functions related to DSA must be made manually)
- Pipelines;
  1. Code Implementation
  2. Video Demonstration (Not too long)
  3. Digital Portfolio/Report
  4. The Payload (Compress source code to .zip format, including video link, portfolio)
- Must include Front Page and Evaluation Form
- Rename ALL your files according to this format: **G\<Group\>_S\<Section\>_BERR**
- Presentation and demonstration will be done in **Week 14 & 15**
 
### Markings

| Aspects | Marks |
| --- | --- | 
| Ideation | 5 |
| Creativity & Design | 5 |
| Overall Presentation & Clarity | 5 |
| Quality | 10 |
| Functionality | 10 |
| Video | 10 |
| Coding Implementation | 15 |

<hr>

### To-Do List

#### Implementation
Atleast 4 of these:
- [x] Array (Grid, enemy postion, dialogue and more)
- [x] Queue (Dialogue Animation; Boss attack plan in Boss.cpp)
- [x] Stack (Boss "rage charge" combo system in Boss.cpp)
- [x] Linked-List (Inventory System)
- [x] Tree (Boss decision tree / AI brain in Boss.cpp)
- [x] Sorting (Bubble sort for leaderboard)
- [x] Searching (BFS for enemy detection)
#### Post Implementation
##### Team Portfolio
- [ ] Video (Documentation, Reflections)
- [ ] Front Page, Intro, Objectives & Ideation
- [ ] Methodology, Implementation, Results
- [ ] Challenges and Solution, Conclusion

##### Report
- [ ] File(Include Evaluation Form)
- [ ] Rename and Zip

_please add more tasks_

<hr>


<hr>

  
<br>

# Documentation

### Idea

- Turn-Based Game (Similar to Pokemon) + Path Finding Game (Undertale?)
- Similar battle structure as pokemon (linked-list)
- Path finding (searching)
- Pixel art graphics
- Worldbuilding

### Library/Program Used

- raylib
- C++
  
### Architechure

The codebase is structured using Object-Oriented Programming (OOP) principles, dividing the game logic into several core modular components:

- **Game Engine (game.cpp, main.cpp):** Handles the core application loop, window initialization, and overarching state management.

- **Rendering & UI (renderer.cpp, dialogue.cpp):** Manages the visual output, drawing the grid map, sprites, and animating dialogue boxes.

- **Entities (player.cpp, Boss.cpp):** Contains the classes, stats, and behaviors for the player character and enemy logic.

- **Environment (map.cpp, Battle.cpp):** Manages collision detection, overworld navigation, and the isolated turn-based battle sequences.

- **Item Management (Items.cpp, Inventory.cpp):** Controls the logic for discovering, storing, and utilizing in-game items.

### Data Structures & Algorithms Implementation

The core technical achievement of this project is the manual implementation of fundamental data structures to drive game mechanics without relying on built-in libraries:

- **Arrays:** Utilized for managing the fixed-size map grid, translating enemy coordinate positions, and handling static dialogue text.

- **Queue:** Used for text-based dialogue animation system (character-by-character printing) and manages the Boss's sequential attack plan.

- **Stack:** Implements a "rage charge" combo system for the Boss, where attack moves are pushed onto the stack and executed in a Last-In, First-Out (LIFO) sequence.

- **Linked-List:** Powers the dynamic Inventory System, allowing items to be seamlessly added or removed without the memory constraints of a static array.

- **Tree:** Functions as the decision tree (AI Brain) for the Boss entity, allowing it to determine its optimal next move based on the player's health, its own status, and previous turns.

- **Searching (BFS):** Breadth-First Search handles enemy detection and pathfinding across the traversable map grid.

### Debugging & Testing Environment

A self-contained test room is included for the demo/marking.

- In the spawn level, open the chest in the open hall (left side) to get the **Master Key**, then step on the portal beside it.
- The debug room (`src/levels/debug.txt`) contains every item in chests, all enemy types (Slime, Skeleton, and the Boss that uses Queue/Stack/Tree), a key-gated alcove (open with the Iron Key found inside), a signpost, and a return portal.
- Debug hotkeys (overworld): **B** = jump straight into the Boss fight, **N** = jump into a normal fight. Both are repeatable.

