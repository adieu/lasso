/* $Id$
 *
 * Lasso - A free implementation of the Samlerty Alliance specifications.
 *
 * Copyright (C) 2004 Entr'ouvert
 * http://lasso.entrouvert.org
 * 
 * Authors: Nicolas Clapies <nclapies@entrouvert.com>
 *          Valery Febvre <vfebvre@easter-eggs.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <lasso/xml/saml_subject_locality.h>

/*
 * Schema fragment (oasis-sstc-saml-schema-assertion-1.0.xsd):
 * 
 * <element name="SubjectLocality" type="saml:SubjectLocalityType"/>
 * <complexType name="SubjectLocalityType">
 *   <attribute name="IPAddress" type="string" use="optional"/>
 *   <attribute name="DNSAddress" type="string" use="optional"/>
 * </complexType>
 */

/*****************************************************************************/
/* private methods                                                           */
/*****************************************************************************/

#define snippets() \
	LassoSamlSubjectLocality *locality = LASSO_SAML_SUBJECT_LOCALITY(node); \
	struct XmlSnippet snippets[] = { \
		{ "IPAddress", 'a', (void**)&(locality->IPAddress) }, \
		{ "DNSAddress", 'a', (void**)&(locality->DNSAddress) }, \
		{ NULL, 0, NULL} \
	};

static LassoNodeClass *parent_class = NULL;

static xmlNode*
get_xmlNode(LassoNode *node)
{
	xmlNode *xmlnode;
	snippets();

	xmlnode = xmlNewNode(NULL, "SubjectLocality");
	xmlSetNs(xmlnode, xmlNewNs(xmlnode, LASSO_SAML_ASSERTION_HREF, LASSO_SAML_ASSERTION_PREFIX));
	lasso_node_build_xml_with_snippets(xmlnode, snippets);

	return xmlnode;
}

static int
init_from_xml(LassoNode *node, xmlNode *xmlnode)
{
	snippets();

	if (parent_class->init_from_xml(node, xmlnode))
		return -1;
	lasso_node_init_xml_with_snippets(xmlnode, snippets);

	return 0;
}


/*****************************************************************************/
/* instance and class init functions                                         */
/*****************************************************************************/

static void
instance_init(LassoSamlSubjectLocality *node)
{
	node->IPAddress = NULL;
	node->DNSAddress = NULL;
}

static void
class_init(LassoSamlSubjectLocalityClass *klass)
{
	parent_class = g_type_class_peek_parent(klass);
	LASSO_NODE_CLASS(klass)->get_xmlNode = get_xmlNode;
	LASSO_NODE_CLASS(klass)->init_from_xml = init_from_xml;
}

GType
lasso_saml_subject_locality_get_type()
{
	static GType this_type = 0;

	if (!this_type) {
		static const GTypeInfo this_info = {
			sizeof (LassoSamlSubjectLocalityClass),
			NULL,
			NULL,
			(GClassInitFunc) class_init,
			NULL,
			NULL,
			sizeof(LassoSamlSubjectLocality),
			0,
			(GInstanceInitFunc) instance_init,
		};

		this_type = g_type_register_static(LASSO_TYPE_NODE,
				"LassoSamlSubjectLocality", &this_info, 0);
	}
	return this_type;
}

/**
 * lasso_saml_subject_locality_new:
 * 
 * Creates a new <saml:SubjectLocality> node object.
 * 
 * Return value: the new @LassoSamlSubjectLocality
 **/
LassoNode* lasso_saml_subject_locality_new()
{
	return g_object_new(LASSO_TYPE_SAML_SUBJECT_LOCALITY, NULL);
}

