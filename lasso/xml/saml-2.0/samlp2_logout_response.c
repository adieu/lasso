/* $Id$ 
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

#include "samlp2_logout_response.h"

/*
 * Schema fragment (saml-schema-protocol-2.0.xsd):
 *
 * <element name="LogoutResponse" type="samlp:StatusResponseType"/>
 */

/*****************************************************************************/
/* private methods                                                           */
/*****************************************************************************/


static struct XmlSnippet schema_snippets[] = {
	{NULL, 0, 0}
};

static LassoNodeClass *parent_class = NULL;


static gchar*
build_query(LassoNode *node)
{
	char *ret, *deflated_message;

	deflated_message = lasso_node_build_deflated_query(node);
	ret = g_strdup_printf("SAMLResponse=%s", deflated_message);
	/* XXX: must support RelayState (which profiles?) */
	g_free(deflated_message);
	return ret;
}


static gboolean
init_from_query(LassoNode *node, char **query_fields)
{
	gboolean rc;
	char *relay_state = NULL;
	rc = lasso_node_init_from_saml2_query_fields(node, query_fields, &relay_state);
	if (rc && relay_state != NULL) {
		/* XXX: support RelayState? */
	}
	return rc;
}


/*****************************************************************************/
/* instance and class init functions                                         */
/*****************************************************************************/

static void
instance_init(LassoSamlp2LogoutResponse *node)
{
}

static void
class_init(LassoSamlp2LogoutResponseClass *klass)
{
	LassoNodeClass *nclass = LASSO_NODE_CLASS(klass);

	parent_class = g_type_class_peek_parent(klass);
	nclass->build_query = build_query;
	nclass->init_from_query = init_from_query;
	nclass->node_data = g_new0(LassoNodeClassData, 1);
	lasso_node_class_set_nodename(nclass, "LogoutResponse"); 
	lasso_node_class_set_ns(nclass, LASSO_SAML2_PROTOCOL_HREF, LASSO_SAML2_PROTOCOL_PREFIX);
	lasso_node_class_add_snippets(nclass, schema_snippets);
}

GType
lasso_samlp2_logout_response_get_type()
{
	static GType this_type = 0;

	if (!this_type) {
		static const GTypeInfo this_info = {
			sizeof (LassoSamlp2LogoutResponseClass),
			NULL,
			NULL,
			(GClassInitFunc) class_init,
			NULL,
			NULL,
			sizeof(LassoSamlp2LogoutResponse),
			0,
			(GInstanceInitFunc) instance_init,
		};

		this_type = g_type_register_static(LASSO_TYPE_SAMLP2_STATUS_RESPONSE,
				"LassoSamlp2LogoutResponse", &this_info, 0);
	}
	return this_type;
}

/**
 * lasso_samlp2_logout_response_new:
 *
 * Creates a new #LassoSamlp2LogoutResponse object.
 *
 * Return value: a newly created #LassoSamlp2LogoutResponse object
 **/
LassoNode*
lasso_samlp2_logout_response_new()
{
	return g_object_new(LASSO_TYPE_SAMLP2_LOGOUT_RESPONSE, NULL);
}