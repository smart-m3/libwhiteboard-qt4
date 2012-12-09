
#include "qapplication.h"
#include "q_testapp.h"
#include "q_whiteboard_node.h"
#include "q_sib_discovery.h"
#include <glib-object.h>


int main(int argc, char **argv)
{

  QSibDiscovery *dis;
  QTestApp *t_app;
  QApplication app(argc, argv);
  g_type_init();
  dbus_g_thread_init();
  
  dis = new QSibDiscovery(&app);
  t_app = new QTestApp;
  
  QObject::connect( dis, SIGNAL( sibList(const QList<QSibInfo *>& ) ),
		    t_app, SLOT( sibList(const QList<QSibInfo *>& ) ) );


  QObject::connect( dis, SIGNAL( sibInserted(const QSibInfo *) ),
		    t_app, SLOT( sibInserted(const QSibInfo * ) ) );

  QObject::connect( dis, SIGNAL( sibRemoved( QString) ),
		    t_app, SLOT( sibRemoved( QString ) ) );


  dis->getSibs();
  t_app->show();

  return app.exec();
}

