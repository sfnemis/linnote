#include "ExampleNotes.h"
#include "NoteManager.h"

void ExampleNotes::createExamples(NoteManager *manager) {
  if (!manager)
    return;

  // Page 1: Keyboard Shortcuts (FIRST - user sees this first!)
  Note shortcuts = manager->createNote("Keyboard Shortcuts");
  manager->updateNoteContent(
      shortcuts.id(),
      "Welcome to LinNote!\n\n"
      "=== Essential Shortcuts ===\n\n"
      "Navigation:\n"
      "  Ctrl+Tab           Next page\n"
      "  Ctrl+Shift+Tab     Previous page\n"
      "  Ctrl+N             New page\n"
      "  Ctrl+D             Delete current page\n"
      "  Ctrl+Home          First page\n"
      "  Ctrl+End           Last page\n"
      "  Ctrl+1-9           Go to page 1-9\n\n"
      "Window:\n"
      "  Esc                Hide window\n"
      "  Ctrl+T             Always on top\n"
      "  Ctrl+E             Export note\n\n"
      "Editing:\n"
      "  Ctrl+F             Find in note\n"
      "  Ctrl+H             Find & Replace\n"
      "  Ctrl+L             Toggle note lock\n"
      "  /                  Open command menu\n\n"
      "Power Features:\n"
      "  Ctrl+Shift+O       OCR screen capture\n"
      "  Ctrl+Shift+L       Shorten selected URL\n\n"
      "Tip: Delete this page when you've learned the shortcuts!\n");

  // Page 2: Calculator Mode
  Note n1 = manager->createNote("Calculator Example");
  manager->updateNoteContent(n1.id(), "calc\n"
                                      "Welcome to Calculator Mode!\n\n"
                                      "Basic math:\n"
                                      "5 + 3 =\n"
                                      "10 * 4 =\n"
                                      "100 / 5 =\n\n"
                                      "Variables:\n"
                                      "tax:18\n"
                                      "price:100\n"
                                      "{price} * {tax}% =\n\n"
                                      "Currency conversion:\n"
                                      "50 USD to EUR =\n"
                                      "100 EUR to TRY =\n\n"
                                      "Unit conversion:\n"
                                      "10 km to miles =\n"
                                      "25 celsius to fahrenheit =\n");

  // Page 2: List Mode
  Note n2 = manager->createNote("Checklist Example");
  manager->updateNoteContent(n2.id(),
                             "list\n"
                             "Welcome to List Mode!\n\n"
                             "Your checklist:\n"
                             "☐ Learn LinNote features\n"
                             "☐ Try keyboard shortcuts\n"
                             "☑ Install LinNote\n\n"
                             "Tip: Type /x at line start to toggle checkbox\n"
                             "Tip: Press Ctrl+Enter to toggle current line\n");

  // Page 3: Sum Function
  Note n3 = manager->createNote("Sum Example");
  manager->updateNoteContent(n3.id(), "Sum Function Example\n\n"
                                      "Shopping list with prices:\n"
                                      "Apple: 25\n"
                                      "Banana: 30\n"
                                      "Orange: 45\n"
                                      "Milk: 15\n"
                                      "Bread: 10\n\n"
                                      "Type 'sum' below to see the total:\n"
                                      "sum\n");

  // Page 4: Average Function
  Note n4 = manager->createNote("Average Example");
  manager->updateNoteContent(n4.id(), "Average Function Example\n\n"
                                      "Student test scores:\n"
                                      "Math: 85\n"
                                      "Physics: 92\n"
                                      "Chemistry: 78\n"
                                      "Biology: 88\n"
                                      "English: 95\n\n"
                                      "Type 'avg' below to see the average:\n"
                                      "avg\n");

  // Page 5: Count Function
  Note n5 = manager->createNote("Count Example");
  manager->updateNoteContent(
      n5.id(), "Count Function Example\n\n"
               "LinNote is a powerful scratchpad for Linux.\n"
               "It helps you take quick notes, calculate,\n"
               "and organize your thoughts efficiently.\n\n"
               "Type 'count' below to see word and character count:\n"
               "count\n");

  // Page 6: Code Mode
  Note n6 = manager->createNote("Code Example");
  manager->updateNoteContent(n6.id(),
                             "code\n"
                             "// JavaScript Example\n"
                             "function greet(name) {\n"
                             "  return `Hello, ${name}!`;\n"
                             "}\n\n"
                             "const users = ['Alice', 'Bob', 'Charlie'];\n"
                             "users.forEach(user => {\n"
                             "  console.log(greet(user));\n"
                             "});\n\n"
                             "# Python Example\n"
                             "def fibonacci(n):\n"
                             "    if n <= 1:\n"
                             "        return n\n"
                             "    return fibonacci(n-1) + fibonacci(n-2)\n\n"
                             "for i in range(10):\n"
                             "    print(fibonacci(i))\n");

  // Page 7: Timer Mode
  Note n7 = manager->createNote("Timer Example");
  manager->updateNoteContent(n7.id(),
                             "timer\n"
                             "Pomodoro Timer Mode\n\n"
                             "This mode enables the timer bar above.\n\n"
                             "How to use:\n"
                             "1. Set your work duration (default: 25 min)\n"
                             "2. Click Start to begin\n"
                             "3. Focus on your task\n"
                             "4. Take a break when timer ends\n\n"
                             "Keyboard shortcuts:\n"
                             "- Space: Start/Pause timer\n"
                             "- R: Reset timer\n");

  // Page 8: Markdown Mode
  Note n8 = manager->createNote("Markdown Example");
  manager->updateNoteContent(n8.id(), "markdown\n"
                                      "# Markdown Mode\n\n"
                                      "## Text Formatting\n"
                                      "**Bold text** and *italic text*\n"
                                      "~~Strikethrough~~ and `inline code`\n\n"
                                      "## Lists\n"
                                      "- Unordered item 1\n"
                                      "- Unordered item 2\n"
                                      "  - Nested item\n\n"
                                      "1. Ordered item 1\n"
                                      "2. Ordered item 2\n\n"
                                      "## Code Block\n"
                                      "```python\n"
                                      "print('Hello, LinNote!')\n"
                                      "```\n\n"
                                      "> This is a blockquote\n\n"
                                      "[Link text](https://example.com)\n");

  // Set current to first page (Calculator)
  manager->setCurrentIndex(0);
}
