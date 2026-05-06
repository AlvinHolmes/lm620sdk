/**********************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 *  distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License  for the specific language governing permissions and limitations under the License.
 *
 ***********************************************************************************************************/

/******************************************************************************
* Referenced head files
*****************************************************************************/

#ifndef __AT_CMDM_H__
#define __AT_CMDM_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <oneos_config.h>
#include <stdint.h>

/******************************************************************************
* Definitions
******************************************************************************/



/******************************************************************************
 * Function declares
 *****************************************************************************/

void at_cmdm_config_set(uint8_t id,uint8_t *para_p, uint16_t lenOfPara);
void at_cmdm_config_ex_set(uint8_t id,uint8_t *para_p, uint16_t lenOfPara);

#ifdef __cplusplus
}
#endif
#endif /* __AT_CMDM_H__*/

