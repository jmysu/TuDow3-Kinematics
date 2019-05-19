/*
 * T3 robot forward/inverse kinematics  by DH parameters
 *
 *
 *
 *      by jmysu@yahoo.com
 *
 *
 *
 *
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

//using namespace qglviewer;

/*
 * QString sM4print(const GLdouble *m)
 *
 *      utility to print 4x4 matrix
 */
QString sM4x4Print(const GLdouble *m) {
    QString s,sOut;
    sOut="";
    for (int r=0;r<4;r++) {
        sOut += "|";
        for (int c=0;c<4;c++)
            sOut +=s.sprintf("%+7.3f", *(m+r*4+c));
        sOut += "|\n";
        }
    return sOut;
    }

QString sV3print(const Vec v) {
    QString s;
    s.sprintf("(%+5.3f, %5.3f,%5.3f)", v.x, v.y, v.z);
    return s;
}

/*
 * anglesVec(Vec v0, Vec v1)
 *
 *   get angle between two vectors
 *
 *   dot    = x1*x2 + y1*y2 + z1*z2    #between [x1, y1, z1] and [x2, y2, z2]
 *   lenSq1 = x1*x1 + y1*y1 + z1*z1
 *   lenSq2 = x2*x2 + y2*y2 + z2*z2
 *   angle = acos(dot/sqrt(lenSq1 * lenSq2))
 *
 */
double VertexAngle(Vec v1, Vec v2)
        {
            v1.normalize();
            v2.normalize();
            // |a x b | = |a||b|sin(theta)
            Vec v3 = v1 ^ v2;
            double dot = v1 * v2;
            //
            //Vec v3;
            //v3.x = v1.y*v2.z-v1.z*v2.y;
            //v3.y = v1.z*v2.x-v1.x*v2.z;
            //v3.z = v1.x*v2.y-v1.y*v2.x;
            //qDebug() << sV3print(v3);

            //here we compute magnitude of cross Product
            //double mCrossProd= qSqrt(v3.x*v3.x + v3.y*v3.y + v3.z*v3.z);
            double mCrossProd= v3.squaredNorm();
            double denom = v1.squaredNorm() * v2.squaredNorm();
            double theta = qAsin(mCrossProd/denom);
            if(v3.z>0) theta = 2*M_PI - theta;
//qDebug() << "theta" << qRadiansToDegrees(theta);
            double angle = qAcos(dot/denom);
//qDebug() << "angle" << qRadiansToDegrees(angle);
            return qRadiansToDegrees(angle);
        }

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(APP_NAME+QString(" V")+APP_VERSION);
    //Change default font for opengl
    QFont font("Courier New");
    font.setStyleHint(QFont::Monospace);
#ifdef Q_OS_MAC
    font.setPointSize(12);
    ui->centralWidget->setStyleSheet("QWidget{font-size:12px}");
#else
    font.setPointSize(9);
    ui->centralWidget->setStyleSheet("QWidget{font-size:9px}");
