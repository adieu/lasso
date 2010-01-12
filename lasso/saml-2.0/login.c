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

#include "../xml/private.h"
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "./providerprivate.h"
#include "./loginprivate.h"
#include "./profileprivate.h"
#include "./federationprivate.h"
#include "./saml2_helper.h"

#include "../id-ff/providerprivate.h"
#include "../id-ff/serverprivate.h"
#include "../id-ff/login.h"
#include "../id-ff/identityprivate.h"
#include "../id-ff/sessionprivate.h"
#include "../id-ff/loginprivate.h"

#include "../xml/xml_enc.h"

#include "../xml/saml-2.0/samlp2_authn_request.h"
#include "../xml/saml-2.0/samlp2_response.h"
#include "../xml/saml-2.0/saml2_assertion.h"
#include "../xml/saml-2.0/saml2_audience_restriction.h"
#include "../xml/saml-2.0/saml2_authn_statement.h"
#include "../xml/saml-2.0/saml2_encrypted_element.h"
#include "../xml/saml-2.0/saml2_attribute.h"
#include "../xml/saml-2.0/saml2_attribute_statement.h"
#include "../xml/saml-2.0/saml2_attribute_value.h"
#include "../xml/saml-2.0/saml2_name_id.h"

#include "../utils.h"

static int lasso_saml20_login_process_federation(LassoLogin *login, gboolean is_consent_obtained);
static gboolean lasso_saml20_login_must_ask_for_consent_private(LassoLogin *login);
static gint lasso_saml20_login_process_response_status_and_assertion(LassoLogin *login);
static char* lasso_saml20_login_get_assertion_consumer_service_url(LassoLogin *login,
		LassoProvider *remote_provider);

/* No need to check type of arguments, it has been done in lasso_login_* methods */

gint
lasso_saml20_login_init_authn_request(LassoLogin *login, LassoHttpMethod http_method)
{
	LassoProfile *profile = NULL;
	LassoSamlp2RequestAbstract *request = NULL;
	gchar *default_name_id_format = NULL;
	int rc = 0;

	profile = &login->parent;

	/* new */
	request = (LassoSamlp2RequestAbstract*)lasso_samlp2_authn_request_new();
	lasso_check_good_rc(lasso_saml20_init_request(profile, profile->remote_providerID, FALSE,
				request, http_method, LASSO_MD_PROTOCOL_TYPE_SINGLE_SIGN_ON));

	/* FIXME: keep old behaviour */
	login->http_method = login->parent.http_request_method;

	/* save request ID, for later check */
	lasso_assign_string(login->private_data->request_id, request->ID);
	/* set name id policy */
	lasso_assign_new_gobject(LASSO_SAMLP2_AUTHN_REQUEST(request)->NameIDPolicy,
			lasso_samlp2_name_id_policy_new());
	/* set name id policy format */
	/* no need to check server, done in init_request */
	default_name_id_format = lasso_provider_get_default_name_id_format(&profile->server->parent);
	if (default_name_id_format) {
		/* steal the string */
		lasso_assign_new_string(LASSO_SAMLP2_AUTHN_REQUEST(request)->NameIDPolicy->Format,
				default_name_id_format);
	} else {
		lasso_assign_string(LASSO_SAMLP2_AUTHN_REQUEST(request)->NameIDPolicy->Format,
			LASSO_SAML2_NAME_IDENTIFIER_FORMAT_TRANSIENT);
	}
	/* set name id policy SP qualifier (the 'destination' of the NameID) */
	lasso_assign_string(LASSO_SAMLP2_AUTHN_REQUEST(request)->NameIDPolicy->SPNameQualifier,
		request->Issuer->content);

cleanup:
	return rc;
}

static gboolean
_lasso_login_must_sign_non_authn_request(LassoLogin *profile)
{
	switch (lasso_profile_get_signature_hint(&profile->parent)) {
		case LASSO_PROFILE_SIGNATURE_HINT_MAYBE:
			return lasso_flag_add_signature;
		case LASSO_PROFILE_SIGNATURE_HINT_FORCE:
			return TRUE;
		case LASSO_PROFILE_SIGNATURE_HINT_FORBID:
			return FALSE;
		default:
			return TRUE;
	}
}

static gboolean
_lasso_login_must_sign(LassoProfile *profile)
{
	char *md_authnRequestsSigned;
	gboolean ret;

	switch (lasso_profile_get_signature_hint(profile)) {
		case LASSO_PROFILE_SIGNATURE_HINT_MAYBE:
			md_authnRequestsSigned = lasso_provider_get_metadata_one(
					LASSO_PROVIDER(profile->server), "AuthnRequestsSigned");
			/* default is to sign ! */
			ret = ! md_authnRequestsSigned || g_strcmp0(md_authnRequestsSigned, "false") != 0;
			lasso_release_string(md_authnRequestsSigned);
			return ret;
		case LASSO_PROFILE_SIGNATURE_HINT_FORCE:
			return TRUE;
		case LASSO_PROFILE_SIGNATURE_HINT_FORBID:
			return FALSE;
		default:
			return TRUE;
	}
}

gint
lasso_saml20_login_build_authn_request_msg(LassoLogin *login)
{
	char *url = NULL;
	gboolean must_sign = TRUE;
	LassoProfile *profile;
	LassoSamlp2AuthnRequest *authn_request;
	int rc = 0;
	LassoHttpMethod http_method;

	profile = &login->parent;
	http_method = login->http_method;

	lasso_extract_node_or_fail(authn_request, profile->request, SAMLP2_AUTHN_REQUEST,
			LASSO_PROFILE_ERROR_INVALID_REQUEST);

	/* default is to sign ! */
	must_sign = _lasso_login_must_sign(profile);

	if (! must_sign) {
		lasso_node_remove_signature(profile->request);
	}

	/* support old way of doing PAOS */
	if (login->http_method == LASSO_HTTP_METHOD_SOAP
			&& g_strcmp0(authn_request->ProtocolBinding,
				LASSO_SAML2_METADATA_BINDING_PAOS) == 0) {
		login->http_method = LASSO_HTTP_METHOD_PAOS;
		/* PAOS is special, the url passed to build_request is the AssertionConsumerServiceURL of
		 * this SP, not the destination. */
		url = lasso_saml20_login_get_assertion_consumer_service_url(login,
				LASSO_PROVIDER(profile->server));
	}

	lasso_check_good_rc(lasso_saml20_profile_build_request_msg(profile, "SingleSignOnService",
				login->http_method, url));

cleanup:
	return rc;
}

