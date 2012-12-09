#include <QApplication>
#include <QGridLayout>
#include <QDebug>
#include <QMessageBox>
#include <QTreeWidget>
#include <QLCDNumber>

#include "q_testapp.h"
#include "q_sib_discovery.h"
#include "q_sib_info.h"
#include "q_whiteboard_node.h"
#include "triple.h"
#include "querybase.h"
#include "templatequery.h"
#include "wqlvaluesquery.h"
#include "wqlrelatedquery.h"
#include "wqlistypequery.h"
#include "wqlissubtypequery.h"
#include "wqlnodetypesquery.h"
#include "templatesubscription.h"
#include "wqlrelatedsubscription.h"
#include "wqlvaluessubscription.h"
#include "ui_qtestapp.h"

QTestApp::QTestApp(QWidget *parent)
  :QMainWindow(parent),
  ui(new Ui::QTestApp),
   subsView(NULL),
   subCounter(0)
{
    ui->setupUi(this);

    QMenu *fileMenu = ui->menuBar->addMenu(tr("File"));

    QAction *closeAct = new QAction(tr("Close"), this);

    closeAct->setStatusTip(tr("Exit"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction(closeAct);
    ui->statusbar->showMessage(tr("Not joined"));

  QStringList types3;
  types3 << tr("Template");
  types3 << tr("WQL-Values");
  types3 << tr("WQL-Related");
  types3 << tr("WQL-NodeTypes");
  types3 << tr("WQL-IsType");
  types3 << tr("WQL-IsSubType");
  types3 << tr("SPARQL");

  ui->qtypeComboBox->addItems(types3);

#if 0
  nlayout->addWidget ( addtograph_button = new QPushButton("Add"), 0,0);
  nlayout->addWidget ( graph_label = new QLabel("Graphsize:"), 1,0);
  nlayout->addWidget ( graph_size = new QLCDNumber(), 2,0);
  graph_size->display( 0 );
#endif

  QStringList types1;
  types1 << tr("uri");
  types1 << tr("bnode");

  QStringList types2;
  types2 << tr("uri");
  types2 << tr("lit");
  types2 << tr("bnode");

  ui->subTypeComboBox->addItems(types1);

  ui->objTypeComboBox->addItems(types2);
#if 0
  tlayout->addWidget( expr_label = new QLabel("Expr"), 4,0);
  tlayout->addWidget( node1_label = new QLabel("Node1"), 5,0);
  tlayout->addWidget( node2_label = new QLabel("Node2"), 6,0);
  
  tlayout->addWidget( expr_txt = new QLineEdit(), 4,1);
  tlayout->addWidget( node1_txt = new QLineEdit(), 5,1);
  tlayout->addWidget( node2_txt = new QLineEdit(), 6,1);

  QStringList types4;
  types4 << tr("uri");
  types4 << tr("lit");

  node1_type = new QComboBox();
  node1_type->addItems(types4);
  node2_type = new QComboBox();
  node2_type->addItems(types4);

  tlayout->addWidget( node1_type, 5,2);
  tlayout->addWidget( node2_type, 6,2);

  layout->addWidget( sib_list = new QListWidget, 0, 0, 1, 1);

  layout->addLayout( blayout, 0,1);
  layout->addLayout( tlayout, 1,0);
  layout->addLayout( nlayout, 1,1);

  connect( join_button, SIGNAL(clicked()), SLOT( joinSelected() ) );
  connect( leave_button, SIGNAL(clicked()), SLOT( leaveSelected() ) );
  connect( close_button, SIGNAL(clicked()), SLOT( quit()));
  connect( insert_button, SIGNAL(clicked()), SLOT( insert()));
  connect( remove_button, SIGNAL(clicked()), SLOT( remove()));
  connect( update_button, SIGNAL(clicked()), SLOT( updatetest()));
  connect( query_button, SIGNAL(clicked()), SLOT( query()));
  connect( addtograph_button, SIGNAL(clicked()), SLOT( addtograph()));
  connect( subscribe_button, SIGNAL(clicked()), SLOT( subscribe()));
  connect( sibany_button, SIGNAL( clicked() ), SLOT( fillsibany() ));
#endif
}

QTestApp::~QTestApp()
{
  QHash<QString, QWhiteBoardNode *>::iterator it= nodes.begin();
  while(it != nodes.end())
    {
      QWhiteBoardNode *node = nodes.take(it.key());
      node->deleteLater();
      it++;
    }
}

void QTestApp::leaveSelected()
{
  QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
  QStringList txt = item->text().split("--");
  QString sib_uri = txt[0];
  if( nodes.contains(sib_uri) )
    {
      qDebug() << "leaving, sib: " << sib_uri;

      if(nodes[sib_uri]->leave()<0)
	{
	  qDebug() << "ERROR: Leave failed" << sib_uri;
	}
      QWhiteBoardNode *node = nodes.take(sib_uri);
      node->deleteLater();
      ui->statusbar->showMessage(tr("Not joined"));
    }

}

void QTestApp::joinSelected()
{
  QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
  QStringList txt = item->text().split("--");
  QString sib_uri = txt[0];
  if( addNode(sib_uri) )
    {
      QString msg = "Joining, sib: ";
      msg.append(sib_uri);
      msg.append(" ...");

      ui->statusbar->showMessage(msg);
      qDebug() << "joining, sib: " << sib_uri;
      connect( nodes[sib_uri], SIGNAL(joinComplete(int)), this, SLOT(joinComplete(int)));
      if(nodes[sib_uri]->join(sib_uri)<0)
	{
	  QMessageBox::critical(this, "Error","Join Failed");
          ui->statusbar->showMessage("Not joined");
	}
    }

}

bool QTestApp::addNode( QString sib )
{
  if( !nodes.contains(sib) )
    {
      nodes[sib]= new QWhiteBoardNode();
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

void QTestApp::on_insertButton_clicked()
{
  if(graph_list.count() == 0)
    {
      on_addToGraphButton_clicked();
    }
  //  QHash<QString, QString> ns;
  //ns["sib"] = "http://www.nokia.com/m3/definitions#";
      
      
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
	  
      if( nodes.contains(sib_uri) &&
	  nodes[sib_uri]->insert( graph_list /*, &ns*/) >= 0 )
	{
	  QMessageBox::information(this, "Insert","OK");
	}
      else
	{
	  QMessageBox::critical(this, "Error","Insert failed");
	}
    }
  else
    {
      QMessageBox::warning(this, "Error", "Insert, No SmartSpaces found");
    }

  while(graph_list.count())
    {
      Triple *triple = graph_list.takeFirst();
      delete triple;
    }
  ui->graph_size->display(0);
}

void QTestApp::on_addToGraphButton_clicked()
{
  if( ui->subTxt->text().isEmpty() ||
      ui->predTxt->text().isEmpty() ||
      ( ( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)  && ui->objTxt->text().isEmpty() ) )
    {
      QMessageBox::warning(this, "Error","All triple elements not specified");
      return;
    }
  TripleElement::ElementType sub_type;
  TripleElement::ElementType obj_type;
  if( ui->subTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      sub_type = TripleElement::ElementTypeURI;
    }
  else if( ui->subTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      sub_type = TripleElement::ElementTypebNode;
    }
  else
    {
      QMessageBox::warning(this, "Error","Unknown subject Type");
      return;
    }
  
  if( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      obj_type = TripleElement::ElementTypeURI;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("lit")) == 0)
    {
      obj_type = TripleElement::ElementTypeLiteral;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      obj_type = TripleElement::ElementTypebNode;
    }
  else
    {
      QMessageBox::warning(this, "Error","Unknown object Type");
      return;
    }
  
  Triple *triple = new Triple( TripleElement(ui->subTxt->text(), sub_type),
                       TripleElement(ui->predTxt->text()),
                       TripleElement(ui->objTxt->text() , obj_type) );
  
  graph_list.append(triple);
  ui->graph_size->display( graph_list.count());
}

void QTestApp::on_sibAnyButton_clicked()
{
  ui->subTxt->setText( tr("sib:any") );
  ui->subTypeComboBox->setCurrentIndex(0);

  ui->predTxt->setText( tr("sib:any") );
  
  ui->objTxt->setText( tr("sib:any") );
  ui->objTypeComboBox->setCurrentIndex(0);

}

void QTestApp::on_removeButton_clicked()
{
  QString triples;


  Triple *triple = NULL;

  if(ui->subTxt->text().isEmpty() || ui->predTxt->text().isEmpty() ||  ( ( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)  && ui->objTxt->text().isEmpty() ) )
    {
      QMessageBox::information(this, "Error","All triple elements not specified");
      return;
    }

  TripleElement::ElementType sub_type;
  TripleElement::ElementType obj_type;
  if( ui->subTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      sub_type = TripleElement::ElementTypeURI;
    }
  else if( ui->subTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      sub_type = TripleElement::ElementTypebNode;
    }
 else
    {
      QMessageBox::warning(this, "Error","Unknown subject Type");
      return;
    }

  if( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      obj_type = TripleElement::ElementTypeURI;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("lit")) == 0)
    {
      obj_type = TripleElement::ElementTypeLiteral;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      obj_type = TripleElement::ElementTypebNode;
    }
  else
    {
      QMessageBox::warning(this, "Error","Unknown object Type");
      return;
    }

  triple = new Triple( TripleElement(ui->subTxt->text(), sub_type),
                       TripleElement(ui->predTxt->text()),
                       TripleElement(ui->objTxt->text() , obj_type ));

  QList<Triple *> list;
  list.append(triple);

  //  QHash<QString, QString> ns;
  //ns["sib"] = "http://www.nokia.com/m3/definitions#";
  

  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      
      if( nodes.contains(sib_uri) && 
	  nodes[sib_uri]->remove( list /*, &ns*/) >= 0 )
	{
	  QMessageBox::information(this, "Remove","OK");
	}
      else
	{
	  QMessageBox::information(this, "Error","Remove failed");
	}
    }
  else
    {
      QMessageBox::information(this, "Error", "Remove, No SmartSpaces found");
    }
  while(list.count())
   {
      triple = list.takeFirst();
      delete triple;
    }
}