#endif
    QApplication::setFont(font);
    ui->centralWidget->setFont(font);

    //Attach QGLViewer to UI
    QLayout *layout = ui->horizontalLayoutQGL;
    layout->addWidget(&viewer); //Add QGLViewer widget to layout
    viewer.setGridIsDrawn();
    viewer.setAxisIsDrawn();
    //Adjust camera range, orientation
    viewer.camera()->frame()->setOrientation(Quaternion(qglviewer::Vec(1, 0.2, 0.2), M_PI/4)); //Init viewer in iso orientation
    viewer.camera()->setSceneRadius(5);
    viewer.camera()->fitSphere(Vec(0, 0, 2), 5);
    viewer.show();

    //Init FK UI
    QVBoxLayout *l=ui->verticalLayoutFKangles;
    for (int i=0; i<ROBOT_BONES; i++) {
        QLabel *lbl = new QLabel(slBoneName[i], this);
        l->addWidget(lbl);
        QSlider *slider = new QSlider(Qt::Horizontal, this);
        //slider->setRange(-180,180);
        slider->setRange(degBoneAngleMin[i], degBoneAngleMax[i]);
        if ((i==E0_BASE) || (i==E4_CLAW))
            slider->setValue(static_cast<int>(degBoneRotX[i]));
        else
            slider->setValue(static_cast<int>(degBoneAngleZ[i]));
        slider->setTickInterval(30);
        slider->setTickPosition(QSlider::TicksAbove);
        slider->setProperty("bone", i); //set individual bone id
        l->addWidget(slider);
        connect(slider, &QSlider::sliderMoved, this, [=](){slotFKanglesSliderMoved();});
        connect(slider, &QSlider::sliderReleased, this, [=](){slotFKanglesSliderReleased();});
        //update lbl with slider value
        QString s;
        s.sprintf("%+4d°", slider->value());
        lbl->setText(slBoneName[i]+ " "+s);
        }

    //Creat IK Robot bone angles
    QVBoxLayout *lb=ui->verticalLayoutBoneAngles;
    for (int i=0; i<ROBOT_BONES; i++) {
        QLabel *lbl = new QLabel(slBoneName[i], this);
        lb->addWidget(lbl);

        QString s;
        s.sprintf("%+4d°", 99);
        lbl->setText(slBoneName[i]+ " "+s);
        }

    //reset tab
    ui->tabWidget->setCurrentIndex(0);

    //Connect myRobot signal back to MainWindow slot
    connect(&viewer.myRobot, SIGNAL(sigSendBoneAngle(int,double)), this, SLOT(slotUpdateFromFABRIKBoneAngle(int,double)));

    //ui->checkBoxShowInfo->click();
    Dprintf("%s", "App started!");
}


MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * update matrix info. in QGLViewer at row on left side
 */
void MainWindow::updateInfoMatrixAtRow(GLdouble *m, int row, QString sName)
{
    viewer.addInfoLeft(row, sName);
    QString s, sOut;
    for (int r=0;r<4;r++) {
        sOut = "|";
        for (int c=0;c<4;c++)
            sOut +=s.sprintf("%+7.3f", *(m+r*4+c));
        sOut += "|";
        viewer.addInfoLeft(r+row+1, sOut);
    }
}

void MainWindow::updateInfoRobotPos()
{
    viewer.addInfoLeft(ROW_LAST_INFO-1, "Last Bone Positions @ Rotations:");
    for (int i = 0; i < ROBOT_BONES ; ++i) {
        QString s;
        Vec v       = viewer.myRobot.frame[i]->position();

        QLayoutItem *lItem = ui->verticalLayoutFKangles->itemAt((i*2)+1);
        QWidget *w = lItem->widget();
        QSlider *slider= dynamic_cast<QSlider*>(w);
        s.sprintf(" (%+5.3f,%+5.3f,%+5.3f) @%+4d°", v.x,v.y,v.z, slider->value());
        viewer.addInfoLeft(ROW_LAST_INFO+i, slBoneName[i]+s);
        }
}

/*
 * update bones info. in QGLViewer on left side
 */