int
lasso_saml20_login_process_authn_request_msg(LassoLogin *login, const char *authn_request_msg)
{
	LassoNode *request;
	LassoProfile *profile = LASSO_PROFILE(login);
	LassoSamlp2StatusResponse *response;
	LassoSamlp2AuthnRequest *authn_request;
	gchar *protocol_binding;
	int rc = 0;

	if (authn_request_msg == NULL) {
		if (profile->request == NULL) {
			return critical_error(LASSO_PROFILE_ERROR_MISSING_REQUEST);
		}

		/* AuthnRequest already set by .._init_idp_initiated_authn_request */
		request = profile->request;
	} else {
		request = lasso_samlp2_authn_request_new();
		lasso_check_good_rc(lasso_saml20_profile_process_any_request(profile, request, authn_request_msg));
	}
	authn_request = LASSO_SAMLP2_AUTHN_REQUEST(request);
	protocol_binding = authn_request->ProtocolBinding;
	if (protocol_binding == NULL) {
		/* protocol binding not set; so it will look into
		 * AssertionConsumingServiceIndex
		 * Also, if AssertionConsumerServiceIndex is not set in request,
		 * its value will be -1, which is just the right value to get
		 * default assertion consumer...  (convenient)
		 */
		gchar *binding;
		LassoProvider *remote_provider;
		int service_index = authn_request->AssertionConsumerServiceIndex;

		remote_provider = lasso_server_get_provider(profile->server, profile->remote_providerID);
		if (remote_provider == NULL) {
			return critical_error(LASSO_PROFILE_ERROR_MISSING_REMOTE_PROVIDERID);
		}

		binding = lasso_saml20_provider_get_assertion_consumer_service_binding(
				remote_provider, service_index);
		if (binding == NULL) {
			if (service_index == -1)
				return LASSO_LOGIN_ERROR_NO_DEFAULT_ENDPOINT;
		} else if (g_strcmp0(binding, "HTTP-Artifact") == 0) {
			login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_ART;
		} else if (g_strcmp0(binding, "HTTP-POST") == 0) {
			login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_POST;
		} else if (g_strcmp0(binding, "HTTP-Redirect") == 0) {
			login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_REDIRECT;
		} else if (g_strcmp0(binding, "SOAP") == 0) {
			login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_LECP;
		} else if (g_strcmp0(binding, "PAOS") == 0) {
			login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_LECP;
		}
		lasso_release_string(binding);
	} else if (g_strcmp0(protocol_binding, LASSO_SAML2_METADATA_BINDING_ARTIFACT) == 0) {
		login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_ART;
	} else if (g_strcmp0(protocol_binding, LASSO_SAML2_METADATA_BINDING_POST) == 0) {
		login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_POST;
	} else if (g_strcmp0(protocol_binding, LASSO_SAML2_METADATA_BINDING_SOAP) == 0) {
		login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_LECP;
	} else if (g_strcmp0(protocol_binding, LASSO_SAML2_METADATA_BINDING_REDIRECT) == 0) {
		login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_REDIRECT;
	} else if (g_strcmp0(protocol_binding, LASSO_SAML2_METADATA_BINDING_PAOS) == 0) {
		login->protocolProfile = LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_LECP;
	} else {
		message(G_LOG_LEVEL_CRITICAL,
				"unhandled protocol binding: %s", protocol_binding);
	}


	response = (LassoSamlp2StatusResponse*) lasso_samlp2_response_new();
	if (profile->signature_status) {
		lasso_check_good_rc(lasso_saml20_profile_init_response(profile, response,
					LASSO_SAML2_STATUS_CODE_REQUESTER,
					LASSO_LIB_STATUS_CODE_INVALID_SIGNATURE));
		rc = profile->signature_status;
	} else {
		lasso_check_good_rc(lasso_saml20_profile_init_response(profile, response,
					LASSO_SAML2_STATUS_CODE_SUCCESS, NULL));
	}
	lasso_assign_string(response->InResponseTo, LASSO_SAMLP2_REQUEST_ABSTRACT(profile->request)->ID);

cleanup:
	return rc;
}


gboolean
lasso_saml20_login_must_authenticate(LassoLogin *login)
{
	LassoSamlp2AuthnRequest *request;
	gboolean matched = TRUE;
	GList *assertions = NULL;
	LassoProfile *profile = &login->parent;

	if (! LASSO_IS_SAMLP2_AUTHN_REQUEST(profile->request))
		return FALSE;

	request = LASSO_SAMLP2_AUTHN_REQUEST(profile->request);
	if (request->ForceAuthn == TRUE && request->IsPassive == FALSE)
		return TRUE;

	if (request->RequestedAuthnContext) {
		char *comparison = request->RequestedAuthnContext->Comparison;
		GList *class_refs = request->RequestedAuthnContext->AuthnContextClassRef;
		char *class_ref;
		GList *t1, *t2;
		int compa;

		if (comparison == NULL || g_strcmp0(comparison, "exact") == 0) {
			compa = 0;
		} else if (g_strcmp0(comparison, "minimum") == 0) {
			message(G_LOG_LEVEL_CRITICAL, "'minimum' comparison is not implemented");
			compa = 0;
		} else if (g_strcmp0(comparison, "better") == 0) {
			message(G_LOG_LEVEL_CRITICAL, "'better' comparison is not implemented");
			compa = 0;
		} else if (g_strcmp0(comparison, "maximum") == 0) {
			message(G_LOG_LEVEL_CRITICAL, "'maximum' comparison is not implemented");
			compa = 0;
		}

		if (class_refs) {
			matched = FALSE;
		}

		assertions = lasso_session_get_assertions(profile->session, NULL);
		for (t1 = class_refs; t1 && !matched; t1 = g_list_next(t1)) {
			class_ref = t1->data;
			for (t2 = assertions; t2 && !matched; t2 = g_list_next(t2)) {
				LassoSaml2Assertion *assertion;
				LassoSaml2AuthnStatement *as = NULL;
				char *method;
				GList *t3;

				if (LASSO_IS_SAML2_ASSERTION(t2->data) == FALSE) {
					continue;
				}

				assertion = t2->data;

				for (t3 = assertion->AuthnStatement; t3; t3 = g_list_next(t3)) {
					if (LASSO_IS_SAML2_AUTHN_STATEMENT(t3->data)) {
						as = t3->data;
						break;
					}
				}

				if (as == NULL)
					continue;

				if (as->AuthnContext == NULL)
					continue;

				method = as->AuthnContext->AuthnContextClassRef;

				if (compa == 0) { /* exact */
					if (g_strcmp0(method, class_ref) == 0) {
						matched = TRUE;
						break;
					}
				} else if (compa == 1) { /* minimum */
					/* XXX: implement 'minimum' comparison */
				} else if (compa == 2) { /* better */
					/* XXX: implement 'better' comparison */
				} else if (compa == 3) { /* maximum */
					/* XXX: implement 'maximum' comparison */
				}
			}
		}
	} else {
		/* if nothing specific was asked; don't look for any
		 * particular assertions, one is enough
		 */
		matched = (profile->session != NULL && \
				lasso_session_count_assertions(profile->session) > 0);
	}
	if (assertions) {
		g_list_free(assertions);
	}
	if (matched == FALSE && request->IsPassive == FALSE)
		return TRUE;
	if (profile->identity == NULL && request->IsPassive) {
		lasso_saml20_profile_set_response_status_responder(LASSO_PROFILE(login),
				LASSO_SAML2_STATUS_CODE_NO_PASSIVE);
		return FALSE;
	}
	return FALSE;
}

