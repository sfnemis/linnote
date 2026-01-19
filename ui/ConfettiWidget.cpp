#include "ConfettiWidget.h"

ConfettiWidget::ConfettiWidget(QWidget *parent)
    : QWidget(parent), m_timer(new QTimer(this)), m_frameCount(0),
      m_running(false) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_NoSystemBackground);
  // Don't set window flags - we're an overlay child widget

  connect(m_timer, &QTimer::timeout, this, &ConfettiWidget::animate);
}

void ConfettiWidget::createParticles() {
  m_particles.clear();
  m_particles.reserve(PARTICLE_COUNT);

  QList<QColor> colors = {
      QColor("#f38ba8"), // Pink
      QColor("#fab387"), // Peach
      QColor("#f9e2af"), // Yellow
      QColor("#a6e3a1"), // Green
      QColor("#89b4fa"), // Blue
      QColor("#cba6f7"), // Mauve
      QColor("#f5c2e7"), // Pink
      QColor("#94e2d5")  // Teal
  };

  int w = width();
  auto *rng = QRandomGenerator::global();

  for (int i = 0; i < PARTICLE_COUNT; ++i) {
    Particle p;
    p.x = rng->bounded(w);
    p.y = -rng->bounded(50);
    p.vx = (rng->generateDouble() * 6.0) - 3.0; // -3 to 3
    p.vy = (rng->generateDouble() * 4.0) + 2.0; // 2 to 6
    p.rotation = rng->bounded(360);
    p.rotationSpeed = (rng->generateDouble() * 20.0) - 10.0; // -10 to 10
    p.color = colors[rng->bounded(static_cast<int>(colors.size()))];
    p.size = rng->bounded(6, 14);
    m_particles.push_back(p);
  }
}

void ConfettiWidget::start() {
  if (m_running)
    return;

  m_running = true;
  m_frameCount = 0;

  // Safely get parent size
  QWidget *p = parentWidget();
  if (p) {
    resize(p->size());
    move(0, 0);
  } else {
    resize(400, 300); // fallback
  }

  createParticles();
  show();
  raise();
  m_timer->start(16); // ~60 FPS
}

void ConfettiWidget::stop() {
  m_running = false;
  m_timer->stop();
  hide();
}

void ConfettiWidget::animate() {
  m_frameCount++;

  // Stop after duration
  if (m_frameCount > (ANIMATION_DURATION_MS / 16)) {
    stop();
    return;
  }

  int h = height();

  for (auto &p : m_particles) {
    p.x += p.vx;
    p.y += p.vy;
    p.vy += 0.1f; // gravity
    p.rotation += p.rotationSpeed;

    // Add some flutter
    p.vx += (QRandomGenerator::global()->generateDouble() * 0.4) - 0.2;
  }

  // Remove particles that are off screen
  m_particles.erase(
      std::remove_if(m_particles.begin(), m_particles.end(),
                     [h](const Particle &p) { return p.y > h + 50; }),
      m_particles.end());

  update();
}

void ConfettiWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  for (const auto &p : m_particles) {
    painter.save();
    painter.translate(p.x, p.y);
    painter.rotate(p.rotation);

    // Draw rectangular confetti
    painter.setBrush(p.color);
    painter.setPen(Qt::NoPen);
    painter.drawRect(-p.size / 2, -p.size / 4, p.size, p.size / 2);

    painter.restore();
  }
}
