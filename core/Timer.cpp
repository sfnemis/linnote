#include "Timer.h"
#include "Settings.h"

TimerWidget::TimerWidget(QWidget *parent)
    : QWidget(parent), m_timer(new QTimer(this)), m_type(TimerType::Stopwatch),
      m_elapsedSeconds(0), m_targetSeconds(0), m_running(false) {
  connect(m_timer, &QTimer::timeout, this, &TimerWidget::onTick);
  setupUI();
}

void TimerWidget::setupUI() {
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(12, 8, 12, 8);
  layout->setSpacing(12);

  // Status label (Work/Break for Pomodoro, Timer type for others)
  m_statusLabel = new QLabel("⏱", this);
  m_statusLabel->setObjectName("timerStatusLabel");
  m_statusLabel->setMinimumWidth(130); // Wide enough for emoji + text
  layout->addWidget(m_statusLabel);

  // Time display
  m_timeDisplay = new QLabel("00:00", this);
  m_timeDisplay->setObjectName("timerDisplay");
  m_timeDisplay->setAlignment(Qt::AlignCenter);
  m_timeDisplay->setMinimumWidth(100);
  layout->addWidget(m_timeDisplay);

  // Session counter (for Pomodoro)
  m_sessionLabel = new QLabel("", this);
  m_sessionLabel->setObjectName("timerSessionLabel");
  m_sessionLabel->setFixedWidth(40);
  layout->addWidget(m_sessionLabel);

  // Spacer
  layout->addStretch();

  // Minutes input (for countdown)
  m_minutesSpin = new QSpinBox(this);
  m_minutesSpin->setObjectName("timerSpinBox");
  m_minutesSpin->setRange(1, 120);
  m_minutesSpin->setValue(25);
  m_minutesSpin->setSuffix(" min");
  m_minutesSpin->setFixedWidth(85);
  m_minutesSpin->hide();
  layout->addWidget(m_minutesSpin);

  // Start/Pause button
  m_startBtn = new QPushButton(this);
  m_startBtn->setObjectName("timerStartBtn");
  m_startBtn->setText("Start");
  m_startBtn->setFixedSize(85, 32);
  m_startBtn->setCursor(Qt::PointingHandCursor);
  connect(m_startBtn, &QPushButton::clicked, this, &TimerWidget::toggle);
  layout->addWidget(m_startBtn);

  // Reset button
  m_resetBtn = new QPushButton(this);
  m_resetBtn->setObjectName("timerResetBtn");
  m_resetBtn->setText("Reset");
  m_resetBtn->setFixedSize(60, 32);
  m_resetBtn->setCursor(Qt::PointingHandCursor);
  connect(m_resetBtn, &QPushButton::clicked, this, &TimerWidget::reset);
  layout->addWidget(m_resetBtn);

  // Close button
  m_closeBtn = new QPushButton(this);
  m_closeBtn->setObjectName("timerCloseBtn");
  m_closeBtn->setText("✕");
  m_closeBtn->setFixedSize(32, 32);
  m_closeBtn->setCursor(Qt::PointingHandCursor);
  m_closeBtn->setToolTip(tr("Close timer"));
  connect(m_closeBtn, &QPushButton::clicked, this, [this]() {
    reset();
    hide();
    emit closeRequested();
  });
  layout->addWidget(m_closeBtn);

  setFixedHeight(52);
}

void TimerWidget::setType(TimerType type) {
  m_type = type;
  reset();

  if (type == TimerType::Stopwatch) {
    m_minutesSpin->hide();
    m_statusLabel->setText("⏱ Stopwatch");
    m_sessionLabel->hide();
  } else if (type == TimerType::Countdown) {
    m_minutesSpin->show();
    m_minutesSpin->setValue(5);
    m_statusLabel->setText("⏳ Countdown");
    m_sessionLabel->hide();
  } else if (type == TimerType::Pomodoro) {
    m_minutesSpin->hide();
    m_pomodoroWorkMinutes = Settings::instance()->pomodoroWorkMinutes();
    m_pomodoroBreakMinutes = Settings::instance()->pomodoroBreakMinutes();
    m_targetSeconds = m_pomodoroWorkMinutes * 60;
    m_pomodoroInBreak = false;
    m_pomodoroCount = 0;
    updatePomodoroStatus();
    updateDisplay();
  }
}

void TimerWidget::updatePomodoroStatus() {
  QString workLabel = Settings::instance()->pomodoroWorkLabel();
  QString breakLabel = Settings::instance()->pomodoroBreakLabel();

  if (m_pomodoroInBreak) {
    m_statusLabel->setText(QString("☕ %1").arg(breakLabel));
  } else {
    m_statusLabel->setText(QString("💪 %1").arg(workLabel));
  }
  m_sessionLabel->setText(QString("#%1").arg(m_pomodoroCount + 1));
  m_sessionLabel->show();
}

