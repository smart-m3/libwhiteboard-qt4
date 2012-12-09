
/* -*- C++ -*-
 *
 */

#ifndef Q_TESTAPP_H

#include <QtGui/QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "q_sib_info.h"
#include "q_whiteboard_node.h"

class QueryBase;
class TemplateQuery;
class WqlValuesQuery;
class WqlRelatedQuery;
class WqlIsTypeQuery;
class WqlIsSubTypeQuery;
class WqlNodeTypesQuery;
class QTreeWidget;
class QTreeWidgetItem;
class QLCDNumber;
class TemplateSubscription;
class WqlRelatedSubscription;
class WqlValuesSubscription;


namespace Ui
{
    class QTestApp;

}

class QTestApp : public QMainWindow
{
    Q_OBJECT
    
public:
  QTestApp(QWidget *parent = 0);
  ~QTestApp();
	     
public slots:
  void joinComplete( int success );
  void sibList( const QList<QSibInfo *> &siblist );
  void sibInserted( const QSibInfo *sib);
  void sibRemoved( QString uri);

  void queryComplete( int success );

  void querycb( int success );
  void wqlvaluesquerycb( int success );
  void wqlrelatedquerycb( int success );
  void wqlistypequerycb( int success );
  void wqlissubtypequerycb( int success );
  void wqlnodetypesquerycb( int success );

  void subscriptionFinished(int success);
  void subscriptionIndication();

  void wqlrelatedindication();
  void wqlvaluesindication();
  void joinSelected();
  void leaveSelected();

  void queryTemplate();
  void queryWqlValues();
  void queryWqlRelated();
  void queryWqlIsType();
  void queryWqlIsSubType();
  void queryWqlNodeTypes();

  void subscribeTemplate();
  void subscribeWqlValues();
  void subscribeWqlRelated();
  void unsubscribe();

  void copyTriple(QTreeWidgetItem *item, int column);
signals:
    void getSibs();
    
private:

    bool addNode(QString sib);
    void leave();
  

    Ui::QTestApp *ui;


  QHash<QString, QWhiteBoardNode *> nodes;

  QLineEdit *expr_txt;
  QLineEdit *node1_txt;
  QLineEdit *node2_txt;
  QLabel *expr_label;
  QLabel *node1_label;
  QLabel *node2_label;

  QComboBox *node1_type;
  QComboBox *node2_type;

  QPushButton *addtograph_button;
  QList<Triple *> graph_list;
  QLabel *graph_label;
  QLCDNumber *graph_size;

  QComboBox * qtype;
  QLabel *qlabel;
  QTreeWidget *subsView;

  QHash<QString, TemplateQuery *> queries;
  QHash<QString, WqlValuesQuery *> wqlvalues_queries;
  QHash<QString, WqlRelatedQuery *> wqlrelated_queries;
  QHash<QString, WqlIsTypeQuery *> wqlistype_queries;
  QHash<QString, WqlIsSubTypeQuery *> wqlissubtype_queries;
  QHash<QString, WqlNodeTypesQuery *> wqlnodetypes_queries;

  QHash<QString, TemplateSubscription *> subscriptions;
  QHash<QString, WqlValuesSubscription *> wqlvalues_subscriptions;
  QHash<QString, WqlRelatedSubscription *> wqlrelated_subscriptions;

  QString activeSubscription;
  int subCounter;

private slots:

    void on_addToGraphButton_clicked();
    void on_sibAnyButton_clicked();
    void on_sib_list_doubleClicked(QModelIndex index);
    void on_subscribeButton_clicked();
    void on_queryButton_clicked();
    void on_updateButton_clicked();
    void on_removeButton_clicked();
    void on_insertButton_clicked();
};



#endif