static gboolean
lasso_saml20_login_must_ask_for_consent_private(LassoLogin *login)
{
	LassoProfile *profile = LASSO_PROFILE(login);
	LassoSamlp2NameIDPolicy *name_id_policy;
	char *consent;
	LassoFederation *federation;
	char *name_id_sp_name_qualifier = NULL;
	LassoProvider *remote_provider;
	gboolean rc = TRUE;

	name_id_policy = LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy;

	if (name_id_policy) {
		char *format = name_id_policy->Format;
		if (g_strcmp0(format, LASSO_SAML2_NAME_IDENTIFIER_FORMAT_TRANSIENT) == 0) {
			goto_cleanup_with_rc (FALSE)
		}
		if (name_id_policy->AllowCreate == FALSE) {
			goto_cleanup_with_rc (FALSE)
		}
	}

	remote_provider = lasso_server_get_provider(profile->server, profile->remote_providerID);
	name_id_sp_name_qualifier = lasso_provider_get_sp_name_qualifier(remote_provider);

	/* if something goes wrong better to ask thant to let go */
	if (name_id_sp_name_qualifier == NULL)
		goto_cleanup_with_rc (TRUE)

	if (profile->identity && profile->identity->federations) {
		federation = g_hash_table_lookup(profile->identity->federations,
				name_id_sp_name_qualifier);
		if (federation) {
			goto_cleanup_with_rc (FALSE)
		}
	}

	consent = LASSO_SAMLP2_REQUEST_ABSTRACT(profile->request)->Consent;
	if (consent == NULL)
		goto_cleanup_with_rc (FALSE)

	if (g_strcmp0(consent, LASSO_SAML2_CONSENT_OBTAINED) == 0)
		goto_cleanup_with_rc (FALSE)

	if (g_strcmp0(consent, LASSO_SAML2_CONSENT_PRIOR) == 0)
		goto_cleanup_with_rc (FALSE)

	if (g_strcmp0(consent, LASSO_SAML2_CONSENT_IMPLICIT) == 0)
		goto_cleanup_with_rc (FALSE)

	if (g_strcmp0(consent, LASSO_SAML2_CONSENT_EXPLICIT) == 0)
		goto_cleanup_with_rc (FALSE)

	if (g_strcmp0(consent, LASSO_SAML2_CONSENT_UNAVAILABLE) == 0)
		goto_cleanup_with_rc (TRUE)

	if (g_strcmp0(consent, LASSO_SAML2_CONSENT_INAPPLICABLE) == 0)
		goto_cleanup_with_rc (TRUE)

cleanup:
	lasso_release_string (name_id_sp_name_qualifier);
	return rc;
}

gboolean
lasso_saml20_login_must_ask_for_consent(LassoLogin *login)
{
	LassoProfile *profile = LASSO_PROFILE(login);

	if (LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->IsPassive)
		return FALSE;

	return lasso_saml20_login_must_ask_for_consent_private(login);
}

int
lasso_saml20_login_validate_request_msg(LassoLogin *login, gboolean authentication_result,
		gboolean is_consent_obtained)
{
	LassoProfile *profile;
	int ret = 0;

	profile = LASSO_PROFILE(login);

	if (authentication_result == FALSE) {
		lasso_saml20_profile_set_response_status_responder(profile,
				LASSO_SAML2_STATUS_CODE_REQUEST_DENIED);
		return LASSO_LOGIN_ERROR_REQUEST_DENIED;
	}

	if (profile->signature_status == LASSO_DS_ERROR_INVALID_SIGNATURE) {
		lasso_saml20_profile_set_response_status_requester(profile,
				LASSO_LIB_STATUS_CODE_INVALID_SIGNATURE);
		return LASSO_LOGIN_ERROR_INVALID_SIGNATURE;
	}

	if (profile->signature_status == LASSO_DS_ERROR_SIGNATURE_NOT_FOUND) {
		lasso_saml20_profile_set_response_status_requester(profile,
				LASSO_LIB_STATUS_CODE_INVALID_SIGNATURE);
		return LASSO_LOGIN_ERROR_UNSIGNED_AUTHN_REQUEST;
	}

	if (profile->signature_status == 0 && authentication_result == TRUE) {
		ret = lasso_saml20_login_process_federation(login, is_consent_obtained);
		if (ret == LASSO_LOGIN_ERROR_FEDERATION_NOT_FOUND) {
			lasso_saml20_profile_set_response_status_requester(profile,
				LASSO_LIB_STATUS_CODE_FEDERATION_DOES_NOT_EXIST);
			return ret;
		}
		/* PROVIDER_NOT_FOUND, CONSENT_NOT_OBTAINED */
		if (ret) {
			lasso_saml20_profile_set_response_status_responder(profile,
				LASSO_SAML2_STATUS_CODE_REQUEST_DENIED);
			return ret;
		}
	}

	lasso_saml20_profile_set_response_status_success(profile, NULL);

	return ret;
}

