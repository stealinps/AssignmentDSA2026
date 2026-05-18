#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <string>
#include "raylib.h"
#include "config.h"

#define MAX_MESSAGES 10

class DialogueBox {
private:
    std::string queue[MAX_MESSAGES];
    int frontIndex;
    int backIndex;
    int messageCount;

    // NEW: Typewriter Effect Variables
    float textTimer; 
    int charCount;   

public:
    DialogueBox();
    
    void Start(); 
    void Enqueue(std::string text);
    void Dequeue();
    
    // NEW: Engine functions for the Undertale feel
    void Update(); 
    bool IsTextFinished(); 
    void SkipTyping();     
    
    bool IsActive();
    void Draw();
};

#endif