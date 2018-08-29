#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "datastructures.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

private:
    int load_track(std::string path);
    int stream_component_open(int stream_index);

private Q_SLOTS:
    void button_open_clicked();
    void button_play_clicked();
    void button_stop_clicked();
    void button_exit_clicked();
    void radio_enable_clicked();
    void radio_disable_clicked();
    void radio_equalizer_clicked();
    void radio_low_clicked();
    void radio_high_clicked();
    void radio_band_clicked();
    void knob_master_changed();
    void knob_balance_changed();

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

    void main_finished();
    void decode_finished();

private:
    enum FilterType {EQUALIZER, LOW_PASS, HIGH_PASS, BAND_PASS, BAND_STOP};
    FilterType _filter_type;

    Ui::MainWindow *ui;
    TrackState _track_state;
    FilterState _filter_state;
    UserData _user_data;

    int _master_volume;
    int _balance;
    int _order;
    int _width;

    bool _playback_enable;
    bool _playback_stop;
    bool _track_loaded;
};

#endif // MAINWINDOW_H