static int
lasso_saml20_login_process_federation(LassoLogin *login, gboolean is_consent_obtained)
{
	LassoProfile *profile = LASSO_PROFILE(login);
	LassoSamlp2NameIDPolicy *name_id_policy;
	char *name_id_policy_format = NULL;
	LassoFederation *federation;
	char *name_id_sp_name_qualifier = NULL;
	LassoProvider *remote_provider;
	int rc = 0;

	/* verify if identity already exists else create it */
	if (profile->identity == NULL) {
		profile->identity = lasso_identity_new();
	}

	name_id_policy = LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy;
	if (name_id_policy) {
		name_id_policy_format = name_id_policy->Format;
	} else {
		name_id_policy_format = LASSO_SAML2_NAME_IDENTIFIER_FORMAT_PERSISTENT;
	}

	lasso_assign_string(login->nameIDPolicy, name_id_policy_format);

	if (name_id_policy_format && g_strcmp0(name_id_policy_format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_TRANSIENT) == 0) {
		goto_cleanup_with_rc (0)
	}

	remote_provider = lasso_server_get_provider(profile->server, profile->remote_providerID);
	if (! LASSO_IS_PROVIDER(remote_provider)) {
		goto_cleanup_with_rc (LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND)
	}
	name_id_sp_name_qualifier = lasso_provider_get_sp_name_qualifier(remote_provider);
	if (name_id_sp_name_qualifier == NULL) {
		goto_cleanup_with_rc (LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND)
	}

	/* search a federation in the identity */
	federation = g_hash_table_lookup(profile->identity->federations, name_id_sp_name_qualifier);
	if (name_id_policy == NULL || name_id_policy->AllowCreate == FALSE) {
		if (LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy == NULL) {
			/* it tried to get a federation, it failed, this is not
			 * a problem */
			goto_cleanup_with_rc (0)
		}
		/* a federation MUST exist */
		if (federation == NULL) {
			goto_cleanup_with_rc (LASSO_LOGIN_ERROR_FEDERATION_NOT_FOUND)
		}
	}

	if (federation == NULL &&
			LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy == NULL) {
		/* it didn't find a federation, and name id policy was not
		 * specified, don't create a federation */
		goto_cleanup_with_rc (0)
	}

	if (federation && LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy == NULL) {
		lasso_assign_new_gobject(LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy,
				LASSO_SAMLP2_NAME_ID_POLICY(lasso_samlp2_name_id_policy_new()));
		lasso_assign_string(LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy->Format,
			LASSO_SAML2_NAME_IDENTIFIER_FORMAT_PERSISTENT);
	}

	if (lasso_saml20_login_must_ask_for_consent_private(login) && !is_consent_obtained) {
		goto_cleanup_with_rc (LASSO_LOGIN_ERROR_CONSENT_NOT_OBTAINED)
	}

	if (federation == NULL) {
		federation = lasso_federation_new(name_id_sp_name_qualifier);
		lasso_saml20_federation_build_local_name_identifier(federation,
				LASSO_PROVIDER(profile->server)->ProviderID,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_PERSISTENT,
				NULL);
		lasso_assign_string(LASSO_SAML2_NAME_ID(federation->local_nameIdentifier)->SPNameQualifier,
				name_id_sp_name_qualifier);
		lasso_identity_add_federation(profile->identity, federation);
	}

	lasso_assign_gobject(profile->nameIdentifier, federation->local_nameIdentifier);

cleanup:
	lasso_release_string (name_id_sp_name_qualifier);
	return rc;
}

static LassoFederation*
_lasso_login_saml20_get_federation(LassoLogin *login) {
	LassoFederation *federation = NULL;
	char *name_id_sp_name_qualifier;


	name_id_sp_name_qualifier = lasso_provider_get_sp_name_qualifier(
			lasso_server_get_provider(login->parent.server, login->parent.remote_providerID));
	federation = lasso_identity_get_federation(login->parent.identity, name_id_sp_name_qualifier);
	lasso_release_string(name_id_sp_name_qualifier);
	return federation;
}

int
lasso_saml20_login_build_assertion(LassoLogin *login,
		const char *authenticationMethod,
		const char *authenticationInstant,
		const char *notBefore,
		const char *notOnOrAfter)
{
	LassoProfile *profile = &login->parent;
#if 0
	LassoFederation *federation = NULL;
#endif
	LassoSaml2Assertion *assertion = NULL;
	LassoSaml2AudienceRestriction *audience_restriction = NULL;
	LassoSamlp2NameIDPolicy *name_id_policy = NULL;
	LassoSaml2NameID *name_id = NULL;
	LassoSaml2AuthnStatement *authentication_statement;
	LassoProvider *provider = NULL;
	LassoSamlp2Response *response = NULL;
	LassoSamlp2RequestAbstract *request_abstract = NULL;
	LassoSamlp2AuthnRequest *authn_request = NULL;
	gboolean do_encrypt_nameid = FALSE;
	gboolean do_encrypt_assertion = FALSE;
	int rc = 0;

	provider = lasso_server_get_provider(profile->server, profile->remote_providerID);

	if (provider) {
		do_encrypt_nameid = lasso_provider_get_encryption_mode(provider) &
			LASSO_ENCRYPTION_MODE_NAMEID;
		do_encrypt_assertion = lasso_provider_get_encryption_mode(provider) &
			LASSO_ENCRYPTION_MODE_ASSERTION;
	}

	if (LASSO_IS_SAMLP2_AUTHN_REQUEST(profile->request)) {
		authn_request = (LassoSamlp2AuthnRequest*)profile->request;
		request_abstract = &authn_request->parent;
	}
	goto_cleanup_if_fail_with_rc(LASSO_IS_SAMLP2_RESPONSE(profile->response),
			LASSO_PROFILE_ERROR_MISSING_RESPONSE);

	response = (LassoSamlp2Response*)profile->response;

	assertion = LASSO_SAML2_ASSERTION(lasso_saml2_assertion_new());
	assertion->ID = lasso_build_unique_id(32);
	lasso_assign_string(assertion->Version, "2.0");
	assertion->IssueInstant = lasso_get_current_time();
	assertion->Issuer = LASSO_SAML2_NAME_ID(lasso_saml2_name_id_new_with_string(
			LASSO_PROVIDER(profile->server)->ProviderID));
	assertion->Conditions = LASSO_SAML2_CONDITIONS(lasso_saml2_conditions_new());

	audience_restriction = LASSO_SAML2_AUDIENCE_RESTRICTION(
			lasso_saml2_audience_restriction_new());
	lasso_assign_string(audience_restriction->Audience, profile->remote_providerID);
	lasso_list_add_new_gobject(assertion->Conditions->AudienceRestriction, audience_restriction);

	assertion->Subject = LASSO_SAML2_SUBJECT(lasso_saml2_subject_new());
	assertion->Subject->SubjectConfirmation = LASSO_SAML2_SUBJECT_CONFIRMATION(
			lasso_saml2_subject_confirmation_new());
	assertion->Subject->SubjectConfirmation->Method = g_strdup(
			LASSO_SAML2_CONFIRMATION_METHOD_BEARER);
	assertion->Subject->SubjectConfirmation->SubjectConfirmationData =
		LASSO_SAML2_SUBJECT_CONFIRMATION_DATA(
			lasso_saml2_subject_confirmation_data_new());
	assertion->Subject->SubjectConfirmation->SubjectConfirmationData->NotBefore = g_strdup(
		notBefore);
	assertion->Subject->SubjectConfirmation->SubjectConfirmationData->NotOnOrAfter = g_strdup(
		notOnOrAfter);

	/* If request is present, refer to it in the response */
	if (authn_request) {
		if (request_abstract->ID) {
			lasso_assign_string(assertion->Subject->SubjectConfirmation->SubjectConfirmationData->InResponseTo,
					request_abstract->ID);
			/*
			 * It MUST NOT contain a NotBefore attribute. If
			 * the containing message is in response to an <AuthnRequest>,
			 * then the InResponseTo attribute MUST match the request's ID.
			 */
			lasso_release_string(assertion->Subject->SubjectConfirmation->SubjectConfirmationData->NotBefore);
		}
		name_id_policy = authn_request->NameIDPolicy;
	}
	/* TRANSIENT */
	if (!name_id_policy || g_strcmp0(name_id_policy->Format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_TRANSIENT) == 0) {
		name_id = (LassoSaml2NameID*)lasso_saml2_name_id_new_with_string(
					lasso_build_unique_id(32));
		lasso_assign_string(name_id->NameQualifier,
				lasso_provider_get_sp_name_qualifier(&profile->server->parent));
		lasso_assign_string(name_id->Format, LASSO_SAML2_NAME_IDENTIFIER_FORMAT_TRANSIENT);
		assertion->Subject->NameID = name_id;
	/* FEDERATED */
	} else if (g_strcmp0(name_id_policy->Format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_PERSISTENT) == 0 ||
			g_strcmp0(name_id_policy->Format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_ENCRYPTED) == 0) {
		LassoFederation *federation;

		federation = _lasso_login_saml20_get_federation(login);
		goto_cleanup_if_fail_with_rc(federation != NULL,
				LASSO_PROFILE_ERROR_FEDERATION_NOT_FOUND);

		if (g_strcmp0(name_id_policy->Format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_ENCRYPTED) == 0) {
			do_encrypt_nameid = TRUE;
		}
		lasso_assign_gobject(assertion->Subject->NameID,
				federation->local_nameIdentifier);
	/* ALL OTHER KIND OF NAME ID FORMATS */
	} else {
		/* caller must set the name identifier content afterwards */
		name_id = LASSO_SAML2_NAME_ID(lasso_saml2_name_id_new());
		lasso_assign_string(name_id->NameQualifier,
				LASSO_PROVIDER(profile->server)->ProviderID);
		lasso_assign_string(name_id->Format, name_id_policy->Format);
		assertion->Subject->NameID = name_id;
		if (do_encrypt_nameid) {
			message(G_LOG_LEVEL_WARNING, "NameID encryption is currently not "
					"supported with non transient or persisent NameID format");
			do_encrypt_nameid = FALSE;
		}
	}

	authentication_statement = LASSO_SAML2_AUTHN_STATEMENT(lasso_saml2_authn_statement_new());
	authentication_statement->AuthnInstant = g_strdup(authenticationInstant);
	authentication_statement->SessionNotOnOrAfter = g_strdup(notOnOrAfter);
	authentication_statement->AuthnContext = LASSO_SAML2_AUTHN_CONTEXT(
			lasso_saml2_authn_context_new());
	authentication_statement->AuthnContext->AuthnContextClassRef = g_strdup(
			authenticationMethod);
	lasso_list_add_new_gobject(assertion->AuthnStatement, authentication_statement);

	/* Save signing material in assertion private datas to be able to sign later */
	lasso_check_good_rc(lasso_server_saml2_assertion_setup_signature(profile->server,
				assertion));


	/* Encrypt NameID */
	if (do_encrypt_nameid) {
		/* FIXME: as with assertions, it should be possible to setup encryption of NameID for later */
		goto_cleanup_if_fail_with_rc(provider != NULL, LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND);

		assertion->Subject->EncryptedID = (LassoSaml2EncryptedElement*)lasso_node_encrypt(
			(LassoNode*)assertion->Subject->NameID,
			lasso_provider_get_encryption_public_key(provider),
			lasso_provider_get_encryption_sym_key_type(provider),
			provider->ProviderID);
		goto_cleanup_if_fail_with_rc(assertion->Subject->EncryptedID != NULL,
				LASSO_DS_ERROR_ENCRYPTION_FAILED);
		lasso_release_gobject(assertion->Subject->NameID);
	}

	/* Save encryption material in assertion private datas to be able to encrypt later */
	if (do_encrypt_assertion) {
		assertion->encryption_activated = TRUE;
		lasso_assign_string(assertion->encryption_public_key_str,
				provider->private_data->encryption_public_key_str);
		assertion->encryption_sym_key_type =
			lasso_provider_get_encryption_sym_key_type(provider);
	}

	/* store assertion in session object */
	if (profile->session == NULL) {
		profile->session = lasso_session_new();
	}

	lasso_session_add_assertion(profile->session, profile->remote_providerID,
			LASSO_NODE(assertion));

	response = LASSO_SAMLP2_RESPONSE(profile->response);
	lasso_list_add_gobject(response->Assertion, assertion);
	lasso_assign_gobject(login->private_data->saml2_assertion, assertion);
cleanup:
	lasso_release_gobject(assertion);
	return rc;
}

