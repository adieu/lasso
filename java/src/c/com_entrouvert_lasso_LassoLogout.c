/*
 * JLasso -- Java bindings for Lasso library
 *
 * Copyright (C) 2004 Entr'ouvert
 * http://lasso.entrouvert.com
 *
 * Authors: Benjamin Poussin <poussin@codelutin.com>
 *          Emmanuel Raviart <eraviart@entrouvert.com>
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

#include <stdio.h>
#include <helper.h>
#include <com_entrouvert_lasso_LassoLogout.h>
#include <lasso/lasso.h>

JNIEXPORT void JNICALL Java_com_entrouvert_lasso_LassoLogout_init
(JNIEnv * env, jobject this, jobject _server,
                             jobject _user,
                             jint _providerType){
    LassoLogout *logout;
    LassoServer* server;
    LassoUser* user;

    server = (LassoServer*)getCObject(env, _server);
    user = (LassoUser*)getCObject(env, _user);

    logout = lasso_logout_new(server, user, _providerType);

    storeCObject(env, this, logout);
}



JNIEXPORT jint JNICALL Java_com_entrouvert_lasso_LassoLogout_buildRequestMsg
(JNIEnv * env, jobject this){
    int result;
    LassoLogout* logout;

    logout = getCObject(env, this);
    result = lasso_logout_build_request_msg(logout);

    return result;
}


JNIEXPORT jint JNICALL Java_com_entrouvert_lasso_LassoLogout_buildResponseMmsg
(JNIEnv * env, jobject this){
    int result;
    LassoLogout* logout;

    logout = getCObject(env, this);
    result = lasso_logout_build_response_msg(logout);

    return result;
}


JNIEXPORT jint JNICALL Java_com_entrouvert_lasso_LassoLogout_initRequest
(JNIEnv * env, jobject this, jstring _providerID){
    int result;
    LassoLogout* logout;
    char *providerID;

    providerID = (char*)(*env)->GetStringUTFChars(env, _providerID, NULL);

    logout = getCObject(env, this);
    result = lasso_logout_init_request(logout, providerID);

    (*env)->ReleaseStringUTFChars(env, _providerID, providerID);

    return result;
}

JNIEXPORT jint JNICALL Java_com_entrouvert_lasso_LassoLogout_processRequestMsg
(JNIEnv * env, jobject this, jstring _requestMsg,
                             jint _requestMethod){
    int result;
    LassoLogout* logout;
    char *requestMsg;

    requestMsg = (char*)(*env)->GetStringUTFChars(env, _requestMsg, NULL);

    logout = getCObject(env, this);
    result = lasso_logout_process_request_msg(logout,
                        requestMsg,
                        _requestMethod);

    (*env)->ReleaseStringUTFChars(env, _requestMsg, requestMsg);

    return result;
}

JNIEXPORT jint JNICALL Java_com_entrouvert_lasso_LassoLogout_processResponseMsg
(JNIEnv * env, jobject this, jstring _responseMsg,
                             jint _responseMethod){
    int result;
    LassoLogout* logout;
    char *responseMsg;

    responseMsg = (char*)(*env)->GetStringUTFChars(env, _responseMsg, NULL);

    logout = getCObject(env, this);
    result = lasso_logout_process_response_msg(logout,
                        responseMsg,
                        _responseMethod);

    (*env)->ReleaseStringUTFChars(env, _responseMsg, responseMsg);

    return result;
}


