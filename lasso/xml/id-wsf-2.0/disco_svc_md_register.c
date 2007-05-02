/* $Id: disco_svc_md_register.c 2261 2005-01-27 23:41:05 $ 
 *
 * Lasso - A free implementation of the Liberty Alliance specifications.
 *
 * Copyright (C) 2004, 2005 Entr'ouvert
 * http://lasso.entrouvert.org
 * 
 * Authors: See AUTHORS file in top-level directory.
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

#include <lasso/xml/id-wsf-2.0/disco_svc_md_register.h>
#include <lasso/xml/id-wsf-2.0/disco_svc_metadata.h>


/*
 * Schema fragment (liberty-idwsf-disco-svc-v2.0.xsd):
 * 
 * <xs:element name="SvcMDRegister" type="SvcMDRegisterType"/>
 * <xs:complexType name="SvcMDRegisterType">
 *    <xs:sequence>
 *       <xs:element ref="SvcMD" maxOccurs="unbounded"/>
 *    </xs:sequence>
 *    <xs:anyAttribute namespace="##other" processContents="lax"/>
 * </xs:complexType>
 */

/*****************************************************************************/
/* private methods                                                           */
/*****************************************************************************/

static struct XmlSnippet schema_snippets[] = {
	{ "SvcMD", SNIPPET_LIST_NODES,
	  G_STRUCT_OFFSET(LassoIdWsf2DiscoSvcMDRegister, metadata_list), "LassoIdWsf2DiscoSvcMetadata" },
	{ NULL, 0, 0}
};

/*****************************************************************************/
/* instance and class init functions                                         */
/*****************************************************************************/

static void
instance_init(LassoIdWsf2DiscoSvcMDRegister *node)
{
	node->metadata_list = NULL;
}

static void
class_init(LassoIdWsf2DiscoSvcMDRegisterClass *klass)
{
	LassoNodeClass *nclass = LASSO_NODE_CLASS(klass);

	nclass->node_data = g_new0(LassoNodeClassData, 1);
	lasso_node_class_set_nodename(nclass, "SvcMDRegister");
	lasso_node_class_set_ns(nclass, LASSO_IDWSF2_DISCO_HREF, LASSO_IDWSF2_DISCO_PREFIX);
	lasso_node_class_add_snippets(nclass, schema_snippets);
}

GType
lasso_idwsf2_disco_svc_md_register_get_type()
{
	static GType this_type = 0;

	if (!this_type) {
		static const GTypeInfo this_info = {
			sizeof (LassoIdWsf2DiscoSvcMDRegisterClass),
			NULL,
			NULL,
			(GClassInitFunc) class_init,
			NULL,
			NULL,
			sizeof(LassoIdWsf2DiscoSvcMDRegister),
			0,
			(GInstanceInitFunc) instance_init,
		};

		this_type = g_type_register_static(LASSO_TYPE_NODE,
				"LassoIdWsf2DiscoSvcMDRegister", &this_info, 0);
	}
	return this_type;
}

LassoIdWsf2DiscoSvcMDRegister*
lasso_idwsf2_disco_svc_md_register_new(const gchar *service_type, const gchar *abstract,
		const gchar *provider_id, const gchar *soap_endpoint)
{
	LassoIdWsf2DiscoSvcMDRegister *metadata_register;
	LassoIdWsf2DiscoSvcMetadata *metadata;

	metadata_register = g_object_new(LASSO_TYPE_IDWSF2_DISCO_SVC_MD_REGISTER, NULL);
	metadata = lasso_idwsf2_disco_svc_metadata_new_full(service_type, abstract, provider_id,
		soap_endpoint);
	metadata_register->metadata_list = g_list_append(
			metadata_register->metadata_list, metadata);

	return metadata_register;
}