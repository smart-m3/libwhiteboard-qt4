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
#include "templatequeryhandler.h"


TemplateQueryHandler::TemplateQueryHandler( QList<Triple *> *list)
  :QXmlDefaultHandler()
  ,m_errorString()
  ,m_inTripleList(false)
  ,m_currentTriple(NULL)
  ,m_componentTxt()
  ,m_resultsList(list)

{
}

TemplateQueryHandler::~TemplateQueryHandler()
{
}



  // from QXmlContentHandler
bool TemplateQueryHandler::characters ( const QString & ch )
{
  //qDebug() << "characters ( ): " << ch;
  if ( ch.trimmed().isEmpty())
    {
      return true;
    }
  else if( m_inTripleList && m_currentTriple)
    {
      m_componentTxt.append(ch);
    }
  else
    {
      m_errorString = "no current triple ";
      if( m_currentTriple )
	m_errorString.append("(non-empty)");
      else
	m_errorString.append("(empty)");
      

      m_errorString.append(", in tripleList ");

     if( m_inTripleList )
	m_errorString.append("true");
      else
	m_errorString.append("false");

      return false;
    }
  return true;
}

bool TemplateQueryHandler::endDocument ()
{
  return true;
}

bool TemplateQueryHandler::endElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName )
{
  //qDebug()<< "endElement: " << qName;
  if(m_inTripleList)
    {
      if(m_currentTriple )
	{
	  if(!m_componentTxt.isEmpty() && qName.compare("subject") == 0)
	    {
	      TripleElement el(m_componentTxt, TripleElement::ElementTypeURI);
	      if(!m_currentTriple->setSubject(el))
		{
		  m_errorString = "Triple already contains subject";
		  return false;
		}
	      m_componentTxt.clear();
	    }
	  else if(!m_componentTxt.isEmpty() && qName.compare("predicate") == 0)
	    {
	      TripleElement el(m_componentTxt, TripleElement::ElementTypeURI);
	      if(!m_currentTriple->setPredicate(el))
		{
		  m_errorString = "Triple already contains predicate";
		  return false;
		}
	      m_componentTxt.clear();
	    }
	  else if( qName.compare("object") == 0)
	    {
	      TripleElement el(m_componentTxt, m_elementType);
	      if(!m_currentTriple->setObject( el))
		{
		  m_errorString = "Triple already contains object";
		  return false;
		}
	      m_componentTxt.clear();
	    }
	  else if( (qName.compare("triple") == 0) &&
		   m_currentTriple->isComplete()  )
	    {
	      m_resultsList->append(m_currentTriple);
	      m_currentTriple= NULL;
	    }
	  else
	    {
	      m_errorString = "Invalid component name, empty content or missing triple component:";
	      m_errorString.append(qName);
	      return false;
	    }
	}
      else if( qName.compare("triple_list") == 0)
	{
	  m_inTripleList = false;
	}
      else
	{
	  m_errorString = "internal error";
	  return false;
	}
    }
  else
    {
      m_errorString = "triple list not started.";
      return false;
    }
  return true;
}

bool TemplateQueryHandler::endPrefixMapping ( const QString & /*prefix*/ )
{
  m_errorString = "endPrefixMapping not supported";
  return false;
}


bool TemplateQueryHandler::ignorableWhitespace ( const QString & /*ch*/ )
{
  m_errorString = "ignorableWhitespace not supported";
  return false;
}

bool TemplateQueryHandler::processingInstruction ( const QString & /*target*/, const QString & /*data*/ )
{
m_errorString = "processingInstruction not supported";
  return false;
}

void TemplateQueryHandler::setDocumentLocator ( QXmlLocator * /*locator*/ )
{

}

bool TemplateQueryHandler::skippedEntity ( const QString & /*name*/ )
{
  m_errorString = "skippedEntity not supported";
  return false;
}

bool TemplateQueryHandler::startDocument ()
{
  return true;
}
  
bool TemplateQueryHandler::startElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & qName, const QXmlAttributes & atts )
{
  //qDebug() << "startElement: " << qName;
  if(!m_inTripleList && (qName.compare("triple_list")==0 ))
    {
      m_inTripleList = true;
      return true;
    }
  if(m_currentTriple)
    {
      if(qName.compare("subject") == 0 )
	{
	  // no need to check for element type, since subject are always URIs (when received)
	  m_component = ESubject;
	}
      else if(qName.compare("predicate") == 0 )
	{
	  // no need to check for element type, since predicates are always URIs 
	  m_component = EPredicate;
	}
      else if(qName.compare("object") == 0 && 
	      (atts.length() == 1) && 
	      (atts.qName(0).compare("type") == 0))
	{
	  //check elementtype for object, bNode is illegal when received.
	  QString attrvalue = atts.value(0);
	  m_component = EObject;
	  if( attrvalue.compare("literal", Qt::CaseInsensitive) == 0) 
	    m_elementType = TripleElement::ElementTypeLiteral;
	  else if (attrvalue.compare("URI", Qt::CaseInsensitive) == 0)
	    m_elementType = TripleElement::ElementTypeURI;
	  else
	    {
	      m_errorString = "Invalid object type: ";
	      m_errorString.append(attrvalue);
	      return false;
	    }
	  m_component = EObject;
	}
      else
	{
	  m_errorString = "Invalid element name for a triple: ";
	  m_errorString.append(qName);
	  return false;
	}
    }
  else
    {
      if(qName.compare("triple") == 0 )
	{
	  m_currentTriple = new Triple();
	}
      else
	{
	  m_errorString = "starting something other than triple element";
	  return false;
	}
    }
  return true;
}

bool TemplateQueryHandler::startPrefixMapping ( const QString & /*prefix*/, const QString & /*uri*/ )
{
  m_errorString = "startPrefixMapping not supported";
  return false;
}


  // from QXmlErrorHandler

bool TemplateQueryHandler::error ( const QXmlParseException & /*exception*/ )
{
    return false;
}

QString TemplateQueryHandler::errorString () const
{
  return m_errorString;
}

bool TemplateQueryHandler::fatalError ( const QXmlParseException & /*exception*/ )
{
    return false;

}

bool TemplateQueryHandler::warning ( const QXmlParseException & /*exception */)
{
    return false;
}
