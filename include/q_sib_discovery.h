/*
@section LICENSE

  Copyright (c) 2009, Nokia Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.  
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.  
    * Neither the name of Nokia nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/* -*- C++ -*-
 *
 */
#ifndef Q_SIB_DISCOVERY_H
#define Q_SIB_DISCOVERY_H
#include <QObject>
#include <QHash>
#include <QUuid>
#include <QMutex>
// #include "/usr/include/dbus-1.0/dbus/dbus.h"
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "q_sib_info.h"

/**
 * @file q_sib_discovery.h
 * 
 * 
 *
 */ 

class QSibDiscovery : public QObject
{
  Q_OBJECT
    
    public:
  QSibDiscovery(QObject *parent = 0);
  ~QSibDiscovery();

 public:
  void handleSibInserted( QString uri, QString name );
  void handleSibRemoved( QString uri);
  void addSib(QString uri, QString name);
  void removeSib(QString uri);
  void allForNow();
  const QSibInfo *getSib(QString uri) const;
  
  public slots:
  void getSibs();

 signals:
  void sibList( const QList<QSibInfo *> &list);
  void sibInserted( const QSibInfo *sib);
  void sibRemoved( QString uri );
 protected:
  
  int g_registerTry();
  QUuid m_uuid;
  DBusConnection *m_connection;
  
  QHash<QString,QSibInfo *> sibs;
  QMutex m_lock;
};


#endif
