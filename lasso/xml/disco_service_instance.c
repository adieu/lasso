/* $Id$ 
 *
 * Lasso - A free implementation of the Liberty Alliance specifications.
 *
 * Copyright (C) 2004 Entr'ouvert
 * http://lasso.entrouvert.org
 * 
 * Authors: Valery Febvre   <vfebvre@easter-eggs.com>
 *          Nicolas Clapies <nclapies@entrouvert.com>
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

#include <lasso/xml/disco_service_instance.h>

/*
 * Schema fragment (liberty-idwsf-disco-svc-1.0-errata-v1.0.xsd):
 * 
 * <xs:complexType name="ServiceInstanceType">
 *   <xs:sequence>
 *     <xs:element ref="ServiceType"/>
 *     <xs:element name="ProviderID" type="md:entityIDType"/>
 *     <xs:element name="Description" type="DescriptionType" minOccurs="1" maxOccurs="unbounded"/>
 *   </xs:sequence>
 * </xs:complexType>
 * 
 * <xs:element name="ServiceType" type="xs:anyURI"/>
 */

/*****************************************************************************/
/* private methods                                                           */
/*****************************************************************************/

static struct XmlSnippet schema_snippets[] = {
	{ "ServiceType", SNIPPET_CONTENT,
	  G_STRUCT_OFFSET(LassoDiscoServiceInstance, ServiceType) },
	{ "ProviderID",  SNIPPET_CONTENT,
	  G_STRUCT_OFFSET(LassoDiscoServiceInstance, ProviderID) },
	{ "Description", SNIPPET_LIST_NODES,
	  G_STRUCT_OFFSET(LassoDiscoServiceInstance, Description) },
	{ NULL, 0, 0}
};

/*****************************************************************************/
/* instance and class init functions                                         */
/*****************************************************************************/

static void
instance_init(LassoDiscoServiceInstance *node)
{
	node->ServiceType = NULL;
	node->ProviderID = NULL;
	node->Description = NULL;
}

static void
class_init(LassoDiscoServiceInstanceClass *klass)
{
	LassoNodeClass *nclass = LASSO_NODE_CLASS(klass);

	nclass->node_data = g_new0(LassoNodeClassData, 1);
	lasso_node_class_set_nodename(nclass, "ServiceInstance");
	lasso_node_class_set_ns(nclass, LASSO_DISCO_HREF, LASSO_DISCO_PREFIX);
	lasso_node_class_add_snippets(nclass, schema_snippets);
}

GType
lasso_disco_service_instance_get_type()
{
	static GType this_type = 0;

	if (!this_type) {
		static const GTypeInfo this_info = {
			sizeof (LassoDiscoServiceInstanceClass),
			NULL,
			NULL,
			(GClassInitFunc) class_init,
			NULL,
			NULL,
			sizeof(LassoDiscoServiceInstance),
			0,
			(GInstanceInitFunc) instance_init,
		};

		this_type = g_type_register_static(LASSO_TYPE_NODE,
				"LassoDiscoServiceInstance", &this_info, 0);
	}
	return this_type;
}

LassoDiscoServiceInstance*
lasso_disco_service_instance_new(const gchar *serviceType,
				 const gchar *providerID,
				 GList       *descriptions)
{
	LassoDiscoServiceInstance *service_instance;

	g_return_val_if_fail (serviceType != NULL, NULL);
	g_return_val_if_fail (providerID != NULL, NULL);
	g_return_val_if_fail(g_list_length(descriptions) >= 1, NULL);

	service_instance = g_object_new(LASSO_TYPE_DISCO_SERVICE_INSTANCE, NULL);

	service_instance->ServiceType = g_strdup(serviceType);
	service_instance->ProviderID = g_strdup(providerID);

	/* FIXME: should Description be a copy ??*/
	service_instance->Description = descriptions;

	return service_instance;
}