void QTestApp::on_queryButton_clicked()
{
  
  if( ui->qtypeComboBox->currentText().compare("Template") == 0 )
    {
      queryTemplate();
    }
  else if( ui->qtypeComboBox->currentText().compare("WQL-Values") == 0 )
    {
      queryWqlValues();
    }
  else if( ui->qtypeComboBox->currentText().compare("WQL-Related") == 0 )
    {
      queryWqlRelated();
    }
 else if( ui->qtypeComboBox->currentText().compare("WQL-NodeTypes") == 0 )
    {
      queryWqlNodeTypes();
    }
 else if( ui->qtypeComboBox->currentText().compare("WQL-IsType") == 0 )
    {
      queryWqlIsType();
    }
 else if( ui->qtypeComboBox->currentText().compare("WQL-IsSubType") == 0 )
    {
      queryWqlIsSubType();
    }
  else
    {
      QMessageBox::information(this, "Error", "Query type unknown or not implemented");
    }
}

void QTestApp::on_subscribeButton_clicked()
{
  if(activeSubscription.isEmpty())
    {
      if( ui->qtypeComboBox->currentText().compare("Template") == 0 )
	{
	  subscribeTemplate();
	}
      else if( ui->qtypeComboBox->currentText().compare("WQL-Values") == 0 )
	{
	  subscribeWqlValues();
	}
      else if( ui->qtypeComboBox->currentText().compare("WQL-Related") == 0 )
	{
	  subscribeWqlRelated();
	}
      else if( ui->qtypeComboBox->currentText().compare("WQL-NodeTypes") == 0 )
	{
	  queryWqlNodeTypes();
	}
      else if( ui->qtypeComboBox->currentText().compare("WQL-IsType") == 0 )
	{
	  queryWqlIsType();
	}
      else if( ui->qtypeComboBox->currentText().compare("WQL-IsSubType") == 0 )
	{
	  queryWqlIsSubType();
	}
      else
	{
	  QMessageBox::information(this, "Error", "Query type unknown or not implemented");
	}
    }
  else
    {
      QMessageBox::information(this, "Error", "Subscription already active");
    }
}

