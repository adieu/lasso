/* $Id$
 *
 * Lasso - A free implementation of the Liberty Alliance specifications.
 *
 * Copyright (C) 2004 Entr'ouvert
 * http://lasso.entrouvert.org
 * 
 * Author: Valery Febvre <vfebvre@easter-eggs.com>
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

#include <lasso/protocols/provider.h>

/*****************************************************************************/
/* public methods                                                            */
/*****************************************************************************/

gchar *
lasso_provider_dump(LassoProvider *provider)
{
  LassoNode *provider_node;
  LassoNodeClass *provider_class;

  provider_node = lasso_node_new();

  /* set the public key, certificate, metadata */
  provider_class = LASSO_NODE_GET_CLASS(provider_node);
  provider_class->set_name(provider_node, LASSO_PROVIDER_NODE);
  provider_class->add_child(provider_node, provider->metadata, FALSE);
  if(provider->public_key)
    provider_class->set_prop(provider_node, LASSO_PROVIDER_PUBLIC_KEY_NODE, provider->public_key);
  if(provider->certificate)
    provider_class->set_prop(provider_node, LASSO_PROVIDER_CERTIFICATE_NODE, provider->certificate);

  return(lasso_node_export(provider_node));
}

gchar *
lasso_provider_get_assertionConsumerServiceURL(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "AssertionConsumerServiceURL", NULL));
}

gchar *
lasso_provider_get_federationTerminationNotificationProtocolProfile(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "FederationTerminationNotificationProtocolProfile", NULL));
}

gchar *
lasso_provider_get_federationTerminationNotificationServiceURL(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "FederationTerminationNotificationServiceURL", NULL));
}

gchar *
lasso_provider_get_providerID(LassoProvider *provider)
{
  return(lasso_node_get_attr_value(provider->metadata, "ProviderID"));
}

gchar *
lasso_provider_get_registerNameIdentifierProtocolProfile(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "RegisterNameIdentifierProtocolProfile", NULL));
}

gchar *
lasso_provider_get_registerNameIdentifierServiceURL(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "RegisterNameIdentifierServiceURL", NULL));
}

gchar *
lasso_provider_get_singleSignOnProtocolProfile(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "SingleSignOnProtocolProfile", NULL));
}

gchar *
lasso_provider_get_singleSignOnServiceURL(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "SingleSignOnServiceURL", NULL));
}

gchar *lasso_provider_get_singleLogoutProtocolProfile(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "SingleLogoutProtocolProfile", NULL));
}

gchar *lasso_provider_get_singleLogoutServiceURL(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "SingleLogoutServiceURL", NULL));
}

gchar *lasso_provider_get_singleLogoutServiceReturnURL(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "SingleLogoutServiceReturnURL", NULL));
}

gchar *
lasso_provider_get_soapEndpoint(LassoProvider *provider)
{
  return(lasso_node_get_child_content(provider->metadata, "SoapEndpoint", NULL));
}

void
lasso_provider_set_public_key(LassoProvider *provider, gchar *public_key)
{
  provider->public_key = g_strdup(public_key);
}

void
lasso_provider_set_certificate(LassoProvider *provider, gchar *certificate)
{
  provider->certificate = g_strdup(certificate);
}

/*****************************************************************************/
/* private methods                                                           */
/*****************************************************************************/

static gchar *lasso_provider_get_direct_child_content(LassoProvider *provider, 
							const gchar *name)
{
  LassoNode *node;
  
  node = lasso_node_get_child(LASSO_NODE(provider), name, NULL);
  if(!node)
    return(NULL);
  return(lasso_node_get_content(node));
}


/*****************************************************************************/
/* instance and class init functions                                         */
/*****************************************************************************/

static void
lasso_provider_instance_init(LassoProvider *provider)
{

}

static void
lasso_provider_class_init(LassoProviderClass *klass) {
}

GType lasso_provider_get_type() {
  static GType this_type = 0;

  if (!this_type) {
    static const GTypeInfo this_info = {
      sizeof (LassoProviderClass),
      NULL,
      NULL,
      (GClassInitFunc) lasso_provider_class_init,
      NULL,
      NULL,
      sizeof(LassoProvider),
      0,
      (GInstanceInitFunc) lasso_provider_instance_init,
    };
    
    this_type = g_type_register_static(G_TYPE_OBJECT,
				       "LassoProvider",
				       &this_info, 0);
  }
  return this_type;
}

LassoProvider*
lasso_provider_new(gchar *metadata_filename,
		   gchar *public_key_filename,
		   gchar *certificate_filename)
{
  LassoProvider *provider;
  
  provider = lasso_provider_new_metadata_filename(metadata_filename);
  provider->public_key = public_key_filename;
  provider->certificate = certificate_filename;
  
  return(provider);
}


LassoProvider*
lasso_provider_new_metadata_xmlNode(xmlNodePtr metadata_xmlNode)
{
  LassoProvider *provider;
  
  provider = LASSO_PROVIDER(g_object_new(LASSO_TYPE_PROVIDER, NULL));
  provider->metadata = lasso_node_new_from_xmlNode(metadata_xmlNode);
  
  return(provider);
}

LassoProvider*
lasso_provider_new_metadata_filename(gchar *metadata_filename)
{
  LassoProvider *provider;
  xmlDocPtr  doc;
  xmlNodePtr root;
  
  provider = LASSO_PROVIDER(g_object_new(LASSO_TYPE_PROVIDER, NULL));
  
  /* get root element of doc and duplicate it */
  doc = xmlParseFile(metadata_filename);
  root = xmlCopyNode(xmlDocGetRootElement(doc), 1);
  xmlFreeDoc(doc);
  provider->metadata = lasso_node_new_from_xmlNode(root);

  return(provider);
}
