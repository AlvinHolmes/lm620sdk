-----------------------------------------------------------------------
-- Xmake script file
-- Copyright (C) 2023, Nanjing Innochip Technology Co.,Ltd.
-- All rights reserved.
--
-- @author      geanfeng
-- @file        xmake.lua
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Configuration
-----------------------------------------------------------------------
local TARGET_NAME = "lwip"

-----------------------------------------------------------------------
-- Export global macro
-----------------------------------------------------------------------

-----------------------------------------------------------------------
-- Export global include directory
-----------------------------------------------------------------------
--add_global_includedirs("inc")
	add_global_includedirs("lwip-2.1.2/src/include")
	add_global_includedirs("lwip-2.1.2/src/include/netif")
	add_global_includedirs("lwip-2.1.2/src/include/compat/posix")
	add_global_includedirs("port")
	add_global_includedirs("port/arch/include")
-----------------------------------------------------------------------
-- Private function
-----------------------------------------------------------------------
-- 1. The minimum set of files needed for lwIP.
function  add_core_files()
	add_srcfile("lwip-2.1.2/src/core/init.c")
	add_srcfile("lwip-2.1.2/src/core/def.c")
	add_srcfile("lwip-2.1.2/src/core/dns.c")
	add_srcfile("lwip-2.1.2/src/core/inet_chksum.c")
	add_srcfile("lwip-2.1.2/src/core/ip.c")
	add_srcfile("lwip-2.1.2/src/core/memp.c")
	add_srcfile("lwip-2.1.2/src/core/netif.c")
	add_srcfile("lwip-2.1.2/src/core/pbuf.c")
	add_srcfile("lwip-2.1.2/src/core/raw.c")
	add_srcfile("lwip-2.1.2/src/core/stats.c")
	add_srcfile("lwip-2.1.2/src/core/sys.c")
	add_srcfile("lwip-2.1.2/src/core/tcp.c")
	add_srcfile("lwip-2.1.2/src/core/tcp_in.c")
	add_srcfile("lwip-2.1.2/src/core/tcp_out.c")
	add_srcfile("lwip-2.1.2/src/core/timeouts.c")
	add_srcfile("lwip-2.1.2/src/core/udp.c")
	-- 1.1 altcp
	add_srcfile("lwip-2.1.2/src/core/altcp.c")
	add_srcfile("lwip-2.1.2/src/core/altcp_alloc.c")
	add_srcfile("lwip-2.1.2/src/core/altcp_tcp.c")
	-- 1.2 ipv4
	add_srcfile("lwip-2.1.2/src/core/ipv4/autoip.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/dhcp.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/etharp.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/icmp.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/igmp.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/ip4_frag.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/ip4.c")
	add_srcfile("lwip-2.1.2/src/core/ipv4/ip4_addr.c")
	-- 1.3 ipv6
	add_srcfile("lwip-2.1.2/src/core/ipv6/dhcp6.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/ethip6.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/icmp6.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/inet6.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/ip6.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/ip6_addr.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/ip6_frag.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/mld6.c")
	add_srcfile("lwip-2.1.2/src/core/ipv6/nd6.c")
end

-- 2. APIFILES: The files which implement the sequential and socket APIs.
function add_api_files()
	add_srcfile("lwip-2.1.2/src/api/api_lib.c")
	add_srcfile("lwip-2.1.2/src/api/api_msg.c")
	add_srcfile("lwip-2.1.2/src/api/err.c")
	add_srcfile("lwip-2.1.2/src/api/if_api.c")
	add_srcfile("lwip-2.1.2/src/api/netbuf.c")
	add_srcfile("lwip-2.1.2/src/api/netdb.c")
	add_srcfile("lwip-2.1.2/src/api/netifapi.c")
	add_srcfile("lwip-2.1.2/src/api/sockets.c")
	add_srcfile("lwip-2.1.2/src/api/tcpip.c")
end

