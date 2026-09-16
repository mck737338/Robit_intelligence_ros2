#include <cmath>
#include <algorithm>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColor>
#include <QProcess>

#include "../include/hw1/main_window.hpp"

namespace
{
constexpr double kLinearSpeed = 2.0;     // WASD 및 도형 변 이동 속도 (m/s)
constexpr double kAngularSpeed = 2.0;    // WASD 제자리 회전 속도 (rad/s)
constexpr double kShapeTurnRate = 1.0;   // 도형 모서리 회전 각속도 (rad/s)
constexpr double kShapeSideLength = 2.4; // 도형 한 변의 길이 (m)
constexpr double kCircleLinear = 1.5;    // 원 그리기 선속도 (m/s)
constexpr double kCircleAngular = 0.5;   // 원 그리기 각속도 (rad/s)
constexpr int kRepublishIntervalMs = 50; // 속도 명령 재전송 주기
}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindowDesign)
{
    ui->setupUi(this);

    QIcon icon("://ros-icon.png");
    this->setWindowIcon(icon);

    // 자식 위젯이 클릭 시 키보드 포커스를 가져가면 MainWindow::keyPressEvent가
    // 더 이상 호출되지 않으므로, MainWindow만 키보드 포커스를 갖도록 설정
    disableChildFocus();
    this->setFocusPolicy(Qt::StrongFocus);
    this->setFocus();

    qnode = new QNode();
    QObject::connect(qnode, SIGNAL(rosShutDown()), this, SLOT(close()));
    connect(qnode, &QNode::poseUpdated, this, &MainWindow::onPoseUpdated);

    launchTurtlesim();
    setupWasdButtons();

    createColorPalette();
    updateColorTabSwatch();
    on_thick_valueChanged(ui->thick->value());

    // turtlesim 프로세스가 뜨는 데 시간이 걸리므로 펜 설정을 두 번 재시도
    QTimer::singleShot(800, this, &MainWindow::applyPen);
    QTimer::singleShot(2000, this, &MainWindow::applyPen);
}

