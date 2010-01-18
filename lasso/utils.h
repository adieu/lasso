/* $Id$
 *
 * Lasso - A free implementation of the Liberty Alliance specifications.
 *
 * Copyright (C) 2004-2007 Entr'ouvert
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

#ifndef __LASSO_UTILS_H__
#define __LASSO_UTILS_H__

#include <glib.h>
#include <glib-object.h>
#include "debug.h"
#include "./backward_comp.h"

#ifdef LASSO_DEBUG
#ifdef __GNUC__
#define lasso_check_type_equality(a,b) \
	{ \
		enum { TYPE_MISMATCH = (1 / __builtin_types_compatible_p(typeof(a), typeof(b))) }; \
	}
#else
#define lasso_check_type_equality(a,b)
#endif
#else
#define lasso_check_type_equality(a,b)
#endif

#ifdef __GNUC__
#define lasso_check_type_equality2(a,b,c) \
	{ \
		enum { TYPE_MISMATCH = (1 / (__builtin_types_compatible_p(typeof(a), typeof(b))+__builtin_types_compatible_p(typeof(a), typeof(c)))) }; \
	}
#else
#define lasso_check_type_equality2(a,b,c)
#endif

#define lasso_private_data(object) ((object)->private_data)

#define lasso_ref(object) ((object) != NULL ? g_object_ref(object) : NULL)

/* Freeing */
#define lasso_release(dest) \
	{ \
		if (dest) { \
			g_free(dest); dest = NULL; \
		} \
	}

#define lasso_release_full(dest, free_function) \
	{ \
		if (dest) { \
			free_function(dest); dest = NULL; \
		} \
	}

#define lasso_release_full2(dest, free_function, type) \
	{ \
		lasso_check_type_equality(dest, type); \
		if (dest) { \
			free_function(dest); dest = NULL; \
		} \
	}

