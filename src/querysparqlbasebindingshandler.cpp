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

#include <QDebug>
#include "querysparqlbasebindingshandler.h"

const QString internalParsingError("internal parsing error");//parsing lost in the woods.



QuerySparqlBaseBindingsHandler::QuerySparqlBaseBindingsHandler( QList<TripleElement> *tripleElements,
								QList<QList<TripleElement *> > *resultRows,
								QList<QString> *selectedVariables,
								QList<QList<TripleElement*>*> *results)
  :QXmlDefaultHandler()
  ,m_errorString()
  ,m_inBindingsList(false)
  ,m_inHead(false)
  ,m_inVariable(false)
  ,m_doneVariableList(false)
  ,m_parsedCharTxt()
  ,m_valueIndex(-1)
  ,m_tripleElements(tripleElements)
  ,m_resultRows(resultRows)
  ,m_selectedVariables(selectedVariables)
  ,m_resultsTable(results)

{
}

QuerySparqlBaseBindingsHandler::~QuerySparqlBaseBindingsHandler()
{
}


const QString errrorContentNotInBinding ("unexpected character content outside of <uri> or <literal>");

// from QXmlContentHandler
bool QuerySparqlBaseBindingsHandler::characters ( const QString & ch )
{
  if ( !ch.trimmed().isEmpty())
    {
      if (!m_inBinding)
	{
	  m_errorString = errrorContentNotInBinding;
	  return false;
	}

      m_parsedCharTxt.append(ch);
    }
  return true;
}

bool QuerySparqlBaseBindingsHandler::endDocument ()
{
  return true;
}

bool QuerySparqlBaseBindingsHandler::endElement ( const QString & namespaceURI, const QString & localName, const QString & qName )
{
  qDebug() << "end element: " << qName;

  if(m_inBindingsList)
    {
      if(!m_doneVariableList)
	{
	  if (m_inHead && m_inVariable && qName.compare("variable") == 0)
	    {
	      m_inVariable = false;
	      return true;
	    }
	  else if(m_inHead && !m_inVariable && qName.compare("head") == 0)
	    {
	      m_inHead = false;
	      m_doneVariableList = true;
	      return true;
	    }
	  else //not in <head>
	    {
	      m_errorString = internalParsingError;
	      return false;
	    }
	}

      //doing required bindings
      //first, internal check..
      if (m_inResult && m_resultsTable->isEmpty() || m_resultRows->last().size() != m_selectedVariables->size())
	{
	  m_errorString = "internal error, results table improperly initialized";
	  return false;
	}
      if (m_inResults)
	if(m_inResult && m_inBinding && !m_parsedCharTxt.isEmpty() && (qName.compare("uri")==0 || qName.compare("literal")==0))
	  { //binding has been define for defined triple element value
	    //so, add a triple element to the internal triple list and
	    //update (overwrite NULL), for the current (m_valueIndex) variable's value with the current (last on list) triple address
	    TripleElement tE (m_parsedCharTxt,
			      (qName.compare("uri")==0)?
			      TripleElement::ElementTypeURI:
			      TripleElement::ElementTypeLiteral);
	    *m_tripleElements << tE;
	    (m_resultsTable->last())->replace(m_valueIndex, &m_tripleElements->last());
	    m_parsedCharTxt.clear();
	    m_valueIndex = -1;
	    return true;
	  }
	else if(m_inResult && m_inBinding && qName.compare("binding")==0)
	  {
	    m_inBinding = false;
	    return true;
	  }
	else if(m_inResult && qName.compare("result")==0)
	  {
	    m_inResult = false;
	    return true;
	  }
	else if(qName.compare("results")==0)
	  {
	    m_inResults = false;
	    return true;
	  }
	else
	  {
	    m_errorString = internalParsingError;
	    return false;
	  }

      if( qName.compare("sparql_results")==0)
	{
	  m_inBindingsList = false;
	  return true;
	}
      else
	{
	  m_errorString = internalParsingError;
	  return false;
	}
    }

  m_errorString = internalParsingError;
  return false;

#if 0 //just for model / old stuff:
       else if(!m_componentTxt.isEmpty() && (m_elementName.compare(qName) == 0) )
	{
	  TripleElement::ElementType type;
	  if(m_elementName.compare("literal") == 0)
	    type =  TripleElement::ElementTypeLiteral;
	  else
	    type =  TripleElement::ElementTypeURI;

	  TripleElement el(m_componentTxt, type);
//***>	  m_resultsList->append( el );
	  m_componentTxt.clear();
	  m_elementName.clear();
	}
#endif
}

bool QuerySparqlBaseBindingsHandler::endPrefixMapping ( const QString & prefix )
{
  m_errorString = "endPrefixMapping not supported";
  return false;
}


bool QuerySparqlBaseBindingsHandler::ignorableWhitespace ( const QString & ch )
{
  m_errorString = "ignorableWhitespace not supported";
  return false;
}

