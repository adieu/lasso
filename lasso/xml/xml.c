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

#include <lasso/xml/xml.h>
#include "../export.h"

struct _LassoNodePrivate
{
  gboolean   dispose_has_run;
  xmlNodePtr node;
};

/*****************************************************************************/
/* virtual public methods                                                    */
/*****************************************************************************/

GString *
lasso_node_build_query(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->build_query(node));
}

LassoNode *
lasso_node_copy(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->copy(node));
}

xmlChar *
lasso_node_dump(LassoNode     *node,
		const xmlChar *encoding,
		int            format)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->dump(node, encoding, format));
}

LassoAttr *
lasso_node_get_attr(LassoNode     *node,
		    const xmlChar *name)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_attr(node, name));
}

xmlChar *
lasso_node_get_attr_value(LassoNode     *node,
			  const xmlChar *name)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_attr_value(node, name));
}

GPtrArray *
lasso_node_get_attrs(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_attrs(node));
}

LassoNode *
lasso_node_get_child(LassoNode     *node,
		     const xmlChar *name)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_child(node, name));
}

GPtrArray *
lasso_node_get_children(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_children(node));
}

/**
 * lasso_node_get_content:
 * @node: the LassoNode
 * 
 * Read the value of a node, this can be either the text carried directly by
 * this node if it's a TEXT node or the aggregate string of the values carried
 * by this node child's (TEXT and ENTITY_REF). Entity references are
 * substituted.
 * 
 * Return value: a new xmlChar * or NULL if no content is available.
 * It's up to the caller to free the memory with xmlFree().
 **/
xmlChar *
lasso_node_get_content(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_content(node));
}

const xmlChar *
lasso_node_get_name(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_name(node));
}

void
lasso_node_load_from_buffer(LassoNode  *node,
			    const char *buffer)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->load_from_buffer(node, buffer);
}

void
lasso_node_rename_prop(LassoNode     *node,
		       const xmlChar *old_name,
		       const xmlChar *new_name)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->rename_prop(node, old_name, new_name);
}

GData *
lasso_node_serialize(LassoNode *node,
		     GData     *gd)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->serialize(node, gd));
}

gchar *
lasso_node_soap_envelop(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->soap_envelop(node));
}

gchar *
lasso_node_url_encode(LassoNode   *node,
		      guint        sign_method,
		      const gchar *private_key_file)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->url_encode(node, sign_method, private_key_file));
}

gint
lasso_node_verify_signature(LassoNode   *node,
			    const gchar *certificate_file)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), -1);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->verify_signature(node, certificate_file));
}

/*****************************************************************************/
/* virtual private methods                                                   */
/*****************************************************************************/

static void
lasso_node_add_child(LassoNode *node,
		     LassoNode *child,
		     gboolean   unbounded)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->add_child(node, child, unbounded);
}

static xmlNodePtr
lasso_node_get_xmlNode(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  return (class->get_xmlNode(node));
}

/**
 * lasso_node_new_child:
 * @node: the LassoNode
 * @name: the name of the child
 * @content: the content of the child
 * @unbounded: if TRUE, several children with the same name can be added else
 * the child must be unique.
 * 
 * Add a new child in node.
 * This is an internal function and should not be called by application
 * directly.
 **/
static void
lasso_node_new_child(LassoNode     *node,
		     const xmlChar *name,
		     const xmlChar *content,
		     gboolean       unbounded)
{
  g_return_if_fail(LASSO_IS_NODE(node)); 

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->new_child(node, name, content, unbounded);
}

static void
lasso_node_new_ns(LassoNode     *node,
		  const xmlChar *href,
		  const xmlChar *prefix)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->new_ns(node, href, prefix);
}

static void
lasso_node_set_name(LassoNode     *node,
		    const xmlChar *name)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->set_name(node, name);
}

static void
lasso_node_set_ns(LassoNode     *node,
		  const xmlChar *href,
		  const xmlChar *prefix)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->set_ns(node, href, prefix);
}

static void
lasso_node_set_prop(LassoNode     *node,
		    const xmlChar *name,
		    const xmlChar *value)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->set_prop(node, name, value);
}

static void
lasso_node_set_xmlNode(LassoNode *node,
		       xmlNodePtr libxml_node)
{
  g_return_if_fail(LASSO_IS_NODE(node));

  LassoNodeClass *class = LASSO_NODE_GET_CLASS(node);
  class->set_xmlNode(node, libxml_node);
}

