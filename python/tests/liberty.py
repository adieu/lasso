# -*- coding: UTF-8 -*-


# Lasso Simulator
# By: Emmanuel Raviart <eraviart@entrouvert.com>
#
# Copyright (C) 2004 Entr'ouvert
# http://lasso.entrouvert.org
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


from LibertyEnabledClientProxy import LibertyEnabledClientProxyMixin
from IdentityProvider import IdentityProviderMixin
from ServiceProvider import ServiceProviderMixin
from Provider import ProviderMixin
import web


class LibertyEnabledClientProxy(LibertyEnabledClientProxyMixin, web.WebClient):
    def __init__(self):
        web.WebClient.__init__(self)
        LibertyEnabledClientProxyMixin.__init__(self)


class Provider(ProviderMixin, web.WebSite):
    def __init__(self, url):
        web.WebSite.__init__(self, url)
        ProviderMixin.__init__(self)


class IdentityProvider(IdentityProviderMixin, Provider):
    def __init__(self, url):
        Provider.__init__(self, url)
        IdentityProviderMixin.__init__(self)


class ServiceProvider(ServiceProviderMixin, Provider):
    def __init__(self, url):
        Provider.__init__(self, url)
        ServiceProviderMixin.__init__(self)