bool QuerySparqlBaseBindingsHandler::processingInstruction ( const QString & target, const QString & data )
{
m_errorString = "processingInstruction not supported";
  return false;
}

void QuerySparqlBaseBindingsHandler::setDocumentLocator ( QXmlLocator * locator )
{

}

bool QuerySparqlBaseBindingsHandler::skippedEntity ( const QString & name )
{
  m_errorString = "skippedEntity not supported";
  return false;
}

bool QuerySparqlBaseBindingsHandler::startDocument ()
{
  return true;
}
  
bool QuerySparqlBaseBindingsHandler::startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts )
{
  qDebug() << "start element: " << qName << atts.count() << " attributes: ";
  for (int i = atts.count(), j = 0; i > 0; --i,++j)
    qDebug() << "  " << atts.qName(j) << "=" << atts.value(j);

  if(!m_inBindingsList)
    if( qName.compare("sparql_results")==0 )
      {
	m_inBindingsList = true;
	return true;
      }
    else
      {
	m_errorString = "Parsed message must start with element sparql_bindings_list, not with: ";
	m_errorString.append(qName);
	return false;
      }

  if(!m_doneVariableList)
    {
      if (!m_inHead)
	if( qName.compare("head")==0 )
	  {
	    m_inHead = true;
	    return true;
	  }
	else
	  {
	    m_errorString = "1st element within first (only) <sparql_results> must be <head>, not: ";
	    m_errorString.append(qName);
	    return false;
	  }

      if( !m_inVariable && qName.compare("variable")==0 && atts.count()==1 && atts.index("name")==0 )
	{
	  m_inVariable = true;
	  *m_selectedVariables << atts.value(0);
	  return true;
	}
      else
	{
	  if (m_inVariable)
	    m_errorString = "new element without closing previous <variable> element";
	  else if (!qName.compare("variable"))
	    m_errorString = m_errorString = "starting something other than <variable> element in <head> list";
	  else if (atts.count()==0)
	    m_errorString = "missing \"name\" attribute";
	  else if (atts.index("name")!=0)
	    m_errorString = "only \"name\" attribute allowed in <variable> element";
	  return false;
	}
    }

  if (!m_inResults)
    if( qName.compare("results")==0 )
      {
	m_inResults = true;
	return true;
      }
    else
      {
	m_errorString = "expected element <results>, not : ";
	m_errorString.append(qName);
	return false;
      }

  if (!m_inResult)
    if( qName.compare("result")==0 )
      { //initialize a new result row with NULLs; replaced if appropriate <binding..> is parsed.
	QList<TripleElement*> list;
	for (int i=0; i < m_selectedVariables->size(); ++i)
	  list << NULL;
	*m_resultRows << list;
	*m_resultsTable << &m_resultRows->last();
	m_inResult = true;
	return true;
      }
    else
      {
	m_errorString = "expected element <result>, not : ";
	m_errorString.append(qName);
	return false;
      }

  if (!m_inBinding && qName.compare("binding")==0 && atts.count()==1 &&
      atts.index("name")==0 && (m_valueIndex=(*m_selectedVariables).indexOf(atts.value(0))) != -1)
    {
      m_inBinding = true;
      return true;
    }
  else
    {
      if (m_inBinding)
	{
	  if (m_parsedCharTxt.isEmpty() && ( qName.compare("uri")==0 || qName.compare("literal")==0 ))
	    return true;
	  else if (!m_parsedCharTxt.isEmpty())
	    m_errorString = errrorContentNotInBinding;
	  else
	    m_errorString = "new element withing <binding> which repeats or is not <uri> or <literal>";
	}
      else if (!qName.compare("binding")) {
	m_errorString = "starting <";
	m_errorString.append( qName );
	m_errorString.append( "> when <binding> is expected after <result>");
      }
      else if (atts.count()==0)
	m_errorString ="missing \"name\" attribute";
      else if (atts.index("name")!=0)
	m_errorString = "only \"name\" attribute allowed in <binding> element";
      else
	m_errorString = "attribute value is not a selected variable";

      return false;
    }

  //should have returned before this
  m_errorString = internalParsingError;
  return false;
}

bool QuerySparqlBaseBindingsHandler::startPrefixMapping ( const QString & prefix, const QString & uri )
{
  m_errorString = "startPrefixMapping not supported";
  return false;
}


  // from QXmlErrorHandler

bool QuerySparqlBaseBindingsHandler::error ( const QXmlParseException & exception )
{
}

QString QuerySparqlBaseBindingsHandler::errorString () const
{
  return m_errorString;
}

bool QuerySparqlBaseBindingsHandler::fatalError ( const QXmlParseException & exception )
{
}

bool QuerySparqlBaseBindingsHandler::warning ( const QXmlParseException & exception )
{
}