/*****************************************************************************/
/* implementation methods                                                    */
/*****************************************************************************/

static void gdata_build_query_foreach_func(GQuark   key_id,
					   gpointer data,
					   gpointer user_data) {
  guint i;
  GString *str;
  GPtrArray *array;

  array = g_ptr_array_new();
  str = g_string_new("");
  for (i=0; i<((GPtrArray *)data)->len; i++) {
    str = g_string_append(str, g_ptr_array_index((GPtrArray *)data, i));
    if (i<((GPtrArray *)data)->len - 1) {
      str = g_string_append(str, " ");
    }
  }
  g_ptr_array_add(array, g_strdup((gpointer)g_quark_to_string(key_id)));
  g_ptr_array_add(array, str->str);
  g_string_free(str, FALSE);
  g_ptr_array_add((GPtrArray *)user_data, array);
}

static GString *
lasso_node_impl_build_query(LassoNode *node)
{
  guint i, j;
  GData *gd;
  GPtrArray *a, *aa;
  GString *query;
  xmlChar *str_escaped;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  gd = lasso_node_serialize(node, NULL);
  a = g_ptr_array_new();
  /* transform dict into array
     each key => [val1, val2, ...] of dict become [key, "val1 val2 ..."] */
  g_datalist_foreach(&gd, gdata_build_query_foreach_func, a);
  
  query = g_string_new("");
  for (i=0; i<a->len; i++) {
    aa = g_ptr_array_index(a, i);
    query = g_string_append(query, g_ptr_array_index(aa, 0));
    query = g_string_append(query, "=");
    str_escaped = lasso_str_escape(g_ptr_array_index(aa, 1));
    query = g_string_append(query, str_escaped);
    xmlFree(str_escaped);
    if (i<a->len - 1) {
      query = g_string_append(query, "&");
    }
    /* free allocated memory for array aa */
    for (j=0; j<aa->len; j++) {
      g_free(aa->pdata[j]);
    }
    g_ptr_array_free(aa, TRUE);
  }
  /* free allocated memory for array a */
  g_ptr_array_free(a, TRUE);
  g_datalist_clear(&gd);

  return (query);
}

static LassoNode *
lasso_node_impl_copy(LassoNode *node)
{
  return (lasso_node_new_from_xmlNode(xmlCopyNode(lasso_node_get_xmlNode(node), 1)));
}

static xmlChar *
lasso_node_impl_dump(LassoNode     *node,
		     const xmlChar *encoding,
		     int            format)
{
  xmlChar *ret;
  int len;
  xmlOutputBufferPtr buf;
  xmlCharEncodingHandlerPtr handler = NULL;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);
  /* encoding is optional */
  g_return_val_if_fail (format == 0 || format == 1, NULL);

  if (encoding != NULL) {
    handler = xmlFindCharEncodingHandler(encoding);
    if (handler == NULL) {
      return (NULL);
    }
  }
  buf = xmlAllocOutputBuffer(handler);
  if (buf == NULL) {
    return (NULL);
  }
  xmlNodeDumpOutput(buf, node->private->node->doc, node->private->node,
		    0, format, encoding);
  xmlOutputBufferFlush(buf);
  if (buf->conv != NULL) {
    len = buf->conv->use;
    ret = buf->conv->content;
    buf->conv->content = NULL;
  }
  else {
    len = buf->buffer->use;
    ret = buf->buffer->content;
    buf->buffer->content = NULL;
  }
  (void) xmlOutputBufferClose(buf);

  return (ret);
}

static LassoAttr*
lasso_node_impl_get_attr(LassoNode     *node,
			 const xmlChar *name)
{
  LassoAttr *prop;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  prop = node->private->node->properties;
  while (prop != NULL) {
    if (xmlStrEqual(prop->name, name)) {
      return (prop);
    }
    prop = prop->next;
  }

  return (NULL);
}

static xmlChar *
lasso_node_impl_get_attr_value(LassoNode     *node,
			       const xmlChar *name)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);
  g_return_val_if_fail (name != NULL, NULL);

  return (lasso_node_get_attr(node, name)->children->content);
}

static GPtrArray *
lasso_node_impl_get_attrs(LassoNode *node)
{
  GPtrArray *attributs;
  LassoAttr *prop;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  attributs = g_ptr_array_new();
  prop = node->private->node->properties;
  while (prop != NULL) {
    g_ptr_array_add(attributs, prop);
    prop = prop->next;
  }

  return (attributs);
}