void QTestApp::queryTemplate()
{
  Triple *triple = NULL;
  if(ui->subTxt->text().isEmpty() || ui->predTxt->text().isEmpty() ||  ( ( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)  && ui->objTxt->text().isEmpty() ) )
    {
      QMessageBox::information(this, "Error","All triple elements not specified");
      return;
    }

  TripleElement::ElementType sub_type;
  TripleElement::ElementType obj_type;
  if( ui->subTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      sub_type = TripleElement::ElementTypeURI;
    }
  else if( ui->subTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      sub_type = TripleElement::ElementTypebNode;
    }
 else
    {
      QMessageBox::warning(this, "Error","Unknown subject Type");
      return;
    }

  if( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      obj_type = TripleElement::ElementTypeURI;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("lit")) == 0)
    {
      obj_type = TripleElement::ElementTypeLiteral;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      obj_type = TripleElement::ElementTypebNode;
    }
  else
    {
      QMessageBox::warning(this, "Error","Unknown object Type");
      return;
    }

  triple = new Triple( TripleElement(ui->subTxt->text(), sub_type),
                       TripleElement(ui->predTxt->text()),
                       TripleElement(ui->objTxt->text() , obj_type ));

  QList<Triple *> list;
  list.append(triple);

 if (ui->sib_list->count() > 0) {
     QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
     QStringList txt = item->text().split("--");
     QString sib_uri = txt[0];
     if( nodes.contains(sib_uri) ) {
         TemplateQuery *q = new TemplateQuery( nodes[sib_uri] );
         q->setObjectName("testQuery");
         if( !queries.contains( q->objectName() ) )
             queries[ q->objectName() ] = q;
         else
         {
             QMessageBox::warning(this, "Error","Query already exists");
             return;
         }

         connect( q, SIGNAL( finished(int) ), this, SLOT( querycb(int) ) );
         q->query(list);
     }
 }
 while(list.count()) {
     triple=list.takeFirst();
     delete triple;
 }

}


void QTestApp::queryWqlValues()
{
  if(expr_txt->text().isEmpty() || 
     ((node1_type->currentText().compare("uri")==0) && 
      node1_txt->text().isEmpty() ))
    {
      QMessageBox::information(this, "Error","Expression and node1 need to be defined for WQL-Values query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlValuesQuery *q = new WqlValuesQuery( nodes[sib_uri] );
	  q->setObjectName("WQLValuesTestQuery");
	  if( !wqlvalues_queries.contains( q->objectName() ) )
	  wqlvalues_queries[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Query already exists");
	      return;
	    }

	  connect( q, SIGNAL( finished(int) ), this, SLOT( wqlvaluesquerycb(int) ) );
	  TripleElement::ElementType elType;
	  if(node1_type->currentText().compare("uri") == 0 )
	    elType = TripleElement::ElementTypeURI;
	  else
	    elType = TripleElement::ElementTypeLiteral;
	  TripleElement el(node1_txt->text(), elType);

	  q->query( el, expr_txt->text() );
	}
    }
}

void QTestApp::queryWqlRelated()
{
  if(expr_txt->text().isEmpty() || node1_txt->text().isEmpty()|| node2_txt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","Expression and node1&node2 need to be defined for WQL-Values query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlRelatedQuery *q = new WqlRelatedQuery( nodes[sib_uri] );
	  q->setObjectName("WQLRelatedTestQuery");
	  if( !wqlrelated_queries.contains( q->objectName() ) )
	  wqlrelated_queries[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Query already exists");
	      return;
	    }

	  connect( q, SIGNAL( finished(int) ), this, SLOT( wqlrelatedquerycb(int) ) );
	  TripleElement::ElementType elType1;
	  TripleElement::ElementType elType2;
	  if(node1_type->currentText().compare("uri") == 0 )
	    elType1 = TripleElement::ElementTypeURI;
	  else
	    elType1 = TripleElement::ElementTypeLiteral;


	  if(node2_type->currentText().compare("uri") == 0 )
	    elType2 = TripleElement::ElementTypeURI;
	  else
	    elType2 = TripleElement::ElementTypeLiteral;
	  
	  TripleElement el1(node1_txt->text(), elType1);
	  TripleElement el2(node2_txt->text(), elType2);

	  q->query( el1,el2, expr_txt->text() );
	}
    }
}



void QTestApp::queryWqlIsType()
{
  if(node1_txt->text().isEmpty()|| node2_txt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","Node1 & node2 need to be defined for WQL-IsType query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlIsTypeQuery *q = new WqlIsTypeQuery( nodes[sib_uri] );
	  q->setObjectName("WQLIsTypeTestQuery");
	  if( !wqlistype_queries.contains( q->objectName() ) )
	  wqlistype_queries[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Query already exists");
	      return;
	    }

	  connect( q, SIGNAL( finished(int) ), this, SLOT( wqlistypequerycb(int) ) );
	  
	  TripleElement el1(node1_txt->text(), TripleElement::ElementTypeURI);
	  TripleElement el2(node2_txt->text(), TripleElement::ElementTypeURI);

	  q->queryIsType( el1,el2 );
	}
    }
}

void QTestApp::queryWqlIsSubType()
{
  if(node1_txt->text().isEmpty()|| node2_txt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","Node1 & node2 need to be defined for WQL-IsSubType query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlIsSubTypeQuery *q = new WqlIsSubTypeQuery( nodes[sib_uri] );
	  q->setObjectName("WQLIsSubTypeTestQuery");
	  if( !wqlissubtype_queries.contains( q->objectName() ) )
	  wqlissubtype_queries[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Query already exists");
	      return;
	    }

	  connect( q, SIGNAL( finished(int) ), this, SLOT( wqlissubtypequerycb(int) ) );
	  
	  TripleElement el1(node1_txt->text(), TripleElement::ElementTypeURI);
	  TripleElement el2(node2_txt->text(), TripleElement::ElementTypeURI);

	  q->queryIsSubType( el1,el2 );
	}
    }
}



void QTestApp::queryWqlNodeTypes()
{
  if(node1_txt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","node1 needs to be defined for WQL-NodeTypes query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlNodeTypesQuery *q = new WqlNodeTypesQuery( nodes[sib_uri] );
	  q->setObjectName("WQLNodeTypesTestQuery");
	  if( !wqlnodetypes_queries.contains( q->objectName() ) )
	  wqlnodetypes_queries[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Query already exists");
	      return;
	    }

	  connect( q, SIGNAL( finished(int) ), this, SLOT( wqlnodetypesquerycb(int) ) );
	  TripleElement::ElementType elType;
	  if(node1_type->currentText().compare("uri") == 0 )
	    elType = TripleElement::ElementTypeURI;
	  else
	    elType = TripleElement::ElementTypeLiteral;
	  TripleElement el(node1_txt->text(), elType);

	  q->query( el );
	}
    }
}