gint
lasso_saml20_login_build_artifact_msg(LassoLogin *login, LassoHttpMethod http_method)
{
	LassoProfile *profile;
	LassoProvider *remote_provider;
	char *url;
	LassoSaml2Assertion *assertion;
	LassoSamlp2StatusResponse *response;
	int rc = 0;

	profile = &login->parent;

	if (profile->remote_providerID == NULL)
		return critical_error(LASSO_PROFILE_ERROR_MISSING_REMOTE_PROVIDERID);

	if (http_method != LASSO_HTTP_METHOD_ARTIFACT_GET && http_method != LASSO_HTTP_METHOD_ARTIFACT_POST) {
		return critical_error(LASSO_PROFILE_ERROR_INVALID_HTTP_METHOD);
	}

	if (! LASSO_IS_SAMLP2_RESPONSE(profile->response)) {
		return critical_error(LASSO_PROFILE_ERROR_MISSING_RESPONSE);
	}
	response = (LassoSamlp2StatusResponse*)profile->response;
	/* XXX: why checking now ? */
	if (response->Status == NULL || response->Status->StatusCode == NULL
			|| response->Status->StatusCode->Value == NULL) {
		return critical_error(LASSO_PROFILE_ERROR_MISSING_STATUS_CODE);
	}

	remote_provider = lasso_server_get_provider(profile->server, profile->remote_providerID);
	if (LASSO_IS_PROVIDER(remote_provider) == FALSE)
		return critical_error(LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND);

	url = lasso_saml20_login_get_assertion_consumer_service_url(login, remote_provider);
	assertion = login->private_data->saml2_assertion;
	if (LASSO_IS_SAML2_ASSERTION(assertion) && url) {
		LassoSaml2SubjectConfirmationData *subject_confirmation_data;

		subject_confirmation_data =
			lasso_saml2_assertion_get_subject_confirmation_data(assertion, TRUE);
		lasso_assign_string(subject_confirmation_data->Recipient, url);
	}

	lasso_check_good_rc(lasso_saml20_profile_build_response_msg(profile, NULL, http_method,
				url));

cleanup:
	lasso_release_string(url);
	return rc;
}


gint
lasso_saml20_login_init_request(LassoLogin *login, gchar *response_msg,
		LassoHttpMethod response_http_method)
{
	return lasso_saml20_profile_init_artifact_resolve(
			LASSO_PROFILE(login), response_msg, response_http_method);
}


