#include "mainwindow.h"
#include "ui_mainwindow.h"
using namespace qglviewer;

/*
 *   get DH matrix from DH list, at link=idx
 *
 *      input: *listDH, link idx
 *      output: *mat (4x4)
 */
void MainWindow::getDHmat(QList<DH*> *l, int idx, GLdouble *mat)
{
GLdouble m[4][4];

    double t = (*l)[idx]->thetaX;
    double a = (*l)[idx]->alphaZ;
    double r = (*l)[idx]->rOffsetX;
    double d = (*l)[idx]->dHeightZ;
qDebug() << t<<a;
    //convert degrees to radian
    t = qDegreesToRadians(t);
    a = qDegreesToRadians(a);
qDebug() << t<<a;
    m[0][0]=  qCos(t);
    m[1][0]= -qSin(t)*qCos(a);
    m[2][0]=  qSin(t)*sin(a);
    m[3][0]=  r*qCos(t);

    m[0][1]=  qSin(t);
    m[1][1]=  qCos(t)*qCos(a);
    m[2][1]= -qCos(t)*qSin(a);
    m[3][1]=  r*qSin(t);

    m[0][2]=  0;
    m[1][2]=  qSin(a);
    m[2][2]=  qCos(a);
    m[3][2]=  d;

    m[0][3]=  0;
    m[1][3]=  0;
    m[2][3]=  0;
    m[3][3]=  1;

    memcpy(mat, m, sizeof(m)); //copy all m[][] back to mat
}

/*
 *  Init DH table lT3DH;
 *
 *      Use list lT3DH to store the DH tables for each link
 *      Create tableWidgetsDH and tableWidgetDHmatrix
 *
 */
void MainWindow::InitDH()
{
    //Creat links of DH table in lT3DH
    lT3DH.clear();
    for (int i=0; i<DH_LINKS; i++) {
        DH *dh = new DH();
        dh->alphaZ  = _a[i];
        dh->dHeightZ= _d[i];
        dh->rOffsetX= _r[i];
        dh->thetaX  = _t[i];
        lT3DH.append(dh);
        }

    QString s;
    qDebug() <<   "| dHeightZ|  alphaZ°| rOffsetX|  thetaX°|";
    for (int i=0; i<DH_LINKS; i++) {
        s.sprintf("| %+7.3f |%+8.3f |%+7.3f  |%+8.3f |", lT3DH[i]->dHeightZ, lT3DH[i]->alphaZ, lT3DH[i]->rOffsetX, lT3DH[i]->thetaX);
        qDebug().noquote()<<s;
        }

    initTableWidgetDH();
    updateTableWidgetDH();
    initTableWidgetDHmatrix();
    //updateTableWidgetDHmatrixByLink(0); //initial
    ui->tableWidgetDH->selectRow(0); //the row selection should trig the matrix update
}

/*
 * initTableWidgetDH()
 *
 *      Create tablewidgetDH and set row selection signal/slot
 *
 */
void MainWindow::initTableWidgetDH()
{
    //Init DH TableWidget
    QTableWidget *tw = ui->tableWidgetDH;
    tw->clear();
    tw->setRowCount(DH_LINKS);
    tw->setColumnCount(4);

    QStringList slTableHeader; slTableHeader <<"dHeightZ"<<"alphaZ°"<<"rOffsetX" <<"thetaX°";
    tw->setHorizontalHeaderLabels(slTableHeader);
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tw->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tw->verticalHeader()->setDefaultSectionSize(16);
    tw->verticalHeader()->setVisible(false);
    tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw->setSelectionBehavior(QAbstractItemView::SelectRows);
    tw->setSelectionMode(QAbstractItemView::SingleSelection);
    tw->setShowGrid(true);
    tw->setStyleSheet("QTableView {selection-background-color:grey;}");

    int hHeight=tw->horizontalHeader()->height();
    int vHeight=tw->verticalHeader()->sectionSize(0) * DH_LINKS;
    int vTotal = hHeight+vHeight + 2;
    tw->setMinimumHeight(vTotal);
    tw->setMaximumHeight(vTotal);

    connect(tw->selectionModel(), &QItemSelectionModel::selectionChanged,
            [=](){slotTableWidgetDHselectionChanged();});
}
/*
 * updateTableWidgetDH()
 *
 *      update tablewidgetDH from lT3DH.
 *
 */