static LassoNode *
lasso_node_impl_get_child(LassoNode     *node,
			  const xmlChar *name)
{
  /*   /\* No recurssive version *\/ */
  /*   xmlNodePtr cur; */
  
  /*   cur = node->private->node->children; */
  /*   while (cur != NULL) { */
  /*     if(cur->type == XML_ELEMENT_NODE) { */
  /*       if (xmlStrEqual(cur->name, name)) { */
  /*   	return (lasso_node_new_from_xmlNode(cur)); */
  /*       } */
  /*     } */
  /*     cur = cur->next; */
  /*   } */
  /*   return (NULL); */

  /* Recurssive version */
  xmlNodePtr cur;
  LassoNode *ret;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);
  g_return_val_if_fail (name != NULL, NULL);
        
  cur = node->private->node;
  while (cur != NULL) {
    if ((cur->type == XML_ELEMENT_NODE) && xmlStrEqual(cur->name, name)) {
      return (lasso_node_new_from_xmlNode(cur));
    }
    if (cur->children != NULL) {
      ret = lasso_node_get_child(lasso_node_new_from_xmlNode(cur->children),
				 name);
      if (ret != NULL) {
	return (ret);
      }
    }
    cur = cur->next;
  }
  return (NULL);
}

static GPtrArray *
lasso_node_impl_get_children(LassoNode *node)
{
  GPtrArray *children;
  xmlNodePtr cur;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  cur = node->private->node->children;
  if (cur != NULL)
    children = g_ptr_array_new();
  
  while (cur != NULL) {
    g_ptr_array_add(children, lasso_node_new_from_xmlNode(cur));
    cur = cur->next;
  }

  return (children);
}

static xmlChar *
lasso_node_impl_get_content(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  return (xmlNodeGetContent(node->private->node));
}

static const xmlChar *
lasso_node_impl_get_name(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  return (node->private->node->name);
}

static void
lasso_node_impl_load_from_buffer(LassoNode  *node,
				 const char *buffer)
{
  xmlDocPtr doc;
  xmlNodePtr root;

  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (buffer != NULL);

  doc = xmlParseMemory(buffer, strlen(buffer));
  /* get root element of doc and duplicate it */
  root = xmlCopyNode(xmlDocGetRootElement(doc), 1);
  lasso_node_set_xmlNode(node, root);
  /* free doc */
  xmlFreeDoc(doc);
}

static void
lasso_node_impl_rename_prop(LassoNode     *node,
			    const xmlChar *old_name,
			    const xmlChar *new_name)
{
  xmlChar *value;

  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (old_name != NULL);
  g_return_if_fail (new_name != NULL);

  value = xmlGetProp(node->private->node, old_name);
  if (value != NULL) {
    xmlRemoveProp(lasso_node_get_attr(node, old_name));
    lasso_node_set_prop(node, new_name, value);
  }
}

static void
gdata_serialize_destroy_notify(gpointer data)
{
  gint i;
  GPtrArray *array = data;

  for (i=0; i<array->len; i++) {
    xmlFree(array->pdata[i]);
  }
  g_ptr_array_free(array, TRUE);
}

static GData *
lasso_node_impl_serialize(LassoNode *node,
			  GData     *gd)
{
  GPtrArray *attrs, *children;
  GPtrArray *values;
  const xmlChar *name;
  xmlChar *val;
  int i;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  if (gd == NULL) {
    g_datalist_init(&gd);
  }

  attrs = lasso_node_get_attrs(node);
  for(i=0; i<attrs->len; i++) {
    values = g_ptr_array_new();
    name = (xmlChar *)((LassoAttr *)g_ptr_array_index(attrs, i))->name;
    /* xmlGetProp returns a COPY of attr value
       each val must be xmlFree in gdata_serialize_destroy_notify()
       which is called by g_datalist_clear() */
    val = xmlGetProp(node->private->node, name);
    g_ptr_array_add(values, val);
    g_datalist_set_data_full(&gd, name, values, gdata_serialize_destroy_notify);
  }
  g_ptr_array_free(attrs, TRUE);

  children = lasso_node_get_children(node);
  if (children != NULL) {
    for(i=0; i<children->len; i++) {
      xmlNodePtr xml_node = ((LassoNode *)g_ptr_array_index(children, i))->private->node;
      switch (xml_node->type) {
      case XML_ELEMENT_NODE:
	gd = lasso_node_serialize(g_ptr_array_index(children, i), gd);
	break;
      case XML_TEXT_NODE:
	name   = lasso_node_get_name(node);
	/* xmlNodeGetContent returns a COPY of node content
	   each val must be xmlFree in gdata_serialize_destroy_notify()
	   which is called by g_datalist_clear() */
	val    = xmlNodeGetContent(node->private->node);
	values = (GPtrArray *)g_datalist_get_data(&gd, name);
	if (values == NULL) {
	  values = g_ptr_array_new();
	  g_ptr_array_add(values, val);
	  g_datalist_set_data_full(&gd, name, values,
				   gdata_serialize_destroy_notify);
	}
	else {
	  g_ptr_array_add(values, val);
	}
	break;
      }
    }
  }
  g_ptr_array_free(children, TRUE);
    
  return (gd);
}

