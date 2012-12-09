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

#include "querywqlbaselisthandler.h"


QueryWqlBaseListHandler::QueryWqlBaseListHandler( QList<TripleElement> *list)
    :QXmlDefaultHandler()
    ,m_errorString()
    ,m_inNodeList(false)
    ,m_elementName()
    ,m_componentTxt()
    ,m_resultsList(list)

{
}

QueryWqlBaseListHandler::~QueryWqlBaseListHandler()
{
}



// from QXmlContentHandler
bool QueryWqlBaseListHandler::characters ( const QString & ch )
{
    if ( !ch.trimmed().isEmpty())
    {
        m_componentTxt.append(ch);
    }
    return true;
}

bool QueryWqlBaseListHandler::endDocument ()
{
    return true;
}

bool QueryWqlBaseListHandler::endElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName )
{
    if(m_inNodeList)
    {
        if( qName.compare("node_list") == 0)
	{
            m_inNodeList = false;
	}
        else if((m_elementName.compare(qName) == 0) )
	{
            TripleElement::ElementType type;
            if(m_elementName.compare("literal") == 0)
                type =  TripleElement::ElementTypeLiteral;
            else
                type =  TripleElement::ElementTypeURI;
	    if( (type ==  TripleElement::ElementTypeLiteral) || 
		( !m_componentTxt.isEmpty() ) )
	      {
		TripleElement el(m_componentTxt, type);
		m_resultsList->append( el );
	      }
	    else
	      {
		m_errorString = "Empty URI";
		return false;
	      }
	    m_componentTxt.clear();
	    m_elementName.clear();
	}
        else
	{
	  m_errorString = "Unknown element end: ";
	  m_errorString.append(qName);
	  return false;
	}
    }
    else
    {
        m_errorString = "nodelist not started.";
        return false;
    }
    return true;
}

bool QueryWqlBaseListHandler::endPrefixMapping ( const QString & /*prefix*/ )
{
    m_errorString = "endPrefixMapping not supported";
    return false;
}


bool QueryWqlBaseListHandler::ignorableWhitespace ( const QString & /*ch*/ )
{
    m_errorString = "ignorableWhitespace not supported";
    return false;
}

bool QueryWqlBaseListHandler::processingInstruction ( const QString & /*target*/, const QString & /*data*/ )
{
    m_errorString = "processingInstruction not supported";
    return false;
}

void QueryWqlBaseListHandler::setDocumentLocator ( QXmlLocator * /* locator */)
{

}

bool QueryWqlBaseListHandler::skippedEntity ( const QString & /* name */)
{
    m_errorString = "skippedEntity not supported";
    return false;
}

bool QueryWqlBaseListHandler::startDocument ()
{
    return true;
}

bool QueryWqlBaseListHandler::startElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName, const QXmlAttributes & /*atts*/ )
{

    if(!m_inNodeList)
    {
        if (qName.compare("node_list")==0 )
        {
            m_inNodeList = true;
            return true;
        }
        else
        {
            m_errorString = "Parsed message must start with element node_list, not with: ";
            m_errorString.append(qName);
            return false;
        }
    }

    if(m_elementName.isEmpty() )
    {
        if( (qName.compare("uri") == 0) || (qName.compare("literal") == 0))
	{
            m_elementName = qName;
	}
        else
	{
            m_errorString = "starting something other than uri or literal element";
            return false;
	}
    }
    else if( m_elementName.compare(qName) )
    {
        m_errorString = "Element " ;
        m_errorString.append(qName);
        m_errorString.append(" following element ");
        m_errorString.append(m_elementName);
        return false;
    }
    return true;
}

bool QueryWqlBaseListHandler::startPrefixMapping ( const QString & /*prefix*/, const QString & /*uri*/ )
{
    m_errorString = "startPrefixMapping not supported";
    return false;
}


// from QXmlErrorHandler

bool QueryWqlBaseListHandler::error ( const QXmlParseException & /*exception*/ )
{
    return false;
}

QString QueryWqlBaseListHandler::errorString () const
{
    return m_errorString;
}

bool QueryWqlBaseListHandler::fatalError ( const QXmlParseException & /*exception*/ )
{
    return false;
}

bool QueryWqlBaseListHandler::warning ( const QXmlParseException & /*exception*/ )
{
    return false;
}