gint
lasso_saml20_login_build_request_msg(LassoLogin *login)
{
	LassoProfile *profile;

	profile = &login->parent;
	if (_lasso_login_must_sign_non_authn_request(login)) {
		lasso_profile_saml20_setup_message_signature(profile, profile->request);
	} else {
		lasso_node_remove_signature(profile->request);
	}
	return lasso_saml20_profile_build_request_msg(profile, "ArtifactResolutionService",
			LASSO_HTTP_METHOD_SOAP, NULL);
}

gint
lasso_saml20_login_process_request_msg(LassoLogin *login, gchar *request_msg)
{
	LassoProfile *profile = LASSO_PROFILE(login);
	int rc;

	rc = lasso_saml20_profile_process_artifact_resolve(profile, request_msg);
	if (rc != 0) {
		return rc;
	}
	/* compat with liberty id-ff code */
	lasso_assign_new_string(login->assertionArtifact, lasso_profile_get_artifact(profile));
	return 0;
}

gint
lasso_saml20_login_build_response_msg(LassoLogin *login)
{
	LassoProfile *profile = LASSO_PROFILE(login);
	LassoProvider *remote_provider;
	LassoSaml2Assertion *assertion;
	int rc = 0;

	if (login->protocolProfile == LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_LECP) {
		const char *assertionConsumerURL;

		lasso_check_good_rc(lasso_profile_saml20_setup_message_signature(profile,
					profile->response));
		remote_provider = lasso_server_get_provider(profile->server, profile->remote_providerID);
		if (LASSO_IS_PROVIDER(remote_provider) == FALSE)
			return critical_error(LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND);

		assertionConsumerURL = lasso_saml20_login_get_assertion_consumer_service_url(
						login, remote_provider);

		assertion = login->private_data->saml2_assertion;
		if (LASSO_IS_SAML2_ASSERTION(assertion) == TRUE) {
			assertion->Subject->SubjectConfirmation->SubjectConfirmationData->Recipient
						= g_strdup(assertionConsumerURL);
		}

		/* build an ECP SOAP Response */
		lasso_assign_new_string(profile->msg_body, lasso_node_export_to_ecp_soap_response(
					LASSO_NODE(profile->response), assertionConsumerURL));
		return rc;
	}

	return lasso_saml20_profile_build_artifact_response(LASSO_PROFILE(login));

cleanup:
	return rc;
}

gint
lasso_saml20_login_process_paos_response_msg(LassoLogin *login, gchar *msg)
{
	LassoProfile *profile;
	int rc1, rc2;

	lasso_null_param(msg);

	profile = LASSO_PROFILE(login);
	rc1 = lasso_saml20_profile_process_soap_response(profile, msg);
	rc2 = lasso_saml20_login_process_response_status_and_assertion(login);

	if (rc1) {
		return rc1;
	}
	return rc2;
}

/**
 * lasso_saml20_login_process_authn_response_msg:
 * @login: a #LassoLogin profile object
 * @authn_response_msg: a string containg a response msg to an #LassoSaml2AuthnRequest
 *
 * Parse a response made using binding HTTP-Redirect, HTTP-Post or HTTP-SOAP.  If a signature is
 * missing on the message object, it accepts signatures coming from the first assertion as a
 * sufficient proof. But if the signature verification failed on the message for any other reason
 * than a missing Signature node, an error code is returned.
 *
 * Return value: 0 if succesfull, an error code otherwise.
 */
gint
lasso_saml20_login_process_authn_response_msg(LassoLogin *login, gchar *authn_response_msg)
{
	LassoProfile *profile = NULL;
	int rc1, rc2, message_signature_status;
	LassoSamlp2Response *samlp2_response = NULL;

	lasso_null_param(authn_response_msg);

	/* parse the message */
	profile = LASSO_PROFILE(login);
	samlp2_response = (LassoSamlp2Response*)lasso_samlp2_response_new();
	rc1 = lasso_saml20_profile_process_any_response(profile,
		(LassoSamlp2StatusResponse*)samlp2_response, NULL,
		authn_response_msg);

	message_signature_status = profile->signature_status;

	rc2 = lasso_saml20_login_process_response_status_and_assertion(login);

	/** The more important signature errors */
	lasso_release_gobject(samlp2_response);
	if (message_signature_status) {
		message(G_LOG_LEVEL_WARNING, "Validation of the AuthnResponse message signature failed: %s", lasso_strerror(message_signature_status));
	}
	if (profile->signature_status) {
		return profile->signature_status;
	}
	if (rc1) {
		return rc1;
	}
	return rc2;
}

gint
lasso_saml20_login_process_response_msg(LassoLogin *login, gchar *response_msg)
{
	LassoProfile *profile = LASSO_PROFILE(login);
	int rc;

	rc = lasso_saml20_profile_process_artifact_response(profile, response_msg);
	if (rc) {
		return rc;
	}

	return lasso_saml20_login_process_response_status_and_assertion(login);
}

static gint
lasso_saml20_login_check_assertion_signature(LassoLogin *login,
		LassoSaml2Assertion *assertion)
{
	xmlNode *original_node = NULL;
	LassoSaml2NameID *Issuer = NULL;
	LassoServer *server = NULL;
	LassoProfile *profile = NULL;
	char *remote_provider_id = NULL;
	LassoProvider *remote_provider;
	int rc = 0;

	lasso_bad_param(SAML2_ASSERTION, assertion);

	profile = (LassoProfile*)login;
	lasso_extract_node_or_fail(server, lasso_profile_get_server(profile),
			SERVER, LASSO_PROFILE_ERROR_MISSING_SERVER);

	/* Get an issuer */
	Issuer = assertion->Issuer;
	if (! Issuer || /* No issuer */
			! Issuer->content || /* No issuer content */
			(Issuer->Format &&
			 g_strcmp0(Issuer->Format, LASSO_SAML2_NAME_IDENTIFIER_FORMAT_ENTITY) != 0))
		/* Issuer format is not entity */
	{
		rc = LASSO_PROFILE_ERROR_MISSING_ISSUER;
	} else {
		remote_provider_id = Issuer->content;
	}
	remote_provider = lasso_server_get_provider(server, remote_provider_id);
	goto_cleanup_if_fail_with_rc(remote_provider, LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND);

	/* Get the original node */
	original_node = lasso_node_get_original_xmlnode(LASSO_NODE(assertion));
	goto_cleanup_if_fail_with_rc(original_node, LASSO_PROFILE_ERROR_CANNOT_VERIFY_SIGNATURE);

	rc = profile->signature_status = lasso_provider_verify_saml_signature(remote_provider, original_node, NULL);

#define log_verify_assertion_signature_error(msg) \
			message(G_LOG_LEVEL_WARNING, "Could not verify signature of assertion" \
					"ID:%s, " msg ".", assertion->ID);
cleanup:
	switch (rc) {
		case LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND:
			log_verify_assertion_signature_error("Issuer is unknown");
			break;
		case LASSO_PROFILE_ERROR_MISSING_ISSUER:
			log_verify_assertion_signature_error(
				"no Issuer found or Issuer has bad format");
			break;
		case LASSO_PROFILE_ERROR_CANNOT_VERIFY_SIGNATURE:
			log_verify_assertion_signature_error(
				" the original xmlNode is certainly not accessible anymore");

		default:
			break;
	}
#undef log_verify_assertion_signature_error
	return rc;
}

