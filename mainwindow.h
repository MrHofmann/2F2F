#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private Q_SLOTS:
    void button_open_clicked();
    void button_play_clicked();
    void button_stop_clicked();
    void button_exit_clicked();
    void radio_equilizer_clicked();
    void radio_low_clicked();
    void radio_high_clicked();
    void radio_custom_clicked();
    void playback_finished();
    void enable_filter();
    void line_fmin_clicked();
    void line_fmax_clicked();
    void knob_master_changed();
    void knob_balance_changed();
    void slider_gain_changed();
    void slider_gain0_changed();
    void slider_gain1_changed();
    void slider_gain2_changed();
    void slider_gain3_changed();
    void slider_gain4_changed();
    void slider_gain5_changed();
    void slider_gain6_changed();
    void slider_gain7_changed();
    void slider_gain8_changed();
    void slider_gain9_changed();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
