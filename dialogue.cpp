#include "dialogue.h"

DialogueBox::DialogueBox() {
    Start(); // Ensure everything is zeroed out when the game boots
}

void DialogueBox::Start() {
    frontIndex = 0;
    backIndex = 0;
    messageCount = 0;
    
    // Reset typewriter
    textTimer = 0.0f;
    charCount = 0;
}

void DialogueBox::Dequeue() {
    if (messageCount > 0) {
        frontIndex = (frontIndex + 1) % MAX_MESSAGES;
        messageCount--;
        
        // Reset typewriter for the NEXT sentence
        textTimer = 0.0f;
        charCount = 0;
    }
}

void DialogueBox::Enqueue(std::string text) {
    // Only add a message if the queue isn't completely full
    if (messageCount < MAX_MESSAGES) {
        
        // Put the text in the next available slot
        queue[backIndex] = text;
        
        // Move the back pointer forward, wrapping around if it hits the end!
        backIndex = (backIndex + 1) % MAX_MESSAGES; 
        
        messageCount++;
    }
}

bool DialogueBox::IsActive() {
    return messageCount > 0; // If there are messages, the dialogue is active
}

void DialogueBox::Update() {
    if (!IsActive()) return;

    // GetFrameTime() tells us exactly how much time passed since the last frame
    textTimer += GetFrameTime(); 

    // Every 0.03 seconds, reveal one more letter! (Lower this number to type faster)
    if (textTimer > 0.03f) {
        textTimer = 0.0f;
        
        // If we haven't revealed the whole sentence yet, add a letter
        if (charCount < queue[frontIndex].length()) {
            charCount++;
        }
    }
}

bool DialogueBox::IsTextFinished() {
    return charCount >= queue[frontIndex].length();
}

void DialogueBox::SkipTyping() {
    // Instantly set the character count to the full length of the string
    charCount = queue[frontIndex].length();
}
// -----------------------------

void DialogueBox::Draw() {
    if (!IsActive()) return; 

    // Undertale's layout: Wide, bottom of screen, lots of padding
    float boxWidth = SCREEN_WIDTH - 80;
    float boxHeight = 150;
    float boxX = 40;
    float boxY = SCREEN_HEIGHT - boxHeight - 20;

    Rectangle boxRec = { boxX, boxY, boxWidth, boxHeight };
    
    // 1. Draw the Solid Black Background
    DrawRectangleRec(boxRec, BLACK);
    
    // 2. Draw the Thick White Border (5 pixels thick)
    DrawRectangleLinesEx(boxRec, 5, WHITE);

    // 3. String Manipulation: Create a temporary string containing ONLY the revealed letters
    // and slap the classic Undertale asterisk at the front of it.
    std::string currentText = "* " + queue[frontIndex].substr(0, charCount);
    
    // Draw the text
    DrawText(currentText.c_str(), boxX + 30, boxY + 30, 20, WHITE);
    
    // 4. Only show the "Press SPACE" prompt if the text has finished typing!
    if (IsTextFinished()) {
        DrawText("Press SPACE", boxX + boxWidth - 150, boxY + boxHeight - 30, 15, GRAY);
    }
}