void MainWindow::updateInfoRobotBones()
{
    int iAnglePreviousZ=0;
    for (int i = 0; i < ROBOT_BONES ; ++i) {
        QString s;
        Vec v       = viewer.myRobot.vLastPosition[i];
        int iAngle  = viewer.myRobot.iLastRotationDegrees[i];
        viewer.addInfoRight(ROW_LAST_INFO, "Last Bones Vectors:");
        if (i==E0_BASE) {
            s.sprintf("(%+5.3f,%+5.3f,%+5.3f)@X%+4d°", v.x,v.y,v.z, iAngle);
            viewer.addInfoRight(ROW_LAST_INFO+i, "EndE@X"+s);
            }
        else if ((i==E4_CLAW)) {
            s.sprintf("(%+5.3f,%+5.3f,%+5.3f)@X%+4d°", v.x,v.y,v.z, iAngle);
            viewer.addInfoRight(ROW_LAST_INFO+i, slBoneName[i]+s);
            }
        else {
            s.sprintf("(%+5.3f,%+5.3f,%+5.3f)@Z%+4d°|%+4d°", v.x,v.y,v.z, iAngle, iAngle-iAnglePreviousZ);
            viewer.addInfoRight(ROW_LAST_INFO+i, slBoneName[i]+s);
            iAnglePreviousZ = iAngle;
            }
        //Locate item in layoutBoneAngles
        QLayoutItem *lItem = ui->verticalLayoutBoneAngles->itemAt(i);
        QWidget *w = lItem->widget();
        QLabel *lbl= dynamic_cast<QLabel*>(w);
        lbl->setText(slBoneName[i]+ " "+s);
        }
    //Update base slide value when changes
    QLayoutItem *lItem0 = ui->verticalLayoutFKangles->itemAt(0);
    QWidget *w0 = lItem0->widget();
    QLabel *lbl= dynamic_cast<QLabel*>(w0);
    QLayoutItem *lItem1 = ui->verticalLayoutFKangles->itemAt(1);
    QWidget *w1 = lItem1->widget();
    QSlider *slider= dynamic_cast<QSlider*>(w1);
    int iValue = viewer.myRobot.iLastRotationDegrees[0];
    if (slider->value() != iValue ){
        //slider->setValue(iValue);
        QString s;
        //s.sprintf("%+4d°", slider->value());
        s.sprintf("%+4d°", iValue);
        lbl->setText(slBoneName[0]+ " "+s);
        }
}

/*
 * slot for update robot's angle with FABRIK bone angle
 */
void MainWindow::slotUpdateFromFABRIKBoneAngle(int bone, double angleDegree)
{
//qDebug() <<Q_FUNC_INFO << bone << angleDegree;
Dprintf("Bone%1d Angle>%4.0f",bone,angleDegree);

    QLayoutItem *lItem = ui->verticalLayoutFKangles->itemAt((bone*2)+1);
    QWidget *w = lItem->widget();
    QSlider *slider= dynamic_cast<QSlider*>(w);
    if (slider) {
        //The slider forces the value to be within the legal range: minimum <= value <= maximum.
        slider->setValue(qRound(angleDegree));
        slider->sliderMoved(slider->value());
        Dprintf("Bone%1d Angle=%4d\n",bone, slider->value());
        }
}

/*
 * slot for slider moved on FK
 */
