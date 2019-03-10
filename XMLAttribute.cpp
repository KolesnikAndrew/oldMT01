/*
 * XMLAttribute.cpp
 *
 *  Created on: 12 янв. 2015
 *      Author: rtem
 */

#include "XMLAttribute.h"

XMLAttribute::XMLAttribute(){

    _IsValid = false;
    _Value.clear();

}

XMLAttribute::~XMLAttribute()
{
    _Value.clear();

}

void XMLAttribute::XMLCharsToWideString(xmlChar* const Attribute)
{
    _Value = string((char*) Attribute);
}

void XMLAttribute::CXMLAttr(xmlNodePtr Node, const string& AttributeName)
{
    _Value.clear();
    xmlChar* const Attribute = xmlGetProp(Node,
                                          (const xmlChar*) AttributeName.c_str());

    _IsValid = (Attribute != NULL);

    if (Attribute) {
        XMLCharsToWideString(Attribute);
        xmlFree(Attribute);
    }
}

void XMLAttribute::CXMLNode(xmlNodePtr Node)
{
    _Value.clear();
    xmlChar* const Attribute = xmlNodeGetContent(Node);

    _IsValid = (Attribute != NULL);

    if (Attribute) {
        XMLCharsToWideString(Attribute);
        xmlFree(Attribute);
    }
}

bool XMLAttribute::IsValid() const
{
    return _IsValid;
}

const string& XMLAttribute::Value() const
{
    return _Value;
}
