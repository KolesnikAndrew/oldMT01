/*
 * XMLAttribute.h
 *
 *  Created on: 12 янв. 2015
 *      Author: rtem
 */

#ifndef XMLATTRIBUTE_H_
#define XMLATTRIBUTE_H_
#include <libxml2/libxml/globals.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xmlstring.h>
#include <cstring>
#include <iostream>

using namespace std;
class XMLAttribute
{
public:
    void CXMLAttr(xmlNodePtr Node, const string& AttributeName);
    bool IsValid() const;
    const string& Value() const;
    void CXMLNode(xmlNodePtr Node);
    XMLAttribute();
    virtual ~XMLAttribute();
private:
    void XMLCharsToWideString(xmlChar* const Attribute);
    bool _IsValid;
    string _Value;
};
#endif /* XMLATTRIBUTE_H_ */