void MainWindow::slotFKanglesSliderMoved()
{
//qDebug() << Q_FUNC_INFO;

    QSlider *slider = static_cast<QSlider *>(QObject::sender());
    int bone = slider->property("bone").toInt();
    Dprintf("Bone %1d value %4d", bone, slider->value());

    //Locate item in layout
    QLayoutItem *lItem = ui->verticalLayoutFKangles->itemAt(bone*2);
    QWidget *w = lItem->widget();
    QLabel *lbl= dynamic_cast<QLabel*>(w);
    //Update lbl with slider value
    QString s;
    s.sprintf("%+4d°", slider->value());
    lbl->setText(slBoneName[bone]+ " "+s);

    double rAngle = qDegreesToRadians((double)slider->value());

    //if (bone==0)
    //    qDebug().noquote() << slBoneName[0] <<    "@" << qRadiansToDegrees(viewer.myRobot.frame[0]->rotation().angle()) << sV3print(viewer.myRobot.frame[0]->rotation().axis());

    //if (rAngle<0)
    //    viewer.myRobot.frame[bone]->setRotation( Quaternion(-vecRotation[bone], qAbs(rAngle) ) );
    //else
    viewer.myRobot.frame[bone]->setRotation( Quaternion( vecRotation[bone], rAngle) );

    viewer.myRobot.idSelected = bone;
    viewer.update();

    //debug and Info display
    //viewer.myRobot.lastRotAxis[bone] = viewer.myRobot.frame[bone]->rotation().axis();

    //DebugPrint current world matrix after next rotation and next translation
    Frame *f = new Frame();
    f->setReferenceFrame(viewer.myRobot.frame[bone]);
    if (bone < ROBOT_BONES-1) {
        f->setRotation(viewer.myRobot.frame[bone+1]->rotation());
        f->setTranslation(viewer.myRobot.frame[bone+1]->translation());
        }
    //qDebug().noquote() << "World Matrix after" << slBoneName[bone] << endl <<sM4x4Print(f->worldMatrix());
    //Add matrix to viewer Info
    QString sName = QString("World after ")+slBoneName[bone];
    updateInfoMatrixAtRow((GLdouble*)f->worldMatrix(), ROW_CURRENT_MATRIX, sName);

    //DebugPrint Claw End-Effector frame World Matrix-----------------------------------------------
    //Set Rerefence frame, rotate and translte, read WorldMatrix back
    //Frame *f = new Frame();
    f->setReferenceFrame(viewer.myRobot.frame[E4_CLAW-1]); //set previous frame as reference
    f->setRotation(Quaternion(vecRotation[E4_CLAW], radRotation[E4_CLAW]));
    f->setTranslation(vecTranslate[E4_CLAW]);
    GLdouble wm[16];
    f->getWorldMatrix(wm);

    //Add matrix to viewer Info
    QString sNameEE = QString("World EndEffector ")+slBoneName[E4_CLAW];
    updateInfoMatrixAtRow(wm, ROW_EFFECTOR_MATRIX, sNameEE);

    ui->labelX->setText("X:"+s.sprintf("%+7.3f",wm[12]));
    ui->labelY->setText("Y:"+s.sprintf("%+7.3f",wm[13]));
    ui->labelZ->setText("Z:"+s.sprintf("%+7.3f",wm[14]));
    ui->groupBoxFK_XYZ->setTitle("End-Effector @"+slBoneName[E4_CLAW]);

    updateInfoRobotPos();
    //viewer.frameEndEffector.setPosition(Vec(wm[12], wm[13], wm[14]));   //update Particles position to End-Effector!
    //updateInfoRobotBones(); //update info on viewer background
    //viewer.updateBoneList();//update bone list
    if (ui->tabWidget->currentIndex() == E0_TAB_FK) { //process FABRIK only at TAB_FK, to prevent loop-back
        viewer.frameEndEffector.setPosition(Vec(wm[12], wm[13], wm[14]));   //update Particles position to End-Effector!
        updateInfoRobotBones(); //update info on viewer background
        viewer.updateBoneList();//update bone list
        }

    //Update targetXYZ
    if (ui->tabWidget->currentIndex() == E1_TAB_IK) {
        if (viewer.listBone_.size()) {
            Vec v = viewer.listBone_[viewer.listBone_.size()-1].vTail;
            ui->labelTargetX->setText("X:"+s.sprintf("%+7.3f",v.x));
            ui->labelTargetY->setText("Y:"+s.sprintf("%+7.3f",v.y));
            ui->labelTargetZ->setText("Z:"+s.sprintf("%+7.3f",v.z));
            }
        }
}

/*
 *  no selection when slider released
 */
void MainWindow::slotFKanglesSliderReleased()
{
    viewer.myRobot.idSelected = E5_NONE;
    viewer.update();
}

/*
 * A simple random walker
 *
 *  MG90 Speed:	4.8V: 0.11 sec/60°  ~ 1.8ms per deg => 150ms/1.8~83
 *
 *
 */
