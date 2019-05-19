#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <qglviewer.h>
#include "gl_robot.h"
#include "gl_viewer.h"
#include "my_utils.h"

#include <QMainWindow>
#include <QtGui>
#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QSlider>

namespace Ui {
class MainWindow;
}

#define ROW_LAST_INFO       3
#define ROW_CURRENT_MATRIX  10
#define ROW_EFFECTOR_MATRIX 15

enum TAB {E0_TAB_FK, E1_TAB_IK};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Viewer viewer;

    void slotFKanglesSliderMoved();
    void slotFKanglesSliderReleased();
    void slotFKexercise();
    void slotIKtest();

    void updateInfoMatrixAtRow(GLdouble *m, int row, QString sName);
    void updateInfoRobotPos();
    void updateInfoRobotBones();


public slots:
    void slotUpdateFromFABRIKBoneAngle(int bone, double angleDegree);

signals:
    void mySignal();

private slots:    
    void on_pushButtonFKexercise_clicked();
    void on_checkBoxShowInfo_stateChanged(int arg1);
    void on_tabWidget_currentChanged(int index);
    void on_checkBoxShowFABRIKvec_stateChanged(int arg1);
    void on_pushButtonIKtest_clicked();

private:
    Ui::MainWindow *ui;

    QTimer timerFKexercise;
    QTimer timerIKtest;

};

#endif // MAINWINDOW_H
