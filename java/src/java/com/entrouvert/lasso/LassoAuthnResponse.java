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

package com.entrouvert.lasso;

public class LassoAuthnResponse { // LassoAuthnResponse

    public LassoAuthnResponse(String providerID, LassoNode request){
        init(providerID, request);
    }

    native protected void init(String providerID, LassoNode request);
    native public String getStatus();

    // From LassoSamlpResponse

    native public void setContent(String val);
    native public void setProviderID(String val);
    native public void setRelayState(String val);
} // LassoAuthnResponse

