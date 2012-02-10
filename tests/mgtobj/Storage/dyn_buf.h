/*
 * libdmclient test materials
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
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
 *
 * David Navarro <david.navarro@intel.com>
 *
 */

#ifndef DMC_DYN_BUF_H
#define DMC_DYN_BUF_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

typedef struct dmc_buf_ dmc_buf;

struct dmc_buf_
{
    unsigned int size;
    unsigned int max_size;
    unsigned int block_size;
    uint8_t *buffer;
};

typedef void (*dmc_ptr_array_des) (void *);

typedef struct dmc_ptr_array_ dmc_ptr_array;

struct dmc_ptr_array_
{
    unsigned int size;
    unsigned int block_size;
    unsigned int max_size;
    void **array;
    dmc_ptr_array_des destructor;
};

void dmc_buf_make(dmc_buf *buffer, unsigned int block_size);
void dmc_buf_free(dmc_buf *buffer);
int dmc_buf_append(dmc_buf *buffer, const uint8_t *data,
            unsigned int data_size);
int dmc_buf_append_str(dmc_buf *buffer, const char *data);
int dmc_buf_zero_terminate(dmc_buf *buffer);
uint8_t *dmc_buf_adopt(dmc_buf *buffer);

#define dmc_buf_size(buffer) (buffer)->size

void dmc_ptr_array_make(dmc_ptr_array *array,
                    unsigned int block_size,
                    dmc_ptr_array_des destructor);
void dmc_ptr_array_make_from(dmc_ptr_array *array,
                    void **new_array, unsigned int size,
                    unsigned int block_size,
                    dmc_ptr_array_des destructor);

void dmc_ptr_array_adopt(dmc_ptr_array *array, void **carray,
                    unsigned int *size);

void dmc_ptr_array_free(dmc_ptr_array *array);
void dmc_ptr_array_free_callback(void *array);
int dmc_ptr_array_append(dmc_ptr_array *array, void *pointer);
void dmc_ptr_array_delete(dmc_ptr_array *array, unsigned int index);

#define dmc_ptr_array_get(dmc_ptr_array, index) (dmc_ptr_array)->array[index]
#define dmc_ptr_array_set(dmc_ptr_array, index, object) \
    (dmc_ptr_array)->array[index] = (object)
#define dmc_ptr_array_get_size(dmc_ptr_array) (dmc_ptr_array)->size
#define dmc_ptr_array_get_array(dmc_ptr_array) (dmc_ptr_array)->array

#ifdef __cplusplus
}
#endif

#endif
