#include <lasso/protocols/single_sign_on_and_federation.h>

LassoNode *lasso_build_authnRequest(const xmlChar *providerID,
				    const xmlChar *nameIDPolicy,
				    const xmlChar *isPassive,
				    const xmlChar *forceAuthn,
				    const xmlChar *assertionConsumerServiceID,
				    const xmlChar **authnContextClassRefs,
				    const xmlChar **authnContextStatementRefs,
				    const xmlChar *authnContextComparison,
				    const xmlChar *relayState,
				    const xmlChar *proxyCount,
				    const xmlChar **idpList,
				    const xmlChar *consent)
{
  LassoNode  *request;

  // build AuthnRequest class
  request = lasso_lib_authn_request_new();
  
  lasso_samlp_request_abstract_set_requestID(LASSO_SAMLP_REQUEST_ABSTRACT(request),
					     (const xmlChar *)lasso_build_unique_id(32));
  lasso_samlp_request_abstract_set_minorVersion(LASSO_SAMLP_REQUEST_ABSTRACT(request), 
						lassoLibMinorVersion);
  lasso_samlp_request_abstract_set_issueInstance(LASSO_SAMLP_REQUEST_ABSTRACT(request),
						 lasso_get_current_time());
  lasso_samlp_request_abstract_set_majorVersion(LASSO_SAMLP_REQUEST_ABSTRACT(request),
						lassoLibMajorVersion);

  lasso_lib_authn_request_set_providerID(LASSO_LIB_AUTHN_REQUEST(request),
					 providerID);

  if(nameIDPolicy){
    lasso_lib_authn_request_set_nameIDPolicy(LASSO_LIB_AUTHN_REQUEST(request), nameIDPolicy);
  }

  if(isPassive){
    lasso_lib_authn_request_set_isPassive(LASSO_LIB_AUTHN_REQUEST(request), isPassive);
  }

  if(forceAuthn){
    lasso_lib_authn_request_set_forceAuthn(LASSO_LIB_AUTHN_REQUEST(request), forceAuthn);
  }

  if(assertionConsumerServiceID){
    lasso_lib_authn_request_set_assertionConsumerServiceID(LASSO_LIB_AUTHN_REQUEST(request),
							   assertionConsumerServiceID);
  }

  if(relayState!=NULL){
    lasso_lib_authn_request_set_relayState(LASSO_LIB_AUTHN_REQUEST(request), relayState);
  }

  if(consent!=NULL){
    lasso_lib_authn_request_set_consent(LASSO_LIB_AUTHN_REQUEST(request), consent);
  }

  return(request);
}


LassoNode *lasso_build_authnResponse(LassoNode *request,
				     const xmlChar *providerID)
{
     LassoNode *response;

     response = lasso_lib_authn_response_new();
     
     lasso_samlp_response_abstract_set_responseID(LASSO_SAMLP_RESPONSE_ABSTRACT(response),
						  (const xmlChar *)lasso_build_unique_id(32));
     lasso_samlp_request_abstract_set_majorVersion(LASSO_SAMLP_REQUEST_ABSTRACT(response),
						   lassoLibMajorVersion);     
     lasso_samlp_request_abstract_set_minorVersion(LASSO_SAMLP_REQUEST_ABSTRACT(response), 
						   lassoLibMinorVersion);
     lasso_samlp_request_abstract_set_issueInstance(LASSO_SAMLP_REQUEST_ABSTRACT(response),
						    lasso_get_current_time());

     return(response);
}

LassoNode *lasso_build_response(LassoNode *request,
				const xmlChar *providerID)
{
     LassoNode *response;

     response = lasso_samlp_response_new();
     
     lasso_samlp_response_abstract_set_responseID(LASSO_SAMLP_RESPONSE_ABSTRACT(response),
						  (const xmlChar *)lasso_build_unique_id(32));
     lasso_samlp_request_abstract_set_majorVersion(LASSO_SAMLP_REQUEST_ABSTRACT(response),
						   lassoSamlMajorVersion);     
     lasso_samlp_request_abstract_set_minorVersion(LASSO_SAMLP_REQUEST_ABSTRACT(response), 
						   lassoSamlMinorVersion);
     lasso_samlp_request_abstract_set_issueInstance(LASSO_SAMLP_REQUEST_ABSTRACT(response),
						    lasso_get_current_time());

     return(response);
}