void TimerWidget::start() {
  if (m_type == TimerType::Countdown) {
    if (m_elapsedSeconds == 0) {
      m_targetSeconds = m_minutesSpin->value() * 60;
      m_elapsedSeconds = m_targetSeconds;
    }
  } else if (m_type == TimerType::Pomodoro) {
    if (m_elapsedSeconds == 0) {
      m_elapsedSeconds = m_targetSeconds;
    }
  }

  m_running = true;
  m_timer->start(1000);
  m_startBtn->setText("Pause");
  updateDisplay();
}

void TimerWidget::pause() {
  m_running = false;
  m_timer->stop();
  m_startBtn->setText("Resume");
}

void TimerWidget::reset() {
  m_running = false;
  m_timer->stop();
  m_elapsedSeconds = 0;

  if (m_type == TimerType::Pomodoro) {
    m_pomodoroInBreak = false;
    m_pomodoroCount = 0;
    m_targetSeconds = m_pomodoroWorkMinutes * 60;
    updatePomodoroStatus();
  }

  m_startBtn->setText("Start");
  updateDisplay();
}

void TimerWidget::toggle() {
  if (m_running) {
    pause();
  } else {
    start();
  }
}

void TimerWidget::setCountdownMinutes(int minutes) {
  m_minutesSpin->setValue(minutes);
  m_targetSeconds = minutes * 60;
  m_elapsedSeconds = m_targetSeconds;
  updateDisplay();
}

void TimerWidget::setCountdownSeconds(int seconds) {
  m_minutesSpin->hide();
  m_targetSeconds = seconds;
  m_elapsedSeconds = m_targetSeconds;
  updateDisplay();
}

void TimerWidget::setCountdownName(const QString &name) {
  m_countdownName = name;
  if (!name.isEmpty()) {
    m_statusLabel->setText(QString("⏳ %1").arg(name));
  }
}

void TimerWidget::setCustomPomodoro(int workMin, int breakMin) {
  m_type = TimerType::Pomodoro;
  m_pomodoroWorkMinutes = workMin;
  m_pomodoroBreakMinutes = breakMin;
  m_targetSeconds = workMin * 60;
  m_elapsedSeconds = m_targetSeconds;
  m_pomodoroInBreak = false;
  m_pomodoroCount = 0;
  m_minutesSpin->hide();
  updatePomodoroStatus();
  updateDisplay();
  show();
}

void TimerWidget::startBreatheMode() {
  m_type = TimerType::Stopwatch; // Reuse stopwatch timer
  m_breathePhase = 0;
  m_breatheCounter = 4; // Start with 4 seconds inhale
  m_elapsedSeconds = m_breatheCounter;
  m_minutesSpin->hide();
  m_sessionLabel->hide();
  m_statusLabel->setText("🌬️ Breathe In");
  updateDisplay();
  show();

  // Use a separate timer for breathing
  m_running = true;
  m_timer->start(1000);
  m_startBtn->setText("Pause");
}

void TimerWidget::onBreatheTick() {
  // This is called from onTick when in breathe mode
  // 4-7-8 breathing: 4s inhale, 7s hold, 8s exhale
  m_breatheCounter--;

  if (m_breatheCounter <= 0) {
    m_breathePhase = (m_breathePhase + 1) % 3;
    switch (m_breathePhase) {
    case 0: // Inhale
      m_breatheCounter = 4;
      m_statusLabel->setText("🌬️ Breathe In");
      break;
    case 1: // Hold
      m_breatheCounter = 7;
      m_statusLabel->setText("⏸️ Hold");
      break;
    case 2: // Exhale
      m_breatheCounter = 8;
      m_statusLabel->setText("💨 Breathe Out");
      break;
    }
  }

  m_elapsedSeconds = m_breatheCounter;
  updateDisplay();
}

void TimerWidget::onTick() {
  if (m_type == TimerType::Stopwatch) {
    m_elapsedSeconds++;
  } else {
    m_elapsedSeconds--;

    if (m_elapsedSeconds <= 0) {
      if (m_type == TimerType::Pomodoro) {
        m_pomodoroInBreak = !m_pomodoroInBreak;
        if (m_pomodoroInBreak) {
          // Work period finished! Celebrate!
          m_targetSeconds = m_pomodoroBreakMinutes * 60;
          emit finished(); // Trigger confetti + notification
        } else {
          // Break finished, back to work
          m_targetSeconds = m_pomodoroWorkMinutes * 60;
          m_pomodoroCount++;
        }
        m_elapsedSeconds = m_targetSeconds;
        updatePomodoroStatus();
      } else {
        // Countdown finished
        m_elapsedSeconds = 0;
        updateDisplay();
        pause();
        emit finished();
        return;
      }
    }
  }

  updateDisplay();
  emit tick(m_elapsedSeconds);
}

void TimerWidget::updateDisplay() {
  QString timeStr = formatTime(m_elapsedSeconds);
  m_timeDisplay->setText(timeStr);
}

QString TimerWidget::formatTime(int totalSeconds) {
  int hours = totalSeconds / 3600;
  int minutes = (totalSeconds % 3600) / 60;
  int seconds = totalSeconds % 60;

  if (hours > 0) {
    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
  } else {
    return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
  }
}