static gchar *
lasso_node_impl_soap_envelop(LassoNode *node)
{
  LassoNode *envelope, *body;
  xmlChar *envelope_dump;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);
  
  envelope = lasso_node_new();
  lasso_node_set_name(envelope, "Envelope");
  lasso_node_set_ns(envelope, lassoSoapEnvHRef, lassoSoapEnvPrefix);
  
  body = lasso_node_new();
  lasso_node_set_name(body, "Body");
  lasso_node_set_ns(body, lassoSoapEnvHRef, lassoSoapEnvPrefix);
  
  lasso_node_add_child(body, lasso_node_copy(node), 0);
  lasso_node_add_child(envelope, body, 0);

  envelope_dump = lasso_node_dump(envelope, "utf-8", 1);
  g_object_unref(envelope);
  
  return(envelope_dump);
}

static gchar *
lasso_node_impl_url_encode(LassoNode   *node,
			   guint        sign_method,
			   const gchar *private_key_file)
{
  GString *msg;
  xmlDocPtr doc;
  xmlChar *str1, *str2, *str_escaped;
  gchar *ret;

  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);
  g_return_val_if_fail (private_key_file != NULL, NULL);

  msg = lasso_node_build_query(node);

  if (sign_method > 0 && private_key_file != NULL) {
    msg = g_string_append(msg, "&SigAlg=");
    switch (sign_method) {
    case lassoUrlEncodeRsaSha1:
      str_escaped = lasso_str_escape(xmlSecHrefRsaSha1);
      msg = g_string_append(msg, str_escaped);
      doc = lasso_str_sign(msg->str, xmlSecTransformRsaSha1Id, private_key_file);
      break;
    case lassoUrlEncodeDsaSha1:
      str_escaped = lasso_str_escape(xmlSecHrefDsaSha1);
      msg = g_string_append(msg, str_escaped);
      doc = lasso_str_sign(msg->str, xmlSecTransformDsaSha1Id, private_key_file);
      break;
    }
    xmlFree(str_escaped);
    msg = g_string_append(msg, "&Signature=");
    str1 = lasso_doc_get_node_content(doc, xmlSecNodeSignatureValue);
    str2 = lasso_str_escape(str1);
    xmlFree(str1);
    msg = g_string_append(msg, str2);
    xmlFree(str2);
    xmlFreeDoc(doc);
  }

  ret = g_strdup(msg->str);
  g_string_free(msg, TRUE);
  return (ret);
}

static gint
lasso_node_impl_verify_signature(LassoNode   *node,
				 const gchar *certificate_file)
{
  xmlNodePtr signature;
  xmlSecKeysMngrPtr mngr;
  xmlSecDSigCtxPtr dsigCtx;
  gint ret = -1;

  g_return_val_if_fail (LASSO_IS_NODE(node), -1);
  g_return_val_if_fail (certificate_file != NULL, -1);

  /* find start node */
  signature = xmlSecFindNode(node->private->node, xmlSecNodeSignature,
			     xmlSecDSigNs);
  if (signature == NULL) {
    fprintf(stderr, "Error: start node not found\n");
    goto done;	
  }

  /* create simple keys mngr */
  mngr = xmlSecKeysMngrCreate();
  if (mngr == NULL) {
    fprintf(stderr, "Error: failed to create keys manager.\n");
    goto done;
  }

  if (xmlSecCryptoAppDefaultKeysMngrInit(mngr) < 0) {
    fprintf(stderr, "Error: failed to initialize keys manager.\n");
    goto done;
  }
  
  /* load trusted cert */
  if (xmlSecCryptoAppKeysMngrCertLoad(mngr, certificate_file,
				      xmlSecKeyDataFormatPem,
				      xmlSecKeyDataTypeTrusted) < 0) {
    fprintf(stderr, "Error: failed to load pem certificate from \"%s\"\n",
	    certificate_file);
    goto done;
  }

  /* create signature context */
  dsigCtx = xmlSecDSigCtxCreate(mngr);
  if (dsigCtx == NULL) {
    fprintf(stderr, "Error: failed to create signature context\n");
    goto done;
  }

  /* verify signature */
  if (xmlSecDSigCtxVerify(dsigCtx, signature) < 0) {
    fprintf(stderr, "Error: signature verify\n");
    goto done;
  }

  /* print verification result to stdout */
  if (dsigCtx->status == xmlSecDSigStatusSucceeded) {
    fprintf(stdout, "Signature is OK\n");
    ret = 1;
  }
  else {
    fprintf(stdout, "Signature is INVALID\n");
    ret = 0;
  }

 done:
  /* cleanup */
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }
  if(mngr != NULL) {
    xmlSecKeysMngrDestroy(mngr);
  }
  return (ret);
}

