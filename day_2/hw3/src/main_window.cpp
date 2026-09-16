#include "../include/hw3/main_window.hpp"
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindowDesign)
{
    ui->setupUi(this);

    qnode_ = new QNode();

    connect(qnode_, &QNode::stringUpdated, this, &MainWindow::onStringUpdated);
    connect(qnode_, &QNode::intUpdated, this, &MainWindow::onIntUpdated);
    connect(qnode_, &QNode::floatUpdated, this, &MainWindow::onFloatUpdated);
    connect(qnode_, &QNode::restartResult, this, &MainWindow::onRestartResult);

    // 아직 메시지를 한 번도 받지 못한 초기 상태
    ui->stringValueLabel->setText("(대기 중)");
    ui->intValueLabel->setText("(대기 중)");
    ui->floatValueLabel->setText("(대기 중)");
    setTopicStatus(ui->stringStatusLabel, false);
    setTopicStatus(ui->intStatusLabel, false);
    setTopicStatus(ui->floatStatusLabel, false);

    // 0.5초마다 각 토픽의 타임아웃(2초) 여부를 검사
    timeoutCheckTimer_ = new QTimer(this);
    connect(timeoutCheckTimer_, &QTimer::timeout, this, &MainWindow::checkTimeouts);
    timeoutCheckTimer_->start(500);
}

MainWindow::~MainWindow()
{
    delete qnode_;
    delete ui;
}

void MainWindow::setTopicStatus(QLabel* statusLabel, bool alive)
{
    if (alive)
    {
        statusLabel->setText("정상");
        statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
    else
    {
        statusLabel->setText("응답 없음");
        statusLabel->setStyleSheet("color: red; font-weight: bold;");
    }
}

void MainWindow::onStringUpdated(const QString& value)
{
    ui->stringValueLabel->setText(value);
    stringLastSeen_.start();
    stringSeenOnce_ = true;
    setTopicStatus(ui->stringStatusLabel, true);
}

void MainWindow::onIntUpdated(int value)
{
    ui->intValueLabel->setText(QString::number(value));
    intLastSeen_.start();
    intSeenOnce_ = true;
    setTopicStatus(ui->intStatusLabel, true);
}

void MainWindow::onFloatUpdated(double value)
{
    ui->floatValueLabel->setText(QString::number(value, 'f', 3));
    floatLastSeen_.start();
    floatSeenOnce_ = true;
    setTopicStatus(ui->floatStatusLabel, true);
}

void MainWindow::checkTimeouts()
{
    // 메시지를 한 번이라도 받은 적이 있는 토픽에 한해 무응답 시간을 검사한다.
    if (stringSeenOnce_ && stringLastSeen_.elapsed() > kTimeoutMs)
    {
        setTopicStatus(ui->stringStatusLabel, false);
    }
    if (intSeenOnce_ && intLastSeen_.elapsed() > kTimeoutMs)
    {
        setTopicStatus(ui->intStatusLabel, false);
    }
    if (floatSeenOnce_ && floatLastSeen_.elapsed() > kTimeoutMs)
    {
        setTopicStatus(ui->floatStatusLabel, false);
    }
}

void MainWindow::on_restartButton_clicked()
{
    ui->restartButton->setEnabled(false);
    ui->restartButton->setText("재시작 요청 중...");
    qnode_->requestRestart();
}

void MainWindow::onRestartResult(bool success, const QString& message)
{
    ui->restartButton->setEnabled(true);
    ui->restartButton->setText("퍼블리셔 재시작");

    if (success)
    {
        QMessageBox::information(this, "재시작 성공", message);
    }
    else
    {
        QMessageBox::warning(this, "재시작 실패", message);
    }
}