void MainWindow::disableChildFocus()
{
    ui->buttonW->setFocusPolicy(Qt::NoFocus);
    ui->buttonA->setFocusPolicy(Qt::NoFocus);
    ui->buttonS->setFocusPolicy(Qt::NoFocus);
    ui->buttonD->setFocusPolicy(Qt::NoFocus);
    ui->triangle->setFocusPolicy(Qt::NoFocus);
    ui->square->setFocusPolicy(Qt::NoFocus);
    ui->circle->setFocusPolicy(Qt::NoFocus);
    ui->thick->setFocusPolicy(Qt::NoFocus);
    ui->colorTab->setFocusPolicy(Qt::NoFocus);
    ui->thick_parameter->setFocusPolicy(Qt::ClickFocus);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    qnode->stopMoving();
    if (turtlesimProcess_ && turtlesimProcess_->state() != QProcess::NotRunning)
    {
        turtlesimProcess_->terminate();
        turtlesimProcess_->waitForFinished(1000);
    }
    QMainWindow::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::launchTurtlesim()
{
    turtlesimProcess_ = new QProcess(this);
    turtlesimProcess_->start("ros2", QStringList() << "run" << "turtlesim" << "turtlesim_node");
}

/*****************************************************************************
** WASD movement (keyboard + button, 둘 다 눌려 있는 동안만 이동)
*****************************************************************************/

void MainWindow::setupWasdButtons()
{
    connect(ui->buttonW, &QPushButton::pressed, this, [this]() { beginDirection(Direction::W); });
    connect(ui->buttonW, &QPushButton::released, this, [this]() { endDirection(Direction::W); });

    connect(ui->buttonS, &QPushButton::pressed, this, [this]() { beginDirection(Direction::S); });
    connect(ui->buttonS, &QPushButton::released, this, [this]() { endDirection(Direction::S); });

    connect(ui->buttonA, &QPushButton::pressed, this, [this]() { beginDirection(Direction::A); });
    connect(ui->buttonA, &QPushButton::released, this, [this]() { endDirection(Direction::A); });

    connect(ui->buttonD, &QPushButton::pressed, this, [this]() { beginDirection(Direction::D); });
    connect(ui->buttonD, &QPushButton::released, this, [this]() { endDirection(Direction::D); });
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // OS가 보내는 키 반복 이벤트는 무시 (지속 이동은 moveHoldTimer_가 담당)
    if (event->isAutoRepeat())
    {
        return;
    }

    switch (event->key())
    {
        case Qt::Key_W: beginDirection(Direction::W); return;
        case Qt::Key_A: beginDirection(Direction::A); return;
        case Qt::Key_S: beginDirection(Direction::S); return;
        case Qt::Key_D: beginDirection(Direction::D); return;
        default: break;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
    {
        return;
    }

    switch (event->key())
    {
        case Qt::Key_W: endDirection(Direction::W); return;
        case Qt::Key_A: endDirection(Direction::A); return;
        case Qt::Key_S: endDirection(Direction::S); return;
        case Qt::Key_D: endDirection(Direction::D); return;
        default: break;
    }
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::beginDirection(Direction dir)
{
    // 수동 조작이 시작되면 진행 중이던 도형 그리기는 중단
    stopMotionSequence();

    activeDirection_ = dir;
    switch (dir)
    {
        case Direction::W: activeLinear_ = kLinearSpeed;  activeAngular_ = 0.0;          break;
        case Direction::S: activeLinear_ = -kLinearSpeed; activeAngular_ = 0.0;          break;
        case Direction::A: activeLinear_ = 0.0;           activeAngular_ = kAngularSpeed;  break;
        case Direction::D: activeLinear_ = 0.0;           activeAngular_ = -kAngularSpeed; break;
        default:           activeLinear_ = 0.0;           activeAngular_ = 0.0;          break;
    }

    qnode->publishVelocity(activeLinear_, activeAngular_);

    if (!moveHoldTimer_)
    {
        moveHoldTimer_ = new QTimer(this);
        connect(moveHoldTimer_, &QTimer::timeout, this, [this]() {
            qnode->publishVelocity(activeLinear_, activeAngular_);
        });
    }
    // 키/버튼이 눌려 있는 동안 명령을 주기적으로 재전송하여 유지
    moveHoldTimer_->start(kRepublishIntervalMs);
}

void MainWindow::endDirection(Direction dir)
{
    // 현재 이동을 유발한 입력이 아니면 무시
    if (activeDirection_ != dir)
    {
        return;
    }

    activeDirection_ = Direction::None;
    if (moveHoldTimer_)
    {
        moveHoldTimer_->stop();
    }
    qnode->stopMoving();
}

/*****************************************************************************
** Shape drawing
*****************************************************************************/

void MainWindow::onPoseUpdated(double x, double y, double theta)
{
    curX_ = x;
    curY_ = y;
    curTheta_ = theta;

    if (!havePose_)
    {
        // 첫 pose를 받은 시점의 실제 좌표를 기준으로만 거리/각도를 측정해야 하므로,
        // 대기 중이던 도형 그리기가 있다면 지금 첫 구간을 시작
        havePose_ = true;
        if (pendingSequenceStart_)
        {
            pendingSequenceStart_ = false;
            beginNextMotionSegment();
        }
    }

    if (!motionSegmentActive_ || motionIndex_ >= motionQueue_.size())
    {
        return;
    }

    const MotionSegment& seg = motionQueue_[motionIndex_];
    bool done = false;

    if (seg.target_type == MotionSegment::TargetType::Distance)
    {
        const double dx = curX_ - segStartX_;
        const double dy = curY_ - segStartY_;
        const double dist = std::sqrt(dx * dx + dy * dy);
        done = dist >= seg.target_value;
    }
    else
    {
        // -pi ~ pi 범위로 정규화한 각도 변화량을 누적해 총 회전각을 구함
        double delta = theta - lastTheta_;
        while (delta > M_PI) delta -= 2.0 * M_PI;
        while (delta < -M_PI) delta += 2.0 * M_PI;
        segAccumulatedAngle_ += std::fabs(delta);
        done = segAccumulatedAngle_ >= seg.target_value;
    }

    lastTheta_ = theta;

    if (done)
    {
        motionSegmentActive_ = false;
        ++motionIndex_;
        beginNextMotionSegment();
    }
}

void MainWindow::startMotionSequence(const QVector<MotionSegment>& segments)
{
    // 도형 그리기가 시작되면 진행 중이던 수동 조작(WASD)은 중단
    activeDirection_ = Direction::None;
    if (moveHoldTimer_)
    {
        moveHoldTimer_->stop();
    }

    qnode->stopMoving();
    motionQueue_ = segments;
    motionIndex_ = 0;
    motionSegmentActive_ = false;

    if (!havePose_)
    {
        // 실제 pose를 아직 모르는 상태이므로 지금 시작하면 잘못된 기준점(0,0)으로
        // 거리를 계산하게 됨. 첫 pose가 도착할 때 onPoseUpdated에서 시작하도록 대기
        pendingSequenceStart_ = true;
        return;
    }

    beginNextMotionSegment();
}

void MainWindow::beginNextMotionSegment()
{
    if (motionIndex_ >= motionQueue_.size())
    {
        stopMotionSequence();
        return;
    }

    const MotionSegment& seg = motionQueue_[motionIndex_];
    segStartX_ = curX_;
    segStartY_ = curY_;
    segAccumulatedAngle_ = 0.0;
    lastTheta_ = curTheta_;
    motionSegmentActive_ = true;

    segLinear_ = seg.linear;
    segAngular_ = seg.angular;
    qnode->publishVelocity(segLinear_, segAngular_);

    if (!segmentRepublishTimer_)
    {
        segmentRepublishTimer_ = new QTimer(this);
        connect(segmentRepublishTimer_, &QTimer::timeout, this, [this]() {
            qnode->publishVelocity(segLinear_, segAngular_);
        });
    }
    // 구간이 진행되는 동안 같은 속도 명령을 주기적으로 재전송하여 유지
    segmentRepublishTimer_->start(kRepublishIntervalMs);
}

void MainWindow::stopMotionSequence()
{
    motionSegmentActive_ = false;
    pendingSequenceStart_ = false;
    motionQueue_.clear();
    motionIndex_ = 0;

    if (segmentRepublishTimer_)
    {
        segmentRepublishTimer_->stop();
    }
    qnode->stopMoving();
}

QVector<MainWindow::MotionSegment> MainWindow::buildTriangleSegments() const
{
    const double exteriorAngle = 2.0 * M_PI / 3.0;  // 120도
    QVector<MotionSegment> segments;
    for (int i = 0; i < 3; ++i)
    {
        segments.push_back({kLinearSpeed, 0.0, MotionSegment::TargetType::Distance, kShapeSideLength});
        segments.push_back({0.0, kShapeTurnRate, MotionSegment::TargetType::Angle, exteriorAngle});
    }
    return segments;
}

QVector<MainWindow::MotionSegment> MainWindow::buildSquareSegments() const
{
    const double exteriorAngle = M_PI / 2.0;  // 90도
    QVector<MotionSegment> segments;
    for (int i = 0; i < 4; ++i)
    {
        segments.push_back({kLinearSpeed, 0.0, MotionSegment::TargetType::Distance, kShapeSideLength});
        segments.push_back({0.0, kShapeTurnRate, MotionSegment::TargetType::Angle, exteriorAngle});
    }
    return segments;
}

QVector<MainWindow::MotionSegment> MainWindow::buildCircleSegments() const
{
    // 실제 회전각이 2π(한 바퀴)에 도달할 때까지 같은 선속도/각속도를 유지
    return {{kCircleLinear, kCircleAngular, MotionSegment::TargetType::Angle, 2.0 * M_PI}};
}

void MainWindow::on_triangle_clicked()
{
    startMotionSequence(buildTriangleSegments());
}

void MainWindow::on_square_clicked()
{
    startMotionSequence(buildSquareSegments());
}

void MainWindow::on_circle_clicked()
{
    startMotionSequence(buildCircleSegments());
}

/*****************************************************************************
** Pen thickness
*****************************************************************************/

void MainWindow::on_thick_rangeChanged(int min, int max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
}

void MainWindow::on_thick_valueChanged(int value)
{
    penWidth_ = value;
    applyPen();

    updatingThickText_ = true;
    ui->thick_parameter->setPlainText(QString::number(value));
    updatingThickText_ = false;
}

void MainWindow::on_thick_parameter_textChanged()
{
    if (updatingThickText_)
    {
        return;
    }
    bool ok = false;
    const int value = ui->thick_parameter->toPlainText().trimmed().toInt(&ok);
    if (!ok)
    {
        return;
    }
    const int clamped = std::max(ui->thick->minimum(), std::min(ui->thick->maximum(), value));
    ui->thick->blockSignals(true);
    ui->thick->setValue(clamped);
    ui->thick->blockSignals(false);

    penWidth_ = clamped;
    applyPen();
}

/*****************************************************************************
** Pen color palette
*****************************************************************************/

void MainWindow::createColorPalette()
{
    struct NamedColor
    {
        const char* name;
        int r, g, b;
    };
    static const NamedColor colors[] = {
        {"red", 255, 0, 0},     {"green", 0, 200, 0},   {"blue", 0, 0, 255},
        {"black", 0, 0, 0},     {"yellow", 230, 200, 0}, {"cyan", 0, 200, 200},
        {"magenta", 200, 0, 200},
    };

    colorPalette_ = new QWidget(ui->centralwidget);
    colorPalette_->setGeometry(180, 410, 300, 40);
    QHBoxLayout* layout = new QHBoxLayout(colorPalette_);
    layout->setContentsMargins(0, 0, 0, 0);

    for (const auto& c : colors)
    {
        QPushButton* button = new QPushButton(colorPalette_);
        button->setFixedSize(30, 30);
        button->setFocusPolicy(Qt::NoFocus);
        button->setStyleSheet(
            QString("background-color: rgb(%1,%2,%3); border: 1px solid #333;")
                .arg(c.r)
                .arg(c.g)
                .arg(c.b));
        layout->addWidget(button);

        const int r = c.r, g = c.g, b = c.b;
        QObject::connect(button, &QPushButton::clicked, this, [this, r, g, b]() {
            onColorSelected(r, g, b);
            ui->colorTab->setChecked(false);
        });
    }

    colorPalette_->hide();
}

void MainWindow::on_colorTab_toggled(bool checked)
{
    if (colorPalette_)
    {
        colorPalette_->setVisible(checked);
    }
}

void MainWindow::onColorSelected(int r, int g, int b)
{
    penR_ = r;
    penG_ = g;
    penB_ = b;
    applyPen();
    updateColorTabSwatch();
}

void MainWindow::applyPen()
{
    qnode->setPen(static_cast<uint8_t>(penR_), static_cast<uint8_t>(penG_),
                  static_cast<uint8_t>(penB_), static_cast<uint8_t>(penWidth_), false);
}

void MainWindow::updateColorTabSwatch()
{
    ui->colorTab->setStyleSheet(
        QString("background-color: rgb(%1,%2,%3); border: 1px solid #333;")
            .arg(penR_)
            .arg(penG_)
            .arg(penB_));
}