void QTestApp::subscribeTemplate()
{
  Triple *triple = NULL;
  if(ui->subTxt->text().isEmpty() || ui->predTxt->text().isEmpty() || ui->objTxt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","All triple elements not specified");
      return;
    }

  TripleElement::ElementType sub_type;
  TripleElement::ElementType obj_type;
  if( ui->subTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      sub_type = TripleElement::ElementTypeURI;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      sub_type = TripleElement::ElementTypebNode;
    }
 else
    {
      QMessageBox::warning(this, "Error","Unknown subject Type");
      return;
    }

  if( ui->objTypeComboBox->currentText().compare(tr("uri")) == 0)
    {
      obj_type = TripleElement::ElementTypeURI;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("lit")) == 0)
    {
      obj_type = TripleElement::ElementTypeLiteral;
    }
  else if( ui->objTypeComboBox->currentText().compare(tr("bnode")) == 0)
    {
      obj_type = TripleElement::ElementTypebNode;
    }
  else
    {
      QMessageBox::warning(this, "Error","Unknown object Type");
      return;
    }

  triple = new Triple( TripleElement(ui->subTxt->text(), sub_type),
                       TripleElement(ui->predTxt->text()),
                       TripleElement(ui->objTxt->text() , obj_type ));

  QList<Triple *> list;
  list.append(triple);

 if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  TemplateSubscription *s = new TemplateSubscription( nodes[sib_uri] );
	  s->setObjectName("testTemplateSubscription");
	  if( !subscriptions.contains( s->objectName() ) )
	    subscriptions[ s->objectName() ] = s;
	  else
	    {
	      QMessageBox::warning(this, "Error","Subscription already exists");
	      return;
	    }

	  connect( s, SIGNAL( finished(int) ), this, SLOT( subscriptionFinished(int) ) );
	  connect( s, SIGNAL( indication() ), this, SLOT( subscriptionIndication() ) );
	  s->subscribe(list);
	}
    }
 while(list.count())
   {
     triple = list.takeFirst();
     delete triple;
   }
}


void QTestApp::subscribeWqlValues()
{
  if(expr_txt->text().isEmpty() || node1_txt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","Expression and node1 need to be defined for WQL-Values query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlValuesSubscription *q = new WqlValuesSubscription( nodes[sib_uri] );
	  q->setObjectName("WQLValuesTestSubscription");
	  if( !wqlvalues_subscriptions.contains( q->objectName() ) )
	  wqlvalues_subscriptions[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Subscription already exists");
	      return;
	    }

	  connect( q, SIGNAL( indication() ), this, SLOT( wqlvaluesindication() ) );
	  connect( q, SIGNAL( finished( int ) ), this, SLOT( subscriptionFinished( int ) ) );
	  TripleElement::ElementType elType1;
	  if(node1_type->currentText().compare("uri") == 0 )
	    elType1 = TripleElement::ElementTypeURI;
	  else
	    elType1 = TripleElement::ElementTypeLiteral;

          TripleElement el1(node1_txt->text(), elType1);

	  q->subscribe( el1, expr_txt->text() );
	}
    }


}


void QTestApp::subscribeWqlRelated()
{
  if(expr_txt->text().isEmpty() || node1_txt->text().isEmpty()|| node2_txt->text().isEmpty() )
    {
      QMessageBox::information(this, "Error","Expression and node1&node2 need to be defined for WQL-Related query");
      return;
    }
  
  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      if( nodes.contains(sib_uri) )
	{
	  WqlRelatedSubscription *q = new WqlRelatedSubscription( nodes[sib_uri] );
	  q->setObjectName("WQLRelatedTestSubscription");
	  if( !wqlrelated_subscriptions.contains( q->objectName() ) )
	  wqlrelated_subscriptions[ q->objectName() ] = q;
	  else
	    {
	      QMessageBox::warning(this, "Error","Subscription already exists");
	      return;
	    }

	  connect( q, SIGNAL( indication() ), this, SLOT( wqlrelatedindication() ) );
	  connect( q, SIGNAL( finished( int ) ), this, SLOT( subscriptionFinished( int ) ) );
	  TripleElement::ElementType elType1;
	  TripleElement::ElementType elType2;
	  if(node1_type->currentText().compare("uri") == 0 )
	    elType1 = TripleElement::ElementTypeURI;
	  else
	    elType1 = TripleElement::ElementTypeLiteral;


	  if(node2_type->currentText().compare("uri") == 0 )
	    elType2 = TripleElement::ElementTypeURI;
	  else
	    elType2 = TripleElement::ElementTypeLiteral;
	  
	  TripleElement el1(node1_txt->text(), elType1);
	  TripleElement el2(node2_txt->text(), elType2);

	  q->subscribe( el1,el2, expr_txt->text() );
	}
    }


}

void QTestApp::on_updateButton_clicked()
{
  QList<Triple *> ilist;
  QList<Triple *> rlist;

  Triple *rt = new Triple( TripleElement("Timo"), 
			   TripleElement("livesIn"),
			   TripleElement("Amsterdam", TripleElement::ElementTypeLiteral) );
  
  Triple *it = new Triple( TripleElement("Timo"), 
			   TripleElement("livesIn"),
			   TripleElement("Helsinki", TripleElement::ElementTypeLiteral) );

  ilist.append(it);
  rlist.append(rt);

  //  QHash<QString, QString> ns;
  //ns["sib"] = "http://www.nokia.com/m3/definitions#";

  if(ui->sib_list->count() > 0)
    {
      QListWidgetItem *item = ui->sib_list->item(ui->sib_list->currentRow() );
      QStringList txt = item->text().split("--");
      QString sib_uri = txt[0];
      
      
      if( nodes.contains(sib_uri) && nodes[sib_uri]->update( ilist, rlist /*, &ns*/) >= 0 )
	{
	  qDebug() << "Update ok";
	}
      else
	{
	  qDebug() << "ERROR: Could not remove from sib:" << sib_uri;
	}
    }
  while(ilist.count())
    {
      it = ilist.takeFirst();
      delete it;
    }
  while(rlist.count())
    {
      rt = rlist.takeFirst();
      delete rt;
    }
}