void MainWindow::updateTableWidgetDH()
{
    QTableWidget *tw = ui->tableWidgetDH;
    //insert data
    QString s;
    for (int r=0; r<DH_LINKS; r++) {
        s.sprintf("%+7.3f", lT3DH[r]->dHeightZ);
        {QTableWidgetItem *item = new QTableWidgetItem(s);
         item->setTextAlignment(Qt::AlignCenter);
         tw->setItem(r, 0, item);}

        s.sprintf("%+7.3f", lT3DH[r]->alphaZ);
        {QTableWidgetItem *item = new QTableWidgetItem(s);
         item->setTextAlignment(Qt::AlignCenter);
         tw->setItem(r, 1, item);}

        s.sprintf("%+7.3f", lT3DH[r]->rOffsetX);
        {QTableWidgetItem *item = new QTableWidgetItem(s);
         item->setTextAlignment(Qt::AlignCenter);
         tw->setItem(r, 2, item);}

        s.sprintf("%+7.3f", lT3DH[r]->thetaX);
        {QTableWidgetItem *item = new QTableWidgetItem(s);
         item->setTextAlignment(Qt::AlignCenter);
         tw->setItem(r, 3, item);}
        }
}

/*
 * initTableWidgetDHmatrix()
 *
 *      Create tablewidwetDHmatrix
 *
 */
void MainWindow::initTableWidgetDHmatrix()
{
    QTableWidget *tw = ui->tableWidgetDHmatrix;
    tw->clear();
    tw->setRowCount(4);
    tw->setColumnCount(4);
    tw->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); //set horizontal header stretchable
    tw->horizontalHeader()->setVisible(false);
    tw->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed); //set vertical rows fixed
    tw->verticalHeader()->setDefaultSectionSize(12);
    tw->verticalHeader()->setVisible(false);
    tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw->setSelectionBehavior(QAbstractItemView::SelectRows);
    //tw->setSelectionMode(QAbstractItemView::SingleSelection);
    tw->setSelectionMode(QAbstractItemView::NoSelection);
    tw->setShowGrid(true);
    //tw->setStyleSheet("QTableView {selection-background-color:lightgrey;}");

    //int hHeight=tw->horizontalHeader()->height(); //no more hHeight as we hide it
    int vHeight=tw->verticalHeader()->sectionSize(0) * 4;
    //int vTotal = hHeight+vHeight + 2;
    int vTotal =  vHeight + 2;
    tw->setMinimumHeight(vTotal);
    tw->setMaximumHeight(vTotal);
}

/*
 * updateTableWidgetDHmatrix(row)
 *
 *      udpate tabelwidetDHmatrix data by DH row
 *
 */
void MainWindow::updateTableWidgetDHmatrixByLink(int rowDH)
{
    GLdouble m[16];
    getDHmat(&lT3DH, rowDH, m);
    qDebug().noquote() << "Mat" << rowDH << endl << sM4print(m);
    ui->labelDHmatrix->setText(QString("Matrix(%1): %2").arg(_lLinkname[rowDH]).arg(QString::number(rowDH)) );

    QTableWidget *tw = ui->tableWidgetDHmatrix;
    //---------------------------------------------
    //Update data
    QString s;
    for (int c=0; c<4; c++)
        for (int r=0; r<4; r++) {
            s.sprintf("%+7.3f", *(m+(c*4+r)));
            QTableWidgetItem *item = new QTableWidgetItem(s);
            item->setTextAlignment(Qt::AlignCenter);
            tw->setItem(c, r, item);
            }
}

void MainWindow::slotTableWidgetDHselectionChanged()
{
qDebug() << Q_FUNC_INFO;
    //QModelIndexList selection = ui->tableWidgetDH->selectionModel()->selectedRows();//Here you are getting the indexes of the selected rows
    int row=ui->tableWidgetDH->selectionModel()->currentIndex().row();
    //qDebug() << "Row" <<  row;

    updateTableWidgetDHmatrixByLink(row);
}