static gint
lasso_saml20_login_process_response_status_and_assertion(LassoLogin *login)
{
	LassoSamlp2StatusResponse *response;
	LassoSamlp2Response *samlp2_response = NULL;
	LassoProfile *profile;
	xmlSecKey *encryption_private_key = NULL;
	char *status_value;
	GList *it = NULL;
	int rc = 0, rc1 = 0;

	g_return_val_if_fail(LASSO_IS_LOGIN(login), LASSO_PARAM_ERROR_BAD_TYPE_OR_NULL_OBJ);

	profile = LASSO_PROFILE(login);
	response = LASSO_SAMLP2_STATUS_RESPONSE(profile->response);
	lasso_extract_node_or_fail(response, profile->response, SAMLP2_STATUS_RESPONSE,
			LASSO_PROFILE_ERROR_INVALID_MSG);
	lasso_extract_node_or_fail(samlp2_response, response, SAMLP2_RESPONSE,
			LASSO_PROFILE_ERROR_INVALID_MSG);

	if (response->Status == NULL || ! LASSO_IS_SAMLP2_STATUS(response->Status) ||
			response->Status->StatusCode == NULL ||
			response->Status->StatusCode->Value == NULL) {
		return LASSO_PROFILE_ERROR_MISSING_STATUS_CODE;
	}

	status_value = response->Status->StatusCode->Value;
	if (status_value && g_strcmp0(status_value, LASSO_SAML2_STATUS_CODE_SUCCESS) != 0) {
		if (g_strcmp0(status_value, LASSO_SAML2_STATUS_CODE_REQUEST_DENIED) == 0)
			return LASSO_LOGIN_ERROR_REQUEST_DENIED;
		if (g_strcmp0(status_value, LASSO_SAML2_STATUS_CODE_RESPONDER) == 0) {
			/* samlp:Responder */
			if (response->Status->StatusCode->StatusCode &&
					response->Status->StatusCode->StatusCode->Value) {
				status_value = response->Status->StatusCode->StatusCode->Value;
				if (g_strcmp0(status_value,
					LASSO_LIB_STATUS_CODE_FEDERATION_DOES_NOT_EXIST) == 0) {
					return LASSO_LOGIN_ERROR_FEDERATION_NOT_FOUND;
				}
				if (g_strcmp0(status_value,
						LASSO_LIB_STATUS_CODE_UNKNOWN_PRINCIPAL) == 0) {
					return LASSO_LOGIN_ERROR_UNKNOWN_PRINCIPAL;
				}
			}
		}

		return LASSO_LOGIN_ERROR_STATUS_NOT_SUCCESS;
	}


	if (LASSO_IS_SERVER(profile->server) && profile->server->private_data) {
		encryption_private_key = profile->server->private_data->encryption_private_key;
	}

	/* Decrypt all EncryptedAssertions */
	it = samlp2_response->EncryptedAssertion;
	for (;it;it = it->next) {
		LassoSaml2EncryptedElement *encrypted_assertion;
		LassoSaml2Assertion * assertion = NULL;

		if (! encryption_private_key) {
			message(G_LOG_LEVEL_WARNING, "Missing private encryption key, cannot decrypt assertions.");
			break;
		}

		if (! LASSO_IS_SAML2_ENCRYPTED_ELEMENT(it->data)) {
			message(G_LOG_LEVEL_WARNING, "EncryptedAssertion contains a non EncryptedElement object");
			continue;
		}
		encrypted_assertion = (LassoSaml2EncryptedElement*)it->data;
		rc1 = lasso_saml2_encrypted_element_decrypt(encrypted_assertion, encryption_private_key, (LassoNode**)&assertion);

		if (rc1) {
			message(G_LOG_LEVEL_WARNING, "Could not decrypt an assertion");
			continue;
		}

		if (! LASSO_IS_SAML2_ASSERTION(assertion)) {
			message(G_LOG_LEVEL_WARNING, "EncryptedAssertion contains something that is not an assertion");
			lasso_release_gobject(assertion);
			continue;
		}
		lasso_list_add_gobject(samlp2_response->Assertion, assertion);
		lasso_release_gobject(assertion);
	}

	/** FIXME: treat more than the first assertion ? */
	if (samlp2_response->Assertion != NULL) {
		LassoSaml2Subject *subject;
		LassoSaml2Assertion *assertion = samlp2_response->Assertion->data;
		int rc2 = 0;

		lasso_assign_gobject (login->private_data->saml2_assertion, assertion);

		/* If no signature was validated on the response, check the signature at the
		 * assertion level */
		if (profile->signature_status == LASSO_DS_ERROR_SIGNATURE_NOT_FOUND) {
			profile->signature_status = rc2 = lasso_saml20_login_check_assertion_signature(login, assertion);
		}

		if (! LASSO_IS_SAML2_SUBJECT(assertion->Subject)) {
			return LASSO_PROFILE_ERROR_MISSING_SUBJECT;
		}
		subject = assertion->Subject;

		/* Verify Subject->SubjectConfirmationData->InResponseTo */
		if (login->private_data->request_id && (
			assertion->Subject->SubjectConfirmation == NULL ||
			assertion->Subject->SubjectConfirmation->SubjectConfirmationData == NULL ||
			assertion->Subject->SubjectConfirmation->SubjectConfirmationData->InResponseTo == NULL ||
			g_strcmp0(assertion->Subject->SubjectConfirmation->SubjectConfirmationData->InResponseTo, login->private_data->request_id) != 0)) {
			return LASSO_LOGIN_ERROR_ASSERTION_DOES_NOT_MATCH_REQUEST_ID;
		}

		/** Handle nameid */
		rc2 = lasso_saml20_profile_process_name_identifier_decryption(profile, &subject->NameID, &subject->EncryptedID);

		if (rc2) {
			rc = rc2;
		}
	} else {
		if (rc1) {
			rc = rc1;
		} else {
			rc = LASSO_PROFILE_ERROR_MISSING_ASSERTION;
		}
	}

cleanup:

	return rc;
}


