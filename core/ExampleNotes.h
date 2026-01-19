#ifndef LINNOTE_EXAMPLENOTES_H
#define LINNOTE_EXAMPLENOTES_H

class NoteManager;

/**
 * @brief Creates example notes demonstrating LinNote features
 *
 * Called on first launch if user opts in.
 * Creates 8 pages: calc, list, sum, avg, count, code, timer, markdown
 */
class ExampleNotes {
public:
  /**
   * @brief Create all example notes
   * @param manager NoteManager to create notes in
   */
  static void createExamples(NoteManager *manager);
};

#endif // LINNOTE_EXAMPLENOTES_H
