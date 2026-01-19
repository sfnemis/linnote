#ifndef LINNOTE_CONFETTIWIDGET_H
#define LINNOTE_CONFETTIWIDGET_H

#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <QWidget>
#include <vector>

struct Particle {
  float x, y;
  float vx, vy;
  float rotation;
  float rotationSpeed;
  QColor color;
  int size;
};

class ConfettiWidget : public QWidget {
  Q_OBJECT

public:
  explicit ConfettiWidget(QWidget *parent = nullptr);
  void start();
  void stop();

protected:
  void paintEvent(QPaintEvent *event) override;

private slots:
  void animate();

private:
  void createParticles();

  std::vector<Particle> m_particles;
  QTimer *m_timer;
  int m_frameCount;
  bool m_running;

  static constexpr int PARTICLE_COUNT = 100;
  static constexpr int ANIMATION_DURATION_MS = 3000;
};

#endif // LINNOTE_CONFETTIWIDGET_H