void QTestApp::querycb(int success)
{
  if(queries.contains( sender()->objectName() ) )
    {
      TemplateQuery *q = NULL;
      q = queries[ sender()->objectName() ];
      if(success == 0)
	{
	  QList<Triple *> results = static_cast<TemplateQuery *>(q)->results();
	  QList<Triple *>::iterator it;

	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(950,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  QTreeWidget *triplew = new QTreeWidget(tripled);
	  triplew->setColumnCount(3);
	  triplew->setColumnWidth(0,300);
	  triplew->setColumnWidth(1,300);
	  triplew->setColumnWidth(2,300);
	  QStringList headers;
	  headers << tr("subject");
	  headers << tr("predicate");
	  headers << tr("object");
	  triplew->setHeaderLabels(headers);
	  qDebug() << "Got: " << results.count() << "triples";

	  QList<QTreeWidgetItem *> items;
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      QStringList strings;
	      strings << (*it)->subject().node();
	      strings << (*it)->predicate().node();

	      if(! (*it)->object().node().isEmpty() )
		strings << (*it)->object().node();
	      else
 		strings << "<empty>";

	      QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	      if((*it)->object().type() == TripleElement::ElementTypeLiteral)
		{
		  if( !(*it)->object().node().isEmpty() )
 		    {
		      QColor blue = QColor(0,0,255);
		      item->setForeground(2,QBrush( blue ));
		    }
		  else
		    {
		      QColor green = QColor(0,255,0);
 		      item->setForeground(2,QBrush( green ));
		    }
		}
	      
	      items.append(item);
	    }
	  triplew->insertTopLevelItems(0,items);

	  layout->addWidget( triplew, 0,0);
	  
	  QPushButton *ok_button = new QPushButton("Ok");

	  layout->addWidget( ok_button, 1,0);

	  
	  connect( ok_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  connect( triplew, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int)),
		   this, SLOT( copyTriple(QTreeWidgetItem *, int))); 
	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QMessageBox::warning(this, "Error","Query failed.");
	}
      queries.remove( sender()->objectName() );
      delete q;
    }

}

void QTestApp::copyTriple(QTreeWidgetItem *item, int /*column*/)
{
  ui->subTxt->setText(item->text(0));
  ui->predTxt->setText(item->text(1));
  ui->objTxt->setText(item->text(2));
}

void QTestApp::wqlvaluesquerycb(int success)
{
  if(wqlvalues_queries.contains( sender()->objectName() ) )
    {
      WqlValuesQuery *q = NULL;
      q = wqlvalues_queries[ sender()->objectName() ];
      if(success == 0)
	{
	  QList<TripleElement> results = static_cast<WqlValuesQuery *>(q)->results();
          QList<TripleElement>::iterator it;
      
	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  QTreeWidget *triplew = new QTreeWidget(tripled);
	  triplew->setColumnCount(1);
	  triplew->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("NodeList");
	  triplew->setHeaderLabels(headers);


	  QList<QTreeWidgetItem *> items;
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      QStringList strings;
	      if( !(*it).node().isEmpty() )
		strings << (*it).node();
	      else
		strings << "<empty>";

	      QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	      if( (*it).type() == TripleElement::ElementTypeLiteral)
		{
		  if(  !(*it).node().isEmpty() ) 
		    {
		      QColor blue = QColor(0,0,255);
		      item->setForeground(0, QBrush( blue) );
		    }
		  else
		    {
		      QColor green = QColor(0,255,0);
		      item->setForeground(0, QBrush( green) ); 
		    }
		}
	      items.append(item);
	    }
	  triplew->insertTopLevelItems(0,items);

	  layout->addWidget( triplew, 0,0);
	  
	  QPushButton *ok_button = new QPushButton("Ok");

	  layout->addWidget( ok_button, 1,0);
	  
	  connect( ok_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QMessageBox::warning(this, "Error","Query failed.");
	}
      wqlvalues_queries.remove( sender()->objectName() );
    }

}

void QTestApp::wqlnodetypesquerycb(int success)
{
  if(wqlnodetypes_queries.contains( sender()->objectName() ) )
    {
      WqlNodeTypesQuery *q = NULL;
      q = wqlnodetypes_queries[ sender()->objectName() ];
      if(success == 0)
	{
	  QList<TripleElement> results = q->results();
          QList<TripleElement>::iterator it;
      
	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  QTreeWidget *triplew = new QTreeWidget(tripled);
	  triplew->setColumnCount(1);
	  triplew->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("NodeTypes");
	  triplew->setHeaderLabels(headers);


	  QList<QTreeWidgetItem *> items;
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      QStringList strings;
	      strings << (*it).node();
	      QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	      if( (*it).type() == TripleElement::ElementTypeLiteral)
		{
		  QColor blue = QColor(0,0,255);
		  item->setForeground(0, QBrush( blue) );
		}
	      items.append(item);
	    }
	  triplew->insertTopLevelItems(0,items);

	  layout->addWidget( triplew, 0,0);
	  
	  QPushButton *ok_button = new QPushButton("Ok");

	  layout->addWidget( ok_button, 1,0);
	  
	  connect( ok_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QMessageBox::warning(this, "Error","Query failed.");
	}
      wqlnodetypes_queries.remove( sender()->objectName() );
    }
}

