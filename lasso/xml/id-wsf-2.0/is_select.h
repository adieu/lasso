/* $Id: is_select.h,v 1.0 2005/10/14 15:17:55 fpeters Exp $
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LASSO_IDWSF2_IS_SELECT_H__
#define __LASSO_IDWSF2_IS_SELECT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../xml.h"
#include "is_inquiry_element.h"

#define LASSO_TYPE_IDWSF2_IS_SELECT (lasso_idwsf2_is_select_get_type())
#define LASSO_IDWSF2_IS_SELECT(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), \
		LASSO_TYPE_IDWSF2_IS_SELECT, \
		LassoIdWsf2IsSelect))
#define LASSO_IDWSF2_IS_SELECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), \
		LASSO_TYPE_IDWSF2_IS_SELECT, \
		LassoIdWsf2IsSelectClass))
#define LASSO_IS_IDWSF2_IS_SELECT(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), \
		LASSO_TYPE_IDWSF2_IS_SELECT))
#define LASSO_IS_IDWSF2_IS_SELECT_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), \
		LASSO_TYPE_IDWSF2_IS_SELECT))
#define LASSO_IDWSF2_IS_SELECT_GET_CLASS(o) \
	(G_TYPE_INSTANCE_GET_CLASS ((o), \
		LASSO_TYPE_IDWSF2_IS_SELECT, \
		LassoIdWsf2IsSelectClass))


typedef struct _LassoIdWsf2IsSelect LassoIdWsf2IsSelect;
typedef struct _LassoIdWsf2IsSelectClass LassoIdWsf2IsSelectClass;


struct _LassoIdWsf2IsSelect {
	LassoIdWsf2IsInquiryElement parent;

	/*< public >*/
	/* elements */
	GList *Item; /* of LassoNode */
};


struct _LassoIdWsf2IsSelectClass {
	LassoIdWsf2IsInquiryElementClass parent;
};

LASSO_EXPORT GType lasso_idwsf2_is_select_get_type(void);
LASSO_EXPORT LassoIdWsf2IsSelect* lasso_idwsf2_is_select_new(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LASSO_IDWSF2_IS_SELECT_H__ */