-- 3. Files implementing various generic network interface functions
function add_netif_files()
	add_srcfile("lwip-2.1.2/src/netif/ethernet.c")
	--add_srcfile("lwip-2.1.2/src/port/cat1netif/cat1netif.c")
	--add_srcfile("lwip-2.1.2/src/port/cat1netif/cat1netmgr.c")
	--add_srcfile("lwip-2.1.2/src/port/cat1netif/cat1fastnat.c")
	--add_srcfile("lwip-2.1.2/src/port/netif/ethernetif.c")
	--add_srcfile("lwip-2.1.2/src/port/cfg/lwip_cfg.c")
	--add_srcfile("lwip-2.1.2/src/port/debug/lwip_debug.c")
	--add_srcfile("lwip-2.1.2/src/port/openapi/lwip_openapi.c")
	-- 3.0 netif manager
	--add_srcfile("lwip-2.1.2/src/port/cat1netif/ict_netif.c")
	-- 3.1 Files implementing an IEEE 802.1D bridge by using a multilayer netif approach
	add_srcfile("lwip-2.1.2/src/netif/bridgeif.c")
	add_srcfile("lwip-2.1.2/src/netif/bridgeif_fdb.c")
	-- 3.2 A generic implementation of the SLIP (Serial Line IP) protocol.
	add_srcfile("lwip-2.1.2/src/netif/slipif.c")
end

-- 4. 6LoWPAN
function add_6lowpan_files()
	add_srcfile("lwip-2.1.2/src/netif/lowpan6.c")
	-- 4.1 A 6LoWPAN over Bluetooth Low Energy (BLE) implementation as netif, according to RFC-7668.
	add_srcfile("lwip-2.1.2/src/netif/lowpan6_ble.c")
	-- 4.2 Common 6LowPAN routines for IPv6.
	add_srcfile("lwip-2.1.2/src/netif/lowpan6_common.c")
	-- 4.3 A netif implementing the ZigBee Encapsulation Protocol (ZEP).
	add_srcfile("lwip-2.1.2/src/netif/zepif.c")
end

-- 5. PPP
function add_ppp_files()
	add_srcfile("lwip-2.1.2/src/netif/ppp/auth.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/ccp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/chap-md5.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/chap_ms.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/chap-new.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/demand.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/eap.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/ecp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/eui64.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/fsm.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/ipcp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/ipv6cp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/lcp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/magic.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/mppe.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/multilink.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/ppp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/pppapi.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/pppcrypt.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/pppoe.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/pppol2tp.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/pppos.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/upap.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/utils.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/vj.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/polarssl/arc4.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/polarssl/des.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/polarssl/md4.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/polarssl/md5.c")
	add_srcfile("lwip-2.1.2/src/netif/ppp/polarssl/sha1.c")
end

-- 6. SNMPv3 agent
function add_snmp_files()
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_asn1.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_core.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_icmp.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_interfaces.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_ip.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_snmp.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_system.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_tcp.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_mib2_udp.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_snmpv2_framework.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_snmpv2_usm.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_msg.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmpv3.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_netconn.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_pbuf_stream.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_raw.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_scalar.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_table.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_threadsync.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmp_traps.c")
end

-- 7. HTTP server + client
function add_http_files()
	add_srcfile("lwip-2.1.2/src/apps/http/altcp_proxyconnect.c")
	add_srcfile("lwip-2.1.2/src/apps/http/fs.c")
	add_srcfile("lwip-2.1.2/src/apps/http/http_client.c")
	add_srcfile("lwip-2.1.2/src/apps/http/httpd.c")
end

-- 8. MAKEFSDATA HTTP server host utility
function add_akefsdata_files()
	add_srcfile("lwip-2.1.2/src/apps/http/makefsdata/makefsdata.c")
end

-- 9. IPERF server
function add_iperf_files()
	add_srcfile("lwip-2.1.2/src/apps/lwiperf/lwiperf.c")
end

-- 10. SMTP client
function add_smtp_files()
	add_srcfile("lwip-2.1.2/src/apps/smtp/smtp.c")
end

-- 11. SNTP client
function add_sntp_files()
	add_srcfile("lwip-2.1.2/src/apps/sntp/sntp.c")
end