gint
lasso_saml20_login_accept_sso(LassoLogin *login)
{
	LassoProfile *profile;
	LassoSaml2Assertion *assertion;
	GList *previous_assertions, *t;
	LassoSaml2NameID *ni;
	LassoFederation *federation;

	profile = LASSO_PROFILE(login);
	if (LASSO_SAMLP2_RESPONSE(profile->response)->Assertion == NULL)
		return LASSO_PROFILE_ERROR_MISSING_ASSERTION;

	assertion = LASSO_SAMLP2_RESPONSE(profile->response)->Assertion->data;
	if (assertion == NULL)
		return LASSO_PROFILE_ERROR_MISSING_ASSERTION;

	previous_assertions = lasso_session_get_assertions(profile->session,
			profile->remote_providerID);
	for (t = previous_assertions; t; t = g_list_next(t)) {
		LassoSaml2Assertion *ta;

		if (LASSO_IS_SAML2_ASSERTION(t->data) == FALSE) {
			continue;
		}

		ta = t->data;

		if (g_strcmp0(ta->ID, assertion->ID) == 0) {
			g_list_free(previous_assertions);
			return LASSO_LOGIN_ERROR_ASSERTION_REPLAY;
		}
	}
	g_list_free(previous_assertions);

	lasso_session_add_assertion(profile->session, profile->remote_providerID,
			LASSO_NODE(assertion));

	if (assertion->Subject && assertion->Subject->NameID) {
		ni = assertion->Subject->NameID;
	} else {
		return LASSO_PROFILE_ERROR_MISSING_NAME_IDENTIFIER;
	}

	/* create federation, only if nameidentifier format is Federated */
	if (ni && ni->Format
			&& g_strcmp0(ni->Format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_PERSISTENT) == 0) {
		federation = lasso_federation_new(LASSO_PROFILE(login)->remote_providerID);

		lasso_assign_gobject(federation->local_nameIdentifier, ni);
		/* add federation in identity */
		lasso_identity_add_federation(LASSO_PROFILE(login)->identity, federation);
	}

	return 0;
}

gint
lasso_saml20_login_build_authn_response_msg(LassoLogin *login)
{
	LassoProfile *profile;
	LassoProvider *remote_provider = NULL;
	LassoSaml2Assertion *assertion = NULL;
	LassoHttpMethod http_method;
	char *url = NULL;
	int rc = 0;

	profile = &login->parent;

	if (login->protocolProfile != LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_POST &&
		login->protocolProfile != LASSO_LOGIN_PROTOCOL_PROFILE_REDIRECT) {
		return critical_error(LASSO_PROFILE_ERROR_INVALID_PROTOCOLPROFILE);
	}

	if (_lasso_login_must_sign_non_authn_request(login)) {
		lasso_check_good_rc(lasso_profile_saml20_setup_message_signature(profile,
					profile->response));
	} else {
		lasso_node_remove_signature(profile->response);
	}

	remote_provider = lasso_server_get_provider(profile->server, profile->remote_providerID);
	if (LASSO_IS_PROVIDER(remote_provider) == FALSE)
		return critical_error(LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND);

	url = lasso_saml20_login_get_assertion_consumer_service_url(login, remote_provider);

	assertion = login->private_data->saml2_assertion;
	if (LASSO_IS_SAML2_ASSERTION(assertion) && url) {
		LassoSaml2SubjectConfirmationData *subject_confirmation_data;

		subject_confirmation_data =
			lasso_saml2_assertion_get_subject_confirmation_data(assertion, TRUE);
		lasso_assign_string(subject_confirmation_data->Recipient, url);
	}

	switch (login->protocolProfile) {
		case LASSO_LOGIN_PROTOCOL_PROFILE_BRWS_POST:
			http_method = LASSO_HTTP_METHOD_POST;
			break;
		case LASSO_LOGIN_PROTOCOL_PROFILE_REDIRECT:
			http_method = LASSO_HTTP_METHOD_REDIRECT;
			break;
		default:
			g_critical("Cannot happen");
			break;
	}
	lasso_check_good_rc(lasso_saml20_profile_build_request_msg(profile, NULL, http_method, url));

cleanup:
	return rc;
}

static char*
lasso_saml20_login_get_assertion_consumer_service_url(LassoLogin *login,
	LassoProvider *remote_provider)
{
	LassoSamlp2AuthnRequest *request;
	char *url = NULL;

	request = LASSO_SAMLP2_AUTHN_REQUEST(LASSO_PROFILE(login)->request);

	if (request->AssertionConsumerServiceURL) {
		if (lasso_saml20_provider_check_assertion_consumer_service_url(remote_provider,
					request->AssertionConsumerServiceURL,
					request->ProtocolBinding)) {
			return g_strdup(request->AssertionConsumerServiceURL);
		}
	}

	if (request->AssertionConsumerServiceIndex != -1 || request->ProtocolBinding == NULL) {
		url = lasso_saml20_provider_get_assertion_consumer_service_url(remote_provider,
				request->AssertionConsumerServiceIndex);
	}

	if (url == NULL && request->ProtocolBinding) {
		url = lasso_saml20_provider_get_assertion_consumer_service_url_by_binding(
				remote_provider, request->ProtocolBinding);
	}

	if (url == NULL) {
		message(G_LOG_LEVEL_WARNING,
				"can't find assertion consumer service url (going for default)");
		url = lasso_saml20_provider_get_assertion_consumer_service_url(remote_provider, -1);
	}

	return url;
}

gint
lasso_saml20_login_init_idp_initiated_authn_request(LassoLogin *login,
		const gchar *remote_providerID)
{
	LassoProfile *profile = NULL;
	LassoProvider *provider = NULL;
	LassoServer *server = NULL;
	gchar *default_name_id_format = NULL;
	int rc = 0;

	profile = &login->parent;
	lasso_extract_node_or_fail(server, lasso_profile_get_server(profile), SERVER,
			LASSO_PROFILE_ERROR_MISSING_SERVER);
	provider = lasso_server_get_provider(server, remote_providerID);
	if (! LASSO_IS_PROVIDER(provider))
		return LASSO_SERVER_ERROR_PROVIDER_NOT_FOUND;

	lasso_assign_string(profile->remote_providerID, remote_providerID);
	lasso_assign_gobject(profile->request, lasso_samlp2_authn_request_new());
	lasso_assign_new_gobject(LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy,
			lasso_samlp2_name_id_policy_new());
	lasso_assign_new_gobject(LASSO_SAMLP2_REQUEST_ABSTRACT(profile->request)->Issuer,
			LASSO_SAML2_NAME_ID(lasso_saml2_name_id_new_with_string(
					(char*)remote_providerID)));
	default_name_id_format = lasso_provider_get_default_name_id_format(provider);
	/* Change default NameIDFormat if default exists */
	if (default_name_id_format) {
		lasso_assign_new_string(LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy->Format,
				default_name_id_format);
	} else {
		/* we eventually used the default of the IDP (not of the target SP), so we reset to
		 * Lasso default, that is Transient */
		lasso_assign_string(LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy->Format,
				LASSO_SAML2_NAME_IDENTIFIER_FORMAT_TRANSIENT);
	}
	lasso_assign_string(LASSO_SAMLP2_AUTHN_REQUEST(profile->request)->NameIDPolicy->SPNameQualifier,
		remote_providerID);
cleanup:

	return rc;
}
