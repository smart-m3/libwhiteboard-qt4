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

#include "querywqlbaseboolhandler.h"


QueryWqlBaseBoolHandler::QueryWqlBaseBoolHandler( bool *result)
  :QXmlDefaultHandler()
  ,m_errorString()
  ,m_componentTxt()
  ,m_inParameter(false)
  ,m_results(result)

{
}

QueryWqlBaseBoolHandler::~QueryWqlBaseBoolHandler()
{
}



  // from QXmlContentHandler
bool QueryWqlBaseBoolHandler::characters ( const QString & ch )
{
  if ( ch.trimmed().isEmpty())
    {
      return true;
    }
  else if(m_inParameter)
    {
      m_componentTxt.append(ch);
      return true;
    }
  return false;
}

bool QueryWqlBaseBoolHandler::endDocument ()
{
  return true;
}

bool QueryWqlBaseBoolHandler::endElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName )
{
  if( m_inParameter && qName.compare("parameter") == 0)
    {
      if( m_componentTxt.compare("true", Qt::CaseInsensitive) == 0)
	{
	  *m_results = true;
	}
      else if(m_componentTxt.compare("false", Qt::CaseInsensitive) == 0)
	{
	  *m_results = false;
	}
      else
	{
	  m_errorString = "Invalid results: ";
	  m_errorString.append(m_componentTxt);
	  return false;
	}
    }
  else
    {
      m_errorString = "ending something else than parameter or parameter not started";
      return false;
    }
  return true;
}

bool QueryWqlBaseBoolHandler::endPrefixMapping ( const QString & /*prefix*/ )
{
  m_errorString = "endPrefixMapping not supported";
  return false;
}


bool QueryWqlBaseBoolHandler::ignorableWhitespace ( const QString & /*ch*/ )
{
  m_errorString = "ignorableWhitespace not supported";
  return false;
}

bool QueryWqlBaseBoolHandler::processingInstruction ( const QString & /*target*/, const QString & /*data*/ )
{
m_errorString = "processingInstruction not supported";
  return false;
}

void QueryWqlBaseBoolHandler::setDocumentLocator ( QXmlLocator */* locator*/ )
{

}

bool QueryWqlBaseBoolHandler::skippedEntity ( const QString & /*name*/ )
{
  m_errorString = "skippedEntity not supported";
  return false;
}

bool QueryWqlBaseBoolHandler::startDocument ()
{
  return true;
}
  
bool QueryWqlBaseBoolHandler::startElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName, const QXmlAttributes & atts )
{

  if (qName.compare("parameter")==0 )
    {
      if( (atts.length() == 1) && 
	  (atts.qName(0).compare("name") == 0) &&
	  ( atts.value(0).compare("results") == 0 ) ) 
	{
	  m_inParameter = true;
	}
      else
	{
	  m_errorString = "Invalid attributes for element parameter";
	  for(int ii=0; ii<atts.length();ii++)
	    {
	      m_errorString.append("\nname: ");
	      m_errorString.append(atts.qName(ii));
	      m_errorString.append("\tvalue: ");
	      m_errorString.append(atts.value(ii));
	      return false;
	    }
	}
    }
  else
    {
      m_errorString = "Invalid element: ";
      m_errorString.append(qName);
      return false;
    }
  
  return true;
}

bool QueryWqlBaseBoolHandler::startPrefixMapping ( const QString & /*prefix*/, const QString & /*uri*/ )
{
  m_errorString = "startPrefixMapping not supported";
  return false;
}


  // from QXmlErrorHandler

bool QueryWqlBaseBoolHandler::error ( const QXmlParseException & /* exception*/ )
{
    return false;
}

QString QueryWqlBaseBoolHandler::errorString () const
{
  return m_errorString;
}

bool QueryWqlBaseBoolHandler::fatalError ( const QXmlParseException & /*exception*/ )
{
    return false;
}

bool QueryWqlBaseBoolHandler::warning ( const QXmlParseException & /*exception*/ )
{
    return false;
}
