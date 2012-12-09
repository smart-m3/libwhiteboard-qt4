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
#ifndef QUERYWQLBASELISTHANDLER_H
#define QUERYWQLBASELISTHANDLER_H

#include <QtXml/QXmlDefaultHandler>
#include "triple.h"


class QueryWqlBaseListHandler :  public QXmlDefaultHandler
{
 public:
  QueryWqlBaseListHandler( QList<TripleElement> *list);
  virtual ~QueryWqlBaseListHandler();

  enum ComponentName {ESubject, EPredicate, EObject};
  
  // from QXmlContentHandler
  virtual bool characters ( const QString & ch );
  virtual bool endDocument ();

  virtual bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );
  virtual bool endPrefixMapping ( const QString & prefix );

  //  virtual QString errorString () const;

  virtual bool ignorableWhitespace ( const QString & ch ) ;

  virtual bool processingInstruction ( const QString & target, const QString & data );

  virtual void setDocumentLocator ( QXmlLocator * locator );

  virtual bool skippedEntity ( const QString & name );

  virtual bool startDocument ();
  
  virtual bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );

  virtual bool startPrefixMapping ( const QString & prefix, const QString & uri );


  // from QXmlErrorHandler

  virtual bool error ( const QXmlParseException & exception );

  virtual QString errorString () const;

  virtual bool fatalError ( const QXmlParseException & exception );

  virtual bool warning ( const QXmlParseException & exception );

 private:
  QString m_errorString;

  bool m_inNodeList;

  QString m_elementName;
  
  QString m_componentTxt;

  QList<TripleElement> *m_resultsList;
  
};

  


#endif

