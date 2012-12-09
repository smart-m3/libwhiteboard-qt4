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
#include <QDebug>
#include "insertresponsehandler.h"


InsertResponseHandler::InsertResponseHandler( QList<Triple *> *list)
  :QXmlDefaultHandler()
  ,m_errorString()
  ,m_inUriList(false)
  ,m_componentTxt()
  ,m_resultsList(list)
{
  //  qDebug() << "InsertResponseHandler::InsertResponseHandler(): # triples:" << m_resultsList->count();
}

InsertResponseHandler::~InsertResponseHandler()
{
}



  // from QXmlContentHandler
bool InsertResponseHandler::characters ( const QString & ch )
{
  if ( ch.trimmed().isEmpty())
    {
      return true;
    }
  if(!m_tag.isEmpty())
    {
      m_componentTxt.append(ch);
    }
  else
    {
      m_errorString = "current tag empty";
      return false;
    }
  return true;
}

bool InsertResponseHandler::endDocument ()
{
  return true;
}

bool InsertResponseHandler::endElement ( const QString & /*namespaceURI*/, const QString & /*localName */, const QString & qName )
{
  if(m_inUriList)
    {
      if( (qName.compare("uri") == 0) & !m_tag.isEmpty() && !m_componentTxt.isEmpty() )
	{

	  QList<Triple *>::iterator it;
	  bool ret = false;
	  //qDebug() << "InsertResponseHandler::endElement(): # triples:" << m_resultsList->count();
	  for( it = m_resultsList->begin(); it != m_resultsList->end(); it++)
	    {
	      if((*it)->bnode2uri(m_tag, m_componentTxt))
		ret = true;
	    }
	  if( !ret )
	    {
	      m_errorString = "Tag not found";
	      return false;
	    }
	  m_componentTxt.clear();
	  m_tag.clear();
	  if( !ret )
	    {
	      m_errorString = "Tag not found";
	      return false;
	    }
	}
      else if( qName.compare("urilist") == 0)
	{
	  QList<Triple *>::iterator it;
	  bool ret = true;
	  // go trough the triple list and check that no bnodes are found
	  for( it = m_resultsList->begin(); it != m_resultsList->end(); it++)
	    {
	      ret = ret && (*it)->noBnodes();
	    }
	  if(!ret)
	    {
	      m_errorString = "All tags not found";
	      return false;
	    }

	  m_inUriList = false;
	}
      else
	{
	  m_errorString = "Triple did not contain all required elements.";
	  return false;
	}
    }
  else
    {
      m_errorString = "uri list not started.";
      return false;
    }
  return true;
}

bool InsertResponseHandler::endPrefixMapping ( const QString & /*prefix*/ )
{
  m_errorString = "endPrefixMapping not supported";
  return false;
}


bool InsertResponseHandler::ignorableWhitespace ( const QString & /*ch */)
{
  m_errorString = "ignorableWhitespace not supported";
  return false;
}

bool InsertResponseHandler::processingInstruction ( const QString & /*target*/, const QString & /*data*/ )
{
m_errorString = "processingInstruction not supported";
  return false;
}

void InsertResponseHandler::setDocumentLocator ( QXmlLocator * /*locator*/ )
{

}

bool InsertResponseHandler::skippedEntity ( const QString & /*name*/ )
{
  m_errorString = "skippedEntity not supported";
  return false;
}

bool InsertResponseHandler::startDocument ()
{
  return true;
}
  
bool InsertResponseHandler::startElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName, const QXmlAttributes & atts )
{

  if(!m_inUriList && (qName.compare("urilist")==0 ))
    {
      m_inUriList = true;
      return true;
    }
  if((qName.compare("uri") == 0 )&&
     (atts.length() == 1) &&	
     (atts.qName(0).compare("tag") == 0))
    {
      //check elementtype for object, bNode is illegal when received.
      m_tag = atts.value(0);
    }
  else
    {
      m_errorString = "Invalid element or attribute";
      return false;
    }
  
  return true;
}

bool InsertResponseHandler::startPrefixMapping ( const QString & /* prefix */, const QString & /*uri*/ )
{
  m_errorString = "startPrefixMapping not supported";
  return false;
}


  // from QXmlErrorHandler

bool InsertResponseHandler::error ( const QXmlParseException & /*exception*/ )
{
    return false;
}

QString InsertResponseHandler::errorString () const
{
  return m_errorString;
}

bool InsertResponseHandler::fatalError ( const QXmlParseException & /*exception*/ )
{
    return false;
}

bool InsertResponseHandler::warning ( const QXmlParseException & /*exception*/ )
{
    return false;
}