void QTestApp::wqlrelatedquerycb(int success)
{
  if(wqlrelated_queries.contains( sender()->objectName() ) )
    {
      WqlRelatedQuery *q = NULL;
      q = wqlrelated_queries[ sender()->objectName() ];
      if(success == 0)
	{
	  bool results = q->results();

	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  QTreeWidget *triplew = new QTreeWidget(tripled);
	  triplew->setColumnCount(1);
	  triplew->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("Related");
	  triplew->setHeaderLabels(headers);

	  QList<QTreeWidgetItem *> items;
	  QStringList strings;
	  if(results)
	    strings << "TRUE";
	  else
	    strings << "FALSE";

	  QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	  items.append(item);
	  triplew->insertTopLevelItems(0,items);

	  layout->addWidget( triplew, 0,0);
	  
	  QPushButton *ok_button = new QPushButton("Ok");

	  layout->addWidget( ok_button, 1,0);
	  
	  connect( ok_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QMessageBox::warning(this, "Error","Query failed.");
	}
      wqlrelated_queries.remove( sender()->objectName() );
    }
}

void QTestApp::wqlistypequerycb(int success)
{
  if(wqlistype_queries.contains( sender()->objectName() ) )
    {
      WqlIsTypeQuery *q = NULL;
      q = wqlistype_queries[ sender()->objectName() ];
      if(success == 0)
	{
	  bool results = q->results();

	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  QTreeWidget *triplew = new QTreeWidget(tripled);
	  triplew->setColumnCount(1);
	  triplew->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("IsType");
	  triplew->setHeaderLabels(headers);

	  QList<QTreeWidgetItem *> items;
	  QStringList strings;
	  if(results)
	    strings << "TRUE";
	  else
	    strings << "FALSE";

	  QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	  items.append(item);
	  triplew->insertTopLevelItems(0,items);

	  layout->addWidget( triplew, 0,0);
	  
	  QPushButton *ok_button = new QPushButton("Ok");

	  layout->addWidget( ok_button, 1,0);
	  
	  connect( ok_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QMessageBox::warning(this, "Error","Query failed.");
	}
      wqlistype_queries.remove( sender()->objectName() );
    }
}


void QTestApp::wqlissubtypequerycb(int success)
{
  if(wqlissubtype_queries.contains( sender()->objectName() ) )
    {
      WqlIsSubTypeQuery *q = NULL;
      q = wqlissubtype_queries[ sender()->objectName() ];
      if(success == 0)
	{
	  bool results = q->results();

	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  QTreeWidget *triplew = new QTreeWidget(tripled);
	  triplew->setColumnCount(1);
	  triplew->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("IsSubType");
	  triplew->setHeaderLabels(headers);


	  QList<QTreeWidgetItem *> items;
	  QStringList strings;
	  if(results)
	    strings << "TRUE";
	  else
	    strings << "FALSE";

	  QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	  items.append(item);
	  triplew->insertTopLevelItems(0,items);

	  layout->addWidget( triplew, 0,0);
	  
	  QPushButton *ok_button = new QPushButton("Ok");

	  layout->addWidget( ok_button, 1,0);
	  
	  connect( ok_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QMessageBox::warning(this, "Error","Query failed.");
	}
      wqlissubtype_queries.remove( sender()->objectName() );
    }
}


void QTestApp::subscriptionIndication()
{
  qDebug() << "QTestApp::subscriptionIndication()";

  if(subscriptions.contains( sender()->objectName() ) )
    {
      TemplateSubscription *q = NULL;
      q = subscriptions[ sender()->objectName() ];
      if( !subsView )
	{
	  activeSubscription =  sender()->objectName();
	  QList<Triple *> results = q->results();
	  QList<Triple *>::iterator it;

	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(950,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  subsView = new QTreeWidget(tripled);
	  subsView->setColumnCount(3);
	  subsView->setColumnWidth(0,300);
	  subsView->setColumnWidth(1,300);
	  subsView->setColumnWidth(2,300);
	  QStringList headers;
	  headers << tr("subject");
	  headers << tr("predicate");
	  headers << tr("object");
	  subsView->setHeaderLabels(headers);
	  qDebug() << "Got: " << results.count() << "triples";

          QList<QTreeWidgetItem *> items;
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      QStringList strings;
	      strings << (*it)->subject().node();
	      strings << (*it)->predicate().node();
	      if( !(*it)->object().node().isEmpty())
		strings << (*it)->object().node();
	      else
		strings << "<empty>";
	      
	      QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	      
	      if((*it)->object().type() == TripleElement::ElementTypeLiteral)
		{
		  if(  !(*it)->object().node().isEmpty())
		    {
		    QColor blue = QColor(0,0,255);
		    item->setForeground(2,QBrush( blue ));
		    }
		  else
		    {
		      QColor green = QColor(0,255,0);
		      item->setForeground(2,QBrush( green ));
		    }
		}

	      items.append(item);
	    }
	  subsView->insertTopLevelItems(0,items);

	  layout->addWidget( subsView, 0,0);
	  
	  QPushButton *q_button = new QPushButton("Unsubscribe");

	  layout->addWidget( q_button, 1,0);
	  
	  connect( q_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  connect( q_button, SIGNAL(clicked()), this, SLOT(unsubscribe()) );
	  connect( subsView, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int)),
		   this, SLOT( copyTriple(QTreeWidgetItem *, int))); 

	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QList<Triple *> resultsAdded = q->resultsAdded();
	  QList<Triple *> resultsObs = q->resultsObsolete();
	  QList<Triple *>::iterator it;
	  QTreeWidgetItem *rootItem = subsView->invisibleRootItem();
          qDebug() << "Got: " << resultsAdded.count() << " new triples";
          qDebug() << "Got: " << resultsObs.count() << " obsolete triples";
	  for(it=resultsObs.begin();it != resultsObs.end(); ++it)
	    {
	      
	      for(int ii=0; ii < rootItem->childCount(); ii++)
		{
		  QTreeWidgetItem *child = rootItem->child(ii);
		  if( ( (*it)->subject().node().compare(child->text(0)) == 0) &&
		      ( (*it)->predicate().node().compare(child->text(1)) == 0) &&
		      (  ( (*it)->object().node().isEmpty() && (child->text(2).compare("<empty>")==0) ) ||
			 ((*it)->object().node().compare(child->text(2)) == 0) ) )
		    {
		      QTreeWidgetItem *c1 = rootItem->takeChild(ii);
		      delete c1;
		      break;
		    }
		}
	    }
	  if(resultsAdded.count())
	    {
	      QList<QTreeWidgetItem *> items;
	      for(it = resultsAdded.begin(); it != resultsAdded.end(); ++it)
		{
		  QStringList strings;
		  strings << (*it)->subject().node();
		  strings << (*it)->predicate().node();
		  if( !(*it)->object().node().isEmpty())
		    strings << (*it)->object().node();
		  else
		    strings << "<empty>";
	      
		  QTreeWidgetItem *item = new QTreeWidgetItem( strings );
		  if((*it)->object().type() == TripleElement::ElementTypeLiteral)
		    {
		      if(  !(*it)->object().node().isEmpty())
			{
			  QColor blue = QColor(0,0,255);
			  item->setForeground(2,QBrush( blue ));
			}
		      else
			{
			  QColor green = QColor(0,255,0);
			  item->setForeground(2,QBrush( green ));
			}
		    }
		  items.append(item);
		}
	      subsView->addTopLevelItems(items);
	    }
	}
    }

}

void QTestApp::wqlrelatedindication()
{
  qDebug() << "wqlrelatedindication()";
  if(wqlrelated_subscriptions.contains( sender()->objectName() ) )
    {
      WqlRelatedSubscription *q = NULL;
      q = wqlrelated_subscriptions[ sender()->objectName() ];
      if( !subsView )
	{
	  activeSubscription =  sender()->objectName();

	  bool results = q->results();
	  
	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  subsView = new QTreeWidget(tripled);
	  subsView->setColumnCount(1);
	  subsView->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("Related");
	  subsView->setHeaderLabels(headers);
	  

	  QList<QTreeWidgetItem *> items;
	  QStringList strings;
	  if(results)
	    strings << "TRUE";
	  else
	    strings << "FALSE";
	  
	  QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	  items.append(item);
	  subsView->insertTopLevelItems(0,items);
	  
	  layout->addWidget( subsView, 0,0);
      
	  QPushButton *q_button = new QPushButton("Unsubscribe");
	  
	  layout->addWidget( q_button, 1,0);
	  
	  connect( q_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  connect( q_button, SIGNAL(clicked()), this, SLOT(unsubscribe()) );

	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  bool results = q->results();
	  qDebug() << "wqlrelatedindication() " << results;

	  QTreeWidgetItem *rootItem = subsView->invisibleRootItem();
	  
	  for(int ii=0; ii < rootItem->childCount(); ii++)
	    {
	      QTreeWidgetItem *c1 = rootItem->takeChild(ii);
	      delete c1;
	    }
	  QList<QTreeWidgetItem *> items;
	  QStringList strings;
	  if(results)
	    strings << "TRUE";
	  else
	    strings << "FALSE";
	  
	  QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	  items.append(item);
	  subsView->addTopLevelItems(items);
	  
	}
    }
}

void QTestApp::wqlvaluesindication()
{
  qDebug() << "wqlvaluesindication()";
  if(wqlvalues_subscriptions.contains( sender()->objectName() ) )
    {
      WqlValuesSubscription *q = NULL;
      q = wqlvalues_subscriptions[ sender()->objectName() ];
      if( !subsView )
	{
	  activeSubscription =  sender()->objectName();

	  QList<TripleElement> results = q->results();
	  
	  QDialog *tripled = new QDialog(this);
	  tripled->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  tripled->setFixedSize( QSize(350,500));
	  QGridLayout *layout = new QGridLayout(tripled);
	  //layout->setGeometry( QRect( QPoint(100,100), QSize(600,500)));
	  subsView = new QTreeWidget(tripled);
	  subsView->setColumnCount(1);
	  subsView->setColumnWidth(0,300);
	  QStringList headers;
	  headers << tr("Values");
	  subsView->setHeaderLabels(headers);
	  
	  QList<QTreeWidgetItem *> items;
	  QList<TripleElement>::iterator it;
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      QStringList strings;
	      if( !(*it).node().isEmpty())
		strings << (*it).node();
	      else
		strings << "<empty>";
	      
	      QTreeWidgetItem *item = new QTreeWidgetItem( strings );
	      if( (*it).type() == TripleElement::ElementTypeLiteral)
		{
		  if( !(*it).node().isEmpty())
		    {
		      QColor blue = QColor(0,0,255);
		      item->setForeground(0, QBrush( blue) );
		    }
		  else
		    {
		      QColor green = QColor(0,255,0);
		       item->setForeground(0, QBrush( green) );
		    }
		}
	      items.append(item);
	    }
	  subsView->insertTopLevelItems(0,items);
	  
	  layout->addWidget( subsView, 0,0);
      
	  QPushButton *q_button = new QPushButton("Unsubscribe");
	  
	  layout->addWidget( q_button, 1,0);
	  
	  connect( q_button, SIGNAL(clicked()), tripled, SLOT(accept()));
	  connect( q_button, SIGNAL(clicked()), this, SLOT(unsubscribe()) );

	  layout->update();
	  tripled->adjustSize();
	  tripled->setModal(false);
	  tripled->show();
	}
      else
	{
	  QList<TripleElement> resultsObs = q->resultsObsolete();
	  QList<TripleElement> resultsAdded = q->resultsAdded();

	  QTreeWidgetItem *rootItem = subsView->invisibleRootItem();
	  QList<TripleElement>::iterator it;
	  for(it=resultsObs.begin();it != resultsObs.end(); ++it)
	    {
	      
	      for(int ii=0; ii < rootItem->childCount(); ii++)
		{
		  QTreeWidgetItem *child = rootItem->child(ii);
		  if( ( (*it).node().isEmpty() && (child->text(0).compare("<empty>")==0) ) ||
		      ( (*it).node().compare(child->text(0)) == 0) )
		    {
		      QTreeWidgetItem *c1 = rootItem->takeChild(ii);
		      delete c1;
		      break;
		    }
		}
	    }
	  if(resultsAdded.count())
	    {
	      QList<QTreeWidgetItem *> items;
	      for(it = resultsAdded.begin(); it != resultsAdded.end(); ++it)
		{
		  QStringList strings;
		  if( !(*it).node().isEmpty())
		    strings << (*it).node();
		  else
		    strings << "<empty>";

		  qDebug() << "newResult:" << (*it).node();
		  QTreeWidgetItem *item = new QTreeWidgetItem( strings );

		  if( (*it).type() == TripleElement::ElementTypeLiteral)
		    {
		      if( !(*it).node().isEmpty())
			{
			  QColor blue = QColor(0,0,255);
			  item->setForeground(0, QBrush( blue) );
			}
		      else
			{
			  QColor green = QColor(0,255,0);
			  item->setForeground(0, QBrush( green) );
			}
		    }
		  items.append(item);
		}
	      subsView->addTopLevelItems(items);
	    }
	}
    }
}


void QTestApp::unsubscribe()
{
  if( !activeSubscription.isEmpty())
    {
      if( subscriptions.contains( activeSubscription ) )
	{
	  TemplateSubscription *s = subscriptions.take( activeSubscription );
	  s->unsubscribe();
	}
      else if (wqlvalues_subscriptions.contains( activeSubscription ) )
	{
	  WqlValuesSubscription *s = wqlvalues_subscriptions.take( activeSubscription );
	  s->unsubscribe();
	}
      else if(wqlrelated_subscriptions.contains( activeSubscription ) )
	{
	  WqlRelatedSubscription *s = wqlrelated_subscriptions.take( activeSubscription );
	  s->unsubscribe();
	}
      activeSubscription.clear();
      if(subsView)
	{
	  delete subsView;
	  subsView=NULL;
	}
    }
}


void QTestApp::queryComplete(int success)
{

  if(queries.contains( sender()->objectName() ) )
    {
      TemplateQuery *q = NULL;
      q = queries[ sender()->objectName() ];
      qDebug() << "QTestApp::queryComplete: query " << q->objectName() << "complete(" << success << ")";
      if(success == 0)
	{
	  QList<Triple *> results = q->results();
	  QList<Triple *>::iterator it;
	  qDebug() << "Got " << results.count() << " triples";
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      (*it)->print();
	    }
	}
      queries.remove( sender()->objectName() );

    }
  else if( wqlvalues_queries.contains( sender()->objectName() ))
    {
      WqlValuesQuery *q = NULL;
      q = wqlvalues_queries[ sender()->objectName() ];
      qDebug() << "QTestApp::queryComplete: query " << q->objectName() << "complete(" << success << ")";
      if(success == 0)
	{
	  QList<TripleElement> results = q->results();
	  QList<TripleElement>::iterator it;
	  qDebug() << "Got " << results.count() << " uris/literals";
	  for(it = results.begin(); it != results.end(); ++it)
	    {
	      qDebug() << (*it).node();
	    }
	}
    }
  else if( wqlrelated_queries.contains( sender()->objectName() ))
    {
      WqlRelatedQuery *q = NULL;
      q = wqlrelated_queries[ sender()->objectName() ];
      qDebug() << "QTestApp::queryComplete: query " << q->objectName() << "complete(" << success << ")";
      if(success == 0)
	{
	  bool results = q->results();
	  if( results )
	    qDebug() << "Results: TRUE";
	  else
	    qDebug() << "Results: FALSE";
	}
    }
  
}

void QTestApp::subscriptionFinished(int success)
{
  SubscriptionBase *q = NULL;
  qDebug() << "QTestApp::subscriptionFinished";
  if(subscriptions.contains( sender()->objectName() ) )
    {
      q = subscriptions.take(sender()->objectName());
      qDebug() << "QTestApp::subscriptionFinished: subscription " << q->objectName() << "finished(" << success << ")";
      
      delete q;
    }
  else if(wqlrelated_subscriptions.contains( sender()->objectName() ) )
    {
      q = wqlrelated_subscriptions.take(sender()->objectName());
      qDebug() << "QTestApp::subscriptionFinished: subscription " << q->objectName() << "finished(" << success << ")";
      
      delete q;
    }
  else if(wqlvalues_subscriptions.contains( sender()->objectName() ) )
    {
      q = wqlvalues_subscriptions.take(sender()->objectName());
      qDebug() << "QTestApp::subscriptionFinished: subscription " << q->objectName() << "finished(" << success << ")";
      
       delete q;
    }
}

#if 0
void QTestApp::subscriptionIndication()
{
  SubscriptionBase *q = NULL;
  if(subscriptions.contains( sender()->objectName() ) )
    {
      q = subscriptions[ sender()->objectName() ];
      subCounter++;
      QList<Triple *> results = static_cast<TemplateSubscription *>(q)->results();
      QList<Triple *>::iterator it;
      qDebug() << "QTestApp::subscriptionIndication: subscription " << q->objectName()
	       << "Count: " << subCounter;
      qDebug() << "Got " << results.count() << " triples";
      for(it = results.begin(); it != results.end(); ++it)
	{
	  (*it)->print();
	}
    }

}
#endif
void QTestApp::joinComplete(int success)
{
  QWhiteBoardNode *node = static_cast<QWhiteBoardNode *>(sender());
  if(node)
    {
      QString sib =  nodes.key(node);
      if(success<0)
	{
	  QMessageBox::critical(this, "Error","Join Failed");
          ui->statusbar->showMessage(tr("Not joined"));
	}
      else
	{
           ui->statusbar->showMessage(tr("Joined"));
	}
    }
  else
    {
      QMessageBox::critical(this, "Error","Internal Join error");
      ui->statusbar->showMessage(tr("Not joined"));
    }
}

void QTestApp::sibList( const QList<QSibInfo *> &siblist )
{

  QList<QSibInfo *>::const_iterator it;
  qDebug() << "QTestApp::sibList, len: " << siblist.size();
  for(it = siblist.begin(); it != siblist.end(); ++it)
    {
      qDebug() << "SIB found:";
      qDebug() << "Uri: " << (*it)->uri();
      qDebug() << "Name: " << (*it)->name();

      QStringList new_sib;
      new_sib << (*it)->uri()
	      << (*it)->name();
      QString label = new_sib.join("--");
      if( ui->sib_list->findItems( label, Qt::MatchStartsWith ).count() == 0)
        ui->sib_list->addItem( label );
    }
}

void QTestApp::sibInserted( const QSibInfo *sib)
{
  QStringList new_sib;
  new_sib << sib->uri()
	      << sib->name();
      QString label = new_sib.join("--");
      if( ui->sib_list->findItems( label, Qt::MatchStartsWith ).count() == 0)
        ui->sib_list->addItem( label );
}

void QTestApp::sibRemoved( QString uri)
{
  qDebug() << "SIB removed:";
  qDebug() << "URI: "<< uri;
  QList<QListWidgetItem *> items =  ui->sib_list->findItems( uri, Qt::MatchStartsWith );
  QList<QListWidgetItem *>::iterator item_it;
  for( item_it = items.begin(); item_it != items.end(); item_it++)
    {
      qDebug() << "removing " << (*item_it)->text();
      QListWidgetItem *item = ui->sib_list->takeItem( ui->sib_list->row(*item_it) );
      delete item;
    }
}

//node = new QWhiteBoardNode(&app);
//(node == NULL )
//
//  qCritical("Cannot create QWhiteBoardNode instance.");
//  return -1;
//}
//node->unregister();

void QTestApp::on_sib_list_doubleClicked(QModelIndex index)
{

    QListWidgetItem *item = ui->sib_list->item( index.row()  );
    QStringList txt = item->text().split("--");
    QString sib_uri = txt[0];
    if( nodes.contains(sib_uri) )
    {
        leaveSelected();
    }
    else
    {
        joinSelected();
    }
}

