#! /usr/bin/env php
<?php
# Lasso - A free implementation of the Liberty Alliance specifications.
# 
# Copyright (C) 2004-2007 Entr'ouvert
# http://lasso.entrouvert.org
#
# Authors: See AUTHORS file in top-level directory.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

require("../lasso.php");

define("DATA_DIR", "../../../tests/data/");

function test01() {
    echo "Testing lasso_provider_get_organization binding which must return an xmlNode*... ";

    $organisation_string = '<Organization xmlns="urn:liberty:metadata:2003-08">
  <OrganizationName>Name of the organization</OrganizationName>
 </Organization>';

    $server = new LassoServer(
        DATA_DIR . "sp1-la/metadata.xml",
        DATA_DIR . "sp1-la/private-key-raw.pem",
        NULL,
        DATA_DIR . "sp1-la/certificate.pem");
    assert(!is_null($server->organization));
    assert($server->organization == $organisation_string);

    echo "OK.\n";
}

function test02() {
    echo "Get and set a GList of strings... ";

    $requestAuthnContext = new LassoLibRequestAuthnContext();
    $requestAuthnContext->authnContextClassRef = array(LASSO_LIB_AUTHN_CONTEXT_CLASS_REF_PASSWORD);
    assert(! is_null($requestAuthnContext->authnContextClassRef));
    assert(sizeof($requestAuthnContext->authnContextClassRef) == 1);
    assert($requestAuthnContext->authnContextClassRef[0] == LASSO_LIB_AUTHN_CONTEXT_CLASS_REF_PASSWORD);

    echo "OK.\n";
}

function test03() {
    echo "Get and set 'providers' attribute (hashtable) of a server object... ";

    $server = new LassoServer(
        DATA_DIR . "sp1-la/metadata.xml",
        DATA_DIR . "sp1-la/private-key-raw.pem",
        NULL,
        DATA_DIR . "sp1-la/certificate.pem");
    $server->addProvider(
        LASSO_PROVIDER_ROLE_IDP,
        DATA_DIR . "idp1-la/metadata.xml",
        DATA_DIR . "idp1-la/public-key.pem",
        DATA_DIR . "idp1-la/certificate.pem");
    assert(!is_null($server->providers));
    assert($server->providers["https://idp1/metadata"]->providerId == "https://idp1/metadata");
    $tmp_providers = $server->providers;
    $server->providers = NULL;
    assert(!$server->providers);
    $server->providers = $tmp_providers;
    assert($server->providers["https://idp1/metadata"]->providerId == "https://idp1/metadata");

    echo "OK.\n";
}


lasso_init();
test01();
test02();
test03();
lasso_shutdown();
