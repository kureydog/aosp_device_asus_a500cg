/*
 * Copyright 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _CAPSULE_H
#define _CAPSULE_H

#include "stdbool.h"

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint8_t u8;

struct capsule_signature {
	u32 signature;
	u8 sha256[32];
	u8 reserved[516];
} __packed;

struct capsule_version {
	u32 signature;
	u32 length;
	u32 iafw_stage1_version;
	u32 iafw_stage2_version;
	u32 mcu_version;
	u32 sec_version;
} __packed;

struct capsule_refs {
	u32 iafw_stage1_offset;
	u32 iafw_stage1_size;
	u32 iafw_stage2_offset;
	u32 iafw_stage2_size;
	u32 pdr_offset;
	u32 pdr_size;
	u32 sec_offset;
	u32 sec_size;
} __packed;

struct capsule_header {
	struct capsule_signature sig;
	struct capsule_version ver;
	struct capsule_refs refs;
} __packed;

struct capsule {
	struct capsule_header header;
	void *iafw_stage1;
	void *iafw_stage2;
	void *pdr;
	void *sec_fw;
};

typedef struct {
	int val_0;
	int val_1;
	int val_2;
} sec_version_t;

#endif
