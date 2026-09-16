#ifndef hw3_MAIN_WINDOW_H
#define hw3_MAIN_WINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QLabel>
#include "qnode.hpp"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // QNode에서 오는 토픽 갱신 시그널
    void onStringUpdated(const QString& value);
    void onIntUpdated(int value);
    void onFloatUpdated(double value);
    void onRestartResult(bool success, const QString& message);

    // 각 토픽의 응답 없음(timeout) 여부를 주기적으로 검사
    void checkTimeouts();

    // Qt 자동 슬롯 연결: 재시작 버튼
    void on_restartButton_clicked();

private:
    Ui::MainWindowDesign* ui;
    QNode* qnode_;
    QTimer* timeoutCheckTimer_;

    // 각 토픽이 마지막으로 갱신된 시각(경과 시간 측정용).
    // 이 시각으로부터 kTimeoutMs 이상 지나면 "응답 없음"으로 표시한다.
    QElapsedTimer stringLastSeen_;
    QElapsedTimer intLastSeen_;
    QElapsedTimer floatLastSeen_;
    bool stringSeenOnce_ = false;
    bool intSeenOnce_ = false;
    bool floatSeenOnce_ = false;

    static constexpr qint64 kTimeoutMs = 2000;  // 2초 이상 무응답 -> "응답 없음"

    void setTopicStatus(QLabel* statusLabel, bool alive);
};

#endif  // hw3_MAIN_WINDOW_H
