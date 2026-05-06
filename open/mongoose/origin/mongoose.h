// Copyright (c) 2004-2013 Sergey Lyubka
// Copyright (c) 2013-2022 Cesanta Software Limited
// All rights reserved
//
// This software is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see http://www.gnu.org/licenses/
//
// You are free to use this software under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this software under a commercial
// license, as set out in https://www.mongoose.ws/licensing/
//
// SPDX-License-Identifier: GPL-2.0-only or commercial

#ifndef MONGOOSE_H
#define MONGOOSE_H

#define MG_VERSION "7.12"

#ifdef __cplusplus
extern "C" {
#endif

#include "src/arch.h"
#include "src/net_ft.h"
#include "src/net_lwip.h"
#include "src/net_rl.h"
#include "src/config.h"
#include "src/str.h"
#include "src/queue.h"
#include "src/fmt.h"
#include "src/printf.h"
#include "src/log.h"
#include "src/timer.h"
#include "src/fs.h"
#include "src/util.h"
#include "src/url.h"
#include "src/iobuf.h"
#include "src/base64.h"
#include "src/md5.h"
#include "src/sha1.h"
#include "src/event.h"
#include "src/net.h"
#include "src/http.h"
#include "src/ssi.h"
#include "src/tls.h"
#include "src/tls_mbed.h"
#include "src/tls_openssl.h"
#include "src/ws.h"
#include "src/sntp.h"
#include "src/mqtt.h"
#include "src/dns.h"
#include "src/json.h"
#include "src/rpc.h"
#include "src/ota.h"
#include "src/device.h"
#include "src/net_builtin.h"

#ifdef __cplusplus
}
#endif
#endif  // MONGOOSE_H
