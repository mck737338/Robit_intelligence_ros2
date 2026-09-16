#ifndef hw1_MAIN_WINDOW_H
#define hw1_MAIN_WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QVector>
#include <QKeyEvent>
#include "QIcon"
#include "qnode.hpp"
#include "ui_mainwindow.h"

class QProcess;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    QNode* qnode;

private:
    // 도형을 이루는 한 구간: 지정한 선속도/각속도로 움직이며,
    // 실제 이동 거리 또는 실제 회전 각도가 target_value에 도달하면 종료
    struct MotionSegment
    {
        enum class TargetType { Distance, Angle };
        double linear;
        double angular;
        TargetType target_type;
        double target_value;
    };

    // 현재 이동을 유발하고 있는 입력(키보드 또는 버튼)의 방향.
    // 키/버튼 입력을 하나로 통합해 다루기 위해 사용
    enum class Direction { None, W, A, S, D };

protected:
    // 키보드 W/A/S/D 입력을 감지해 눌려 있는 동안 이동시키기 위한 오버라이드
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void on_thick_rangeChanged(int min, int max);
    void on_thick_valueChanged(int value);
    void on_thick_parameter_textChanged();
    void on_colorTab_toggled(bool checked);

    void on_triangle_clicked();
    void on_square_clicked();
    void on_circle_clicked();

    // 터틀의 실제 pose가 갱신될 때 호출되어 진행 중인 구간의 도달 여부를 판정
    void onPoseUpdated(double x, double y, double theta);

private:
    Ui::MainWindowDesign* ui;
    void closeEvent(QCloseEvent* event);

    // 키보드/버튼 어느 쪽으로 눌렀든 같은 경로로 처리되는 방향 이동 시작/종료
    void setupWasdButtons();
    void beginDirection(Direction dir);
    void endDirection(Direction dir);

    // 버튼/슬라이더/텍스트박스가 키보드 포커스를 가져가지 않도록 하여
    // MainWindow가 항상 W/A/S/D 키를 받을 수 있게 함
    void disableChildFocus();

    Direction activeDirection_ = Direction::None;
    QTimer* moveHoldTimer_ = nullptr;
    double activeLinear_ = 0.0;
    double activeAngular_ = 0.0;

    // 실제 위치/각도를 측정하며 목표에 도달할 때까지 명령을 유지하는 도형 그리기
    void startMotionSequence(const QVector<MotionSegment>& segments);
    void beginNextMotionSegment();
    void stopMotionSequence();
    QVector<MotionSegment> buildTriangleSegments() const;
    QVector<MotionSegment> buildSquareSegments() const;
    QVector<MotionSegment> buildCircleSegments() const;

    QVector<MotionSegment> motionQueue_;
    int motionIndex_ = 0;
    bool motionSegmentActive_ = false;
    // 도형 그리기가 요청되었지만 아직 첫 pose를 받지 못해 대기 중인 상태
    bool pendingSequenceStart_ = false;
    double segStartX_ = 0.0, segStartY_ = 0.0;
    double segAccumulatedAngle_ = 0.0;
    double lastTheta_ = 0.0;

    // 현재 진행 중인 구간의 목표 속도. 구간이 활성화된 동안
    // segmentRepublishTimer_가 이 값을 주기적으로 재전송한다.
    double segLinear_ = 0.0;
    double segAngular_ = 0.0;
    QTimer* segmentRepublishTimer_ = nullptr;

    // 실제 pose를 한 번이라도 수신했는지 여부.
    // false인 동안은 curX_/curY_/curTheta_가 신뢰할 수 없는 초기값이므로
    // 도형 그리기 구간을 시작하지 않음
    bool havePose_ = false;
    double curX_ = 0.0, curY_ = 0.0, curTheta_ = 0.0;

    void createColorPalette();
    void applyPen();
    void onColorSelected(int r, int g, int b);
    void updateColorTabSwatch();
    QWidget* colorPalette_ = nullptr;
    int penR_ = 0;
    int penG_ = 0;
    int penB_ = 255;
    int penWidth_ = 3;
    bool updatingThickText_ = false;

    void launchTurtlesim();
    QProcess* turtlesimProcess_ = nullptr;
};

#endif  // hw1_MAIN_WINDOW_H