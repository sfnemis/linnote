#ifndef LINNOTE_TIMER_H
#define LINNOTE_TIMER_H

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

enum class TimerType {
  Stopwatch, // Count up from 0
  Countdown, // Count down from set time
  Pomodoro   // 25min work / 5min break cycles
};

class TimerWidget : public QWidget {
  Q_OBJECT

public:
  explicit TimerWidget(QWidget *parent = nullptr);
  ~TimerWidget() override = default;

  void setType(TimerType type);
  TimerType type() const { return m_type; }

  void start();
  void pause();
  void reset();
  void toggle();
  void setCountdownMinutes(int minutes);
  void setCountdownSeconds(int seconds); // Direct seconds support
  void setCountdownName(const QString &name);
  void setCustomPomodoro(int workMin, int breakMin);
  void startBreatheMode();

  bool isRunning() const { return m_running; }
  int elapsedSeconds() const { return m_elapsedSeconds; }

signals:
  void finished();
  void tick(int seconds);
  void closeRequested(); // User wants to close the timer

private slots:
  void onTick();
  void onBreatheTick();

private:
  void updateDisplay();
  void updatePomodoroStatus();
  QString formatTime(int totalSeconds);
  void setupUI();

  QTimer *m_timer;
  QLabel *m_statusLabel; // Shows mode (Stopwatch/Countdown) or Work/Break
  QLabel *m_timeDisplay;
  QLabel *m_sessionLabel; // Shows #1, #2 for Pomodoro
  QPushButton *m_startBtn;
  QPushButton *m_resetBtn;
  QPushButton *m_closeBtn;
  QSpinBox *m_minutesSpin;

  TimerType m_type;
  int m_elapsedSeconds;
  int m_targetSeconds;
  bool m_running;
  QString m_countdownName;

  // Pomodoro
  int m_pomodoroWorkMinutes = 25;
  int m_pomodoroBreakMinutes = 5;
  bool m_pomodoroInBreak = false;
  int m_pomodoroCount = 0;

  // Breathe mode
  int m_breathePhase = 0; // 0=inhale, 1=hold, 2=exhale
  int m_breatheCounter = 0;
};

#endif // LINNOTE_TIMER_H