#define lasso_release_gobject(dest) \
	{ \
		if (G_IS_OBJECT(dest) || dest == NULL) { \
			lasso_release_full(dest, g_object_unref); \
		} else { \
			g_critical("Trying to unref a non GObject pointer file=%s:%u pointerbybname=%s pointer=%p", __FILE__, __LINE__, #dest, dest); \
		} \
	}

#define lasso_release_string(dest) \
	lasso_release_full(dest, g_free)

#define lasso_release_list(dest) \
	lasso_release_full2(dest, g_list_free, GList*)

#define lasso_release_list_of_full(dest, free_function) \
	{ \
		if (dest) { \
			g_list_foreach(dest, (GFunc)free_function, NULL); \
			lasso_release_list(dest); \
		} \
	}

#define lasso_release_list_of_strings(dest) \
	lasso_release_list_of_full(dest, g_free)

#define lasso_release_list_of_gobjects(dest) \
	lasso_release_list_of_full(dest, g_object_unref)

#define lasso_release_list_of_xml_node(dest) \
	lasso_release_list_of_full(dest, xmlFreeNode)

#define lasso_release_xml_node(node) \
	lasso_release_full2(node, xmlFreeNode, xmlNodePtr)

#define lasso_release_doc(doc) \
	lasso_release_full2(doc, xmlFreeDoc, xmlDocPtr)

#define lasso_release_xml_string(dest) \
	lasso_release_full2(dest, xmlFree, xmlChar*)

#define lasso_release_encrypt_context(dest) \
	lasso_release_full2(dest, xmlSecEncCtxDestroy, xmlSecEncCtxPtr)

#define lasso_release_signature_context(dest) \
	lasso_release_full2(dest, xmlSecDSigCtxDestroy,xmlSecDSigCtxPtr)

#define lasso_release_key_manager(dest) \
	lasso_release_full2(dest, xmlSecKeysMngrDestroy, xmlSecKeysMngrPtr)

#define lasso_release_output_buffer(dest) \
	lasso_release_full2(dest, xmlOutputBufferClose, xmlOutputBufferPtr)

#define lasso_release_xpath_object(dest) \
	lasso_release_full2(dest, xmlXPathFreeObject, xmlXPathObjectPtr)

#define lasso_release_xpath_context(dest) \
	lasso_release_full2(dest, xmlXPathFreeContext, xmlXPathContextPtr)

#define lasso_release_xpath_job(xpathObject, xpathContext, xmlDocument) \
	lasso_release_xpath_object(xpathObject); \
	lasso_release_xpath_context(xpathContext); \
	lasso_release_doc(xmlDocument)

#define lasso_release_sec_key(dest) \
	lasso_release_full2(dest, xmlSecKeyDestroy, xmlSecKeyPtr)

/* Assignment and list appending */
#define lasso_assign_string(dest,src) \
	{ \
		char *__tmp = g_strdup(src);\
		lasso_release_string(dest); \
		dest = __tmp; \
	}

#define lasso_assign_xml_string(dest,src) \
	{ \
		xmlChar *__tmp = xmlStrdup(src); \
		lasso_release_xml_string(dest); \
		dest = __tmp; \
	}

#define lasso_assign_new_string(dest,src) \
	{ \
		char *__tmp = src; \
		if (dest != __tmp) \
			lasso_release_string(dest); \
		dest = __tmp; \
	}

#define lasso_assign_gobject(dest,src) \
	{ \
		GObject *__tmp = G_OBJECT(src); \
		if (__tmp) \
			g_object_ref(__tmp); \
		lasso_release_gobject(dest); \
		dest = (void*)(__tmp); \
	}

#define lasso_assign_new_gobject(dest,src) \
	{ \
		GObject *__tmp = G_OBJECT(src); \
		if (dest != (void*)__tmp) \
			lasso_release_gobject(dest); \
		dest = (void*)(__tmp); \
	}

#define lasso_assign_xml_node(dest,src) \
	{ \
		xmlNode *__tmp = (src); \
		lasso_check_type_equality(dest, src); \
		if (dest) \
			xmlFreeNode(dest); \
		dest = xmlCopyNode(__tmp, 1); \
	}

#define lasso_assign_new_list_of_gobjects(dest, src) \
	{ \
		GList *__tmp = (src); \
		lasso_release_gobject_list(dest); \
		dest = (GList*)__tmp; \
	}

#define lasso_assign_list_of_gobjects(dest, src) \
	{ \
		GList *__tmp = (src); \
		lasso_release_list_of_gobjects(dest); \
		dest = g_list_copy(__tmp); \
		for (;__tmp != NULL; __tmp = g_list_next(__tmp)) { \
			if (G_IS_OBJECT(__tmp->data)) { \
				g_object_ref(__tmp->data); \
			} \
		} \
	}

#define lasso_assign_list_of_strings(dest, src) \
	{ \
		GList *__tmp = src; \
		GList *__iter_dest; \
		lasso_release_list_of_strings(dest); \
		dest = g_list_copy(__tmp); \
		for (__iter_dest = dest ; __iter_dest != NULL ; __iter_dest = g_list_next(__iter_dest)) { \
			__iter_dest->data = g_strdup(__iter_dest->data); \
		} \
	}

#define lasso_assign_new_sec_key(dest, src) \
	{ \
		xmlSecKey *__tmp = (src); \
		if (dest) \
			lasso_release_sec_key(dest); \
		dest = __tmp; \
	}

#define lasso_assign_sec_key(dest, src) \
	{ \
		xmlSecKey *__tmp = xmlSecKeyDup(src); \
		if (dest) \
			lasso_release_sec_key(dest); \
		dest = __tmp; \
	}



/* List appending */
#define lasso_list_add(dest, src) \
	{ \
		lasso_check_type_equality((src), void*); \
		dest = g_list_append(dest, (src)); \
	}

#define lasso_list_add_non_null(dest, src) \
	{ \
		void *__tmp_non_null_src = (src); \
		if (__tmp_non_null_src != NULL) { \
			dest = g_list_append(dest, __tmp_non_null_src); \
		} else { \
			g_critical("Adding a NULL value to a non-NULL content list: dest=%s src=%s", #dest, #src); \
		} \
	}

#define lasso_list_add_string(dest, src) \
	{ \
		lasso_list_add_non_null(dest, g_strdup(src));\
	}

#define lasso_list_add_new_string(dest, src) \
	{ \
		gchar *__tmp = src; \
		lasso_list_add_non_null(dest, __tmp); \
	}

#define lasso_list_add_xml_string(dest, src) \
	{\
		xmlChar *__tmp_src = (src);\
		lasso_list_add_non_null(dest, (void*)g_strdup((char*)__tmp_src));\
	}

#define lasso_list_add_gobject(dest, src) \
	{ \
		void *__tmp_src = (src); \
		if (G_IS_OBJECT(__tmp_src)) { \
			dest = g_list_append(dest, g_object_ref(__tmp_src)); \
		} else { \
			g_critical("Trying to add to a GList* a non GObject pointer dest=%s src=%s", #dest, #src); \
		} \
	}

#define lasso_list_add_new_gobject(dest, src) \
	{ \
		void *__tmp_src = (src); \
		if (G_IS_OBJECT(__tmp_src)) { \
			dest = g_list_append(dest, __tmp_src); \
		} else { \
			g_critical("Trying to add to a GList* a non GObject pointer dest=%s src=%s", #dest, #src); \
		} \
	}

#define lasso_list_add_xml_node(dest, src) \
	{ \
		xmlNode *__tmp_src = src; \
		lasso_list_add_non_null(dest, __tmp_src); \
	}

#define lasso_list_add_gstrv(dest, src) \
	{ \
		GList **__tmp_dest = &(dest); \
		const char **__iter = (const char**)(src); \
		while (__iter && *__iter) { \
			lasso_list_add_string(*__tmp_dest, *__iter); \
		} \
	}

/* Pointer ownership transfer */
#define lasso_transfer_full(dest, src, kind) \
	{\
		lasso_release_##kind((dest)); \
		lasso_check_type_equality(dest, src); \
		(dest) = (void*)(src); \
		(src) = NULL; \
	}

#define lasso_transfer_xpath_object(dest, src) \
	lasso_transfer_full(dest, src, xpath_object)

#define lasso_transfer_string(dest, src) \
	lasso_transfer_full(dest, src, string)

#define lasso_transfer_gobject(dest, src) \
	lasso_transfer_full(dest, src, gobject)

/* Node extraction */
#define lasso_extract_node_or_fail(to, from, kind, error) \
	{\
		void *__tmp = (from); \
		if (LASSO_IS_##kind(__tmp)) { \
			to = LASSO_##kind(__tmp); \
		} else { \
			rc = error; \
			goto cleanup; \
		}\
	}

/* Bad param handling */
#define lasso_return_val_if_invalid_param(kind, name, val) \
	g_return_val_if_fail(LASSO_IS_##kind(name), val)

#define lasso_bad_param(kind, name) \
	lasso_return_val_if_invalid_param(kind, name, \
		LASSO_PARAM_ERROR_BAD_TYPE_OR_NULL_OBJ);

#define lasso_null_param(name) \
	g_return_val_if_fail(name != NULL, LASSO_PARAM_ERROR_BAD_TYPE_OR_NULL_OBJ);

inline static gboolean
lasso_is_empty_string(const char *str) {
	return ((str) == NULL || (str)[0] == '\0');
}

/**
 * lasso_check_non_empty_string:
 * @str: a char pointer
 *
 * Check that @str is non-NULL and not empty, otherwise jump to cleanup and return
 * LASSO_PARAM_ERROR_BAD_TYPE_OR_NULL_OBJ.
 */
#define lasso_check_non_empty_string(str) \
	goto_cleanup_if_fail_with_rc(! lasso_is_empty_string(str), \
			LASSO_PARAM_ERROR_BAD_TYPE_OR_NULL_OBJ);


/*
 * The following macros are made to create some formalism for function's cleanup code.
 *
 * The exit label should be called 'cleanup'. And for functions returning an integer error code, the
 * error code should be named 'rc' and 'return rc;' should be the last statement of the function.
 */

/**
 * goto_cleanup_with_rc:
 * @rc_value: integer return value
 *
 * This macro jump to the 'cleanup' label and set the return value to @rc_value.
 *
 */
#define goto_cleanup_with_rc(rc_value) \
	{\
		rc = (rc_value); \
		goto cleanup; \
	}

/**
 * goto_cleanup_if_fail:
 * @condition: a boolean condition
 *
 * Jump to the 'cleanup' label if the @condition is FALSE.
 *
 */
#define goto_cleanup_if_fail(condition) \
	{\
		if (! (condition) ) {\
			goto cleanup; \
		} \
	}

/**
 * goto_cleanup_if_fail_with_rc:
 * @condition: a boolean condition
 * @rc_value: integer return value
 *
 * Jump to the 'cleanup' label if the @condition is FALSE and set the return value to
 * @rc_value.
 *
 */
#define goto_cleanup_if_fail_with_rc(condition, rc_value) \
	{\
		if (! (condition) ) {\
			rc = (rc_value); \
			goto cleanup; \
		} \
	}

/**
 * goto_cleanup_if_fail_with_rc_with_warning:
 * @condition: a boolean condition
 * @rc_value: integer return value
 *
 * Jump to the 'cleanup' label if the @condition is FALSE and set the return value to
 * @rc_value. Also emit a warning, showing the condition and the return value.
 *
 */
#define goto_cleanup_if_fail_with_rc_with_warning(condition, rc_value) \
	{\
		if (! (condition) ) {\
			g_warning("%s failed, returning %s", #condition, #rc_value);\
			rc = (rc_value); \
			goto cleanup; \
		} \
	}

/**
 * check_good_rc:
 * @what: a call to a function returning a lasso error code
 *
 * Check if return code is 0, if not store it in rc and jump to cleanup label.
 */
#define lasso_check_good_rc(what) \
	{ \
		int __rc = (what);\
		goto_cleanup_if_fail_with_rc(__rc == 0, __rc); \
	}

#define lasso_mem_debug(who, what, where) \
	{ \
		if (lasso_flag_memory_debug) \
		fprintf(stderr, "  freeing %s/%s (at %p)\n", who, what, (void*)where); \
	}

#define lasso_foreach(_iter, _list) \
	for (_iter = (_list); _iter; _iter = g_list_next(_iter))

/**
 * lasso_foreach_full_begin:
 * @_type: the type of the variable @_data
 * @_data: the name of the variable to define to store data values
 * @_iter: the name of the variable to define to store the iterator
 * @_list: the GList* to iterate
 *
 * Traverse a GList* @_list, using @_iter as iteration variable extract data field to variable
 * @_data of type @_type.
 */
#define lasso_foreach_full_begin(_type, _data, _iter, _list) \
	{ \
		_type _data = NULL; \
		GList *_iter = NULL; \
		for (_iter = (_list); _iter && ((_data = _iter->data), 1); _iter = g_list_next(_iter)) \
		{

#define lasso_foreach_full_end() \
				} }

#define lasso_list_get_first_child(list) \
	((list) ? (list)->data : NULL)

/* Declare type of element in a container */
#ifndef OFTYPE
#define OFTYPE(x)
#endif

/* Get a printable extract for error messages */
char* lasso_safe_prefix_string(const char *str, gsize length);
int lasso_gobject_is_of_type(GObject *a, GType b);

/* Get first node of this type in a list */
/* ex: lasso_extract_node (LassoNode, LASSO_TYPE_NODE, list) */
#define lasso_extract_gobject_from_list(type, gobjecttype, list) \
	(type*) g_list_find_custom(list, \
			(gconstpointer)gobjecttype, (GCompareFunc)lasso_gobject_is_of_type);

#endif /* __LASSO_UTILS_H__ */