/*** private methods **********************************************************/

static void
lasso_node_impl_add_child(LassoNode *node,
			  LassoNode *child,
			  gboolean   unbounded)
{
  LassoNode *old_child;

  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (LASSO_IS_NODE(child));

  // if child is not unbounded, we search it
  if (!unbounded) {
    old_child = lasso_node_get_child(node, child->private->node->name);
  }

  if (!unbounded && old_child != NULL) {
    // child replace old child
    xmlReplaceNode(old_child->private->node, child->private->node);
  }
  else {
    // else child is added
    xmlAddChild(node->private->node, child->private->node);
  }
}

static xmlNodePtr
lasso_node_impl_get_xmlNode(LassoNode *node)
{
  g_return_val_if_fail (LASSO_IS_NODE(node), NULL);

  return (node->private->node);
}

static void
lasso_node_impl_new_child(LassoNode     *node,
			  const xmlChar *name,
			  const xmlChar *content,
			  gboolean       unbounded)
{
  LassoNode *old_child = NULL;

  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (name != NULL);
  g_return_if_fail (content != NULL);
  
  if (!unbounded) {
    old_child = lasso_node_get_child(node, name);
  }

  if (!unbounded && old_child != NULL) {
    xmlNodeSetContent(old_child->private->node, content);
  }
  else {
    xmlNewChild(node->private->node, NULL, name, content);
  }
}

static void
lasso_node_impl_new_ns(LassoNode     *node,
		       const xmlChar *href,
		       const xmlChar *prefix)
{
  g_return_if_fail (LASSO_IS_NODE(node));
  /* href may be NULL */
  g_return_if_fail (prefix != NULL);

  xmlSetNs(node->private->node,
	   xmlNewNs(node->private->node, href, prefix));
}

static void
lasso_node_impl_set_name(LassoNode     *node,
			 const xmlChar *name)
{
  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (name != NULL);

  xmlNodeSetName(node->private->node, name);
}

static void
lasso_node_impl_set_ns(LassoNode     *node,
		       const xmlChar *href,
		       const xmlChar *prefix)
{
  xmlNsPtr new_ns;

  g_return_if_fail (LASSO_IS_NODE(node));
  /* href may be NULL */
  g_return_if_fail (prefix != NULL);

  /*   xmlNsPtr cur; */
  /*   cur = node->private->node->ns; */
  /*   while (cur != NULL) { */
  /*     printf("%s:%s\n", cur->prefix, cur->href); */
  /*     cur = cur->next; */
  /*   } */
  /*   cur = node->private->node->nsDef; */
  /*   while (cur != NULL) { */
  /*     printf("%s:%s\n", cur->prefix, cur->href); */
  /*     cur = cur->next; */
  /*   } */

  new_ns = xmlNewNs(node->private->node, href, prefix);
  xmlFreeNs(node->private->node->ns);
  xmlSetNs(node->private->node, new_ns);
  node->private->node->nsDef = new_ns;
}

static void
lasso_node_impl_set_prop(LassoNode     *node,
			 const xmlChar *name,
			 const xmlChar *value)
{
  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (name != NULL);
  g_return_if_fail (value != NULL);

  xmlSetProp(node->private->node, name, value);
}

static void
lasso_node_impl_set_xmlNode(LassoNode  *node,
			    xmlNodePtr  libxml_node)
{
  g_return_if_fail (LASSO_IS_NODE(node));
  g_return_if_fail (libxml_node != NULL);

  xmlFreeNode(node->private->node);
  node->private->node = libxml_node;
}