void MainWindow::slotFKexercise()
{
//qDebug() << Q_FUNC_INFO;

   int bone = qrand()%5;
   //int k   = (qrand()%(degBoneAngleMax[bone]-degBoneAngleMin[bone])) + (degBoneAngleMin[bone]);
   int k   = (qrand()%(83-degBoneAngleMin[bone])) + (degBoneAngleMin[bone]);

   //Locate item in layout
   QLayoutItem *lItem = ui->verticalLayoutFKangles->itemAt((bone*2)+1);
   QWidget *w = lItem->widget();
   QSlider *slider= dynamic_cast<QSlider*>(w);
   if (slider) {
        slider->setValue(k);
        slider->sliderMoved(k);
        }
}

void MainWindow::slotIKtest()
{
static int iListPoly=0;
static int iPoly=0;

QPolygonF p = viewer.polyILoveYou[iListPoly];

    Vec v = Vec(p[iPoly].rx()*0.1f, -0.5f-p[iPoly].ry()*0.12f, 3.0f);

    iPoly++;
    if (iPoly >= viewer.polyILoveYou[iListPoly].size()) {
        iPoly=0;
        iListPoly++;
        if (iListPoly >= viewer.polyILoveYou.size()) {
            iPoly=0;
            iListPoly=0;
            }
        }

    viewer.frameEndEffector.setPosition(v.x, v.y, v.z);
    viewer.FABRIKtarget(v.x, v.y, v.z);

    QString sFABRIKok = (viewer.bFABRIKok?"OK":"NG");
    QString s;
    s.sprintf("%03d:%03d: (%+6.3f,%+6.3f,%+6.3f)[%s]", iListPoly, iPoly, v.x, v.y, v.z, sFABRIKok.toStdString().c_str());
    ui->plainTextEdit->appendPlainText(s.toStdString().c_str());

    ui->labelTargetX->setText("X:"+s.sprintf("%+7.3f",v.x));
    ui->labelTargetY->setText("Y:"+s.sprintf("%+7.3f",v.y));
    ui->labelTargetZ->setText("Z:"+s.sprintf("%+7.3f",v.z));
}

void MainWindow::on_pushButtonFKexercise_clicked()
{
    if (timerFKexercise.isActive()) {
        ui->pushButtonFKexercise->setText("Exercise");
        timerFKexercise.stop();
        disconnect(&timerFKexercise);
        }
    else {
        ui->pushButtonFKexercise->setText("Stop");
        timerFKexercise.start(150);
        connect(&timerFKexercise, &QTimer::timeout, this, [=](){slotFKexercise();} );
        }
}

void MainWindow::on_checkBoxShowInfo_stateChanged(int arg1)
{
    viewer.bShowInfo = arg1;
}

/*
 *  FK:0 IK:1
 */
void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch (index) {
        case E0_TAB_FK: //FK
            viewer.bShowFire=false;
            ui->checkBoxShowFABRIKvec->setEnabled(false);
            ui->checkBoxShowInfo->setEnabled(true);
            ui->pushButtonIKtest->setEnabled(false);
            break;
        case E1_TAB_IK: //IK
            viewer.bShowFire=true;
            ui->checkBoxShowFABRIKvec->setEnabled(true);
            ui->checkBoxShowInfo->setEnabled(true);
            ui->pushButtonIKtest->setEnabled(true);
            updateInfoRobotBones(); //update info on viewer background
            break;
        }
}

void MainWindow::on_checkBoxShowFABRIKvec_stateChanged(int arg1)
{
    viewer.bShowFABRIKvec = arg1;
}

void MainWindow::on_pushButtonIKtest_clicked()
{
    viewer.bShowILoveYou = !viewer.bShowILoveYou;
    if (timerIKtest.isActive()) {
        ui->pushButtonIKtest->setText("IK Test");
        timerIKtest.stop();
        disconnect(&timerIKtest);
        }
    else {
        ui->pushButtonIKtest->setText("Stop");
        timerIKtest.start(75);
        connect(&timerIKtest, &QTimer::timeout, this, [=](){slotIKtest();} );
        }
}