-- 12. MDNS responder
function add_mdns_files()
	add_srcfile("lwip-2.1.2/src/apps/mdns/mdns.c")
end

-- 13. NetBIOS name server
function add_netbios_files()
	add_srcfile("lwip-2.1.2/src/apps/netbiosns/netbiosns.c")
end

-- 14. TFTP server files
function add_tftp_files()
	add_srcfile("lwip-2.1.2/src/apps/tftp/tftp_server.c")
	add_srcfile("lwip-2.1.2/src/apps/tftp/tftp_port.c")
end

-- 15. MQTT client files
function add_mqtt_files()
	add_srcfile("lwip-2.1.2/src/apps/mqtt/mqtt.c")
end

-- 16. ARM MBEDTLS related files of lwIP rep
function add_mbedtls_files()
	add_srcfile("lwip-2.1.2/src/apps/altcp_tls/altcp_tls_mbedtls.c")
	add_srcfile("lwip-2.1.2/src/apps/altcp_tls/altcp_tls_mbedtls_mem.c")
	add_srcfile("lwip-2.1.2/src/apps/snmp/snmpv3_mbedtls.c")
end

-- 17. ping
function add_ping_files()
	add_srcfile("lwip-2.1.2/src/apps/ping/ping.c")
	add_srcfile("lwip-2.1.2/src/apps/ping/lwping.c")
end

-- 18. dhcpd
function add_dhcpd_files()
	add_srcfile("lwip-2.1.2/src/apps/dhcpd/dhcp_server.c")
	add_srcfile("lwip-2.1.2/src/apps/dhcpd/dhcp6_server.c")
end

-- 19. nat
function add_nat_files()
	add_srcfile("lwip-2.1.2/src/apps/nat/ip4_nat.c")
end

-- 20. router
function add_route_files()
	add_srcfile("lwip-2.1.2/src/apps/route/ip6_route.c")
end

-----------------------------------------------------------------------
-- Target
-----------------------------------------------------------------------
target(TARGET_NAME)
-- [don't edit] ---
    set_kind("static")
	add_deps("base")
	add_global_deps(TARGET_NAME)
-- [   end    ] ---
    --私有头文件路径
	add_includedirs("lwip-2.1.2/src")
	add_includedirs("lwip-2.1.2/src/apps")
	add_includedirs("lwip-2.1.2/src/include")
	add_includedirs("lwip-2.1.2/src/include/netif")
	add_includedirs("port")
	add_includedirs("port/arch/include")

	--私有宏定义
	--add_defines("DEBUG")

	--源文件包含
	add_srcfile("port/arch/sys_arch.c")
	-- 1. core
	add_core_files()
	-- 2. api
	add_api_files()
	-- 3. netif
	add_netif_files()

	-- 4. 6LoWPAN
	add_6lowpan_files()
	-- 5. PPP
	add_ppp_files()
	-- 6. SNMPv3 agent
	add_snmp_files()
	-- 7. HTTP server + client
	add_http_files()
	-- 8. MAKEFSDATA HTTP server host utility
	--add_akefsdata_files()
	-- 9. IPERF server
	--add_iperf_files() use startup\apps\lwiperf instead
	-- 10. SMTP client
	--add_smtp_files()
	-- 11. SNTP client
	add_sntp_files()
	-- 12. MDNS responder
	add_mdns_files()
	-- 13. NetBIOS name server
	add_netbios_files()
	-- 14. TFTP server files
	--add_tftp_files()
	-- 15. MQTT client files
	add_mqtt_files()
	-- 16. ARM MBEDTLS related files of lwIP rep
	add_mbedtls_files()
	-- 17. ping
	--add_ping_files()
	-- 18. dhcpd
	--add_dhcpd_files()
	-- 19. nat
	--add_nat_files()
	-- 20. router
	add_route_files()

	--对外SDK头文件
	add_headerfiles("port/(**.h)", {prefixdir = "lwip/port"})
    before_install(function (target)
		os.cp("components/net/lwip/lwip-2.1.2/src/include", target:installdir() .. "/include/lwip")
	end)	
target_end()