/*****************************************************************************/
/* overrided parent class methods                                            */
/*****************************************************************************/

static void
lasso_node_dispose(LassoNode *node)
{
  if (node->private->dispose_has_run) {
    return;
  }
  node->private->dispose_has_run = TRUE;

  /* unref reference counted objects */
  /* we don't have any here */
  g_print("%s 0x%x disposed ...\n", lasso_node_get_name(node), node);
}

static void
lasso_node_finalize(LassoNode *node)
{
  g_print("%s 0x%x finalized ...\n", lasso_node_get_name(node), node);
  xmlFreeNode(node->private->node);
  g_free (node->private);
}

/*****************************************************************************/
/* instance and class init functions                                         */
/*****************************************************************************/

static void
lasso_node_instance_init(LassoNode *instance)
{
  LassoNode *node = LASSO_NODE(instance);

  node->private = g_new (LassoNodePrivate, 1);
  node->private->dispose_has_run = FALSE;
  node->private->node = xmlNewNode(NULL, "no-name-set");
}

static void
lasso_node_class_init(LassoNodeClass *class)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  
  /* virtual public methods */
  class->build_query      = lasso_node_impl_build_query;
  class->copy             = lasso_node_impl_copy;
  class->dump             = lasso_node_impl_dump;
  class->get_attr         = lasso_node_impl_get_attr;
  class->get_attr_value   = lasso_node_impl_get_attr_value;
  class->get_attrs        = lasso_node_impl_get_attrs;
  class->get_child        = lasso_node_impl_get_child;
  class->get_children     = lasso_node_impl_get_children;
  class->get_content      = lasso_node_impl_get_content;
  class->get_name         = lasso_node_impl_get_name;
  class->load_from_buffer = lasso_node_impl_load_from_buffer;
  class->rename_prop      = lasso_node_impl_rename_prop;
  class->serialize        = lasso_node_impl_serialize;
  class->soap_envelop     = lasso_node_impl_soap_envelop;
  class->url_encode       = lasso_node_impl_url_encode;
  class->verify_signature = lasso_node_impl_verify_signature;
  /* virtual private methods */
  class->add_child   = lasso_node_impl_add_child;
  class->get_xmlNode = lasso_node_impl_get_xmlNode;
  class->new_child   = lasso_node_impl_new_child;
  class->new_ns      = lasso_node_impl_new_ns;
  class->set_name    = lasso_node_impl_set_name;
  class->set_ns      = lasso_node_impl_set_ns;
  class->set_prop    = lasso_node_impl_set_prop;
  class->set_xmlNode = lasso_node_impl_set_xmlNode;
  /* override parent class methods */
  gobject_class->dispose  = (void *)lasso_node_dispose;
  gobject_class->finalize = (void *)lasso_node_finalize;
}

GType lasso_node_get_type() {
  static GType this_type = 0;

  if (!this_type) {
    static const GTypeInfo this_info = {
      sizeof (LassoNodeClass),
      NULL,
      NULL,
      (GClassInitFunc) lasso_node_class_init,
      NULL,
      NULL,
      sizeof(LassoNode),
      0,
      (GInstanceInitFunc) lasso_node_instance_init,
    };
    
    this_type = g_type_register_static(G_TYPE_OBJECT , "LassoNode",
				       &this_info, 0);
  }
  return this_type;
}

LassoNode*
lasso_node_new()
{
  return (LASSO_NODE(g_object_new(LASSO_TYPE_NODE, NULL)));
}

LassoNode*
lasso_node_new_from_dump(xmlChar *buffer)
{
  LassoNode *node;
  xmlDocPtr  doc;
  xmlNodePtr root;

  g_return_val_if_fail (buffer != NULL, NULL);

  node = lasso_node_new();
  doc = xmlParseMemory(buffer, strlen(buffer));
  /* get root element of doc and duplicate it */
  root = xmlCopyNode(xmlDocGetRootElement(doc), 1);
  lasso_node_set_xmlNode(node, root);
  /* free doc */
  xmlFreeDoc(doc);

  return (node);
}

LassoNode*
lasso_node_new_from_xmlNode(xmlNodePtr node)
{
  LassoNode *lasso_node;

  g_return_val_if_fail (node != NULL, NULL);

  lasso_node = lasso_node_new();
  lasso_node_set_xmlNode(lasso_node, node);

  return (lasso_node);
